// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "driver/kernel/kernel_mmu_mapper.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cinttypes>  // for PRI*64
#include <string>

#include "driver/hardware_structures.h"
#include "driver/kernel/linux_gasket_ioctl.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/status.h"
#include "port/statusor.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"
#include "port/tracing.h"

namespace platforms {
namespace darwinn {
namespace driver {

KernelMmuMapper::KernelMmuMapper(const std::string &device_path)
    : device_path_(device_path) {}

util::Status KernelMmuMapper::Open(
    int num_simple_page_table_entries_requested) {
  StdMutexLock lock(&mutex_);
  if (fd_ != -1) {
    return util::FailedPreconditionError("Device already open.");
  }

  fd_ = open(device_path_.c_str(), O_RDWR);
  if (fd_ < 0) {
    return util::FailedPreconditionError(
        StringPrintf("Device open failed : %d (%s)", fd_, strerror(errno)));
  }

  gasket_page_table_ioctl ioctl_buffer;
  memset(&ioctl_buffer, 0, sizeof(ioctl_buffer));
  ioctl_buffer.page_table_index = 0;
  ioctl_buffer.size = num_simple_page_table_entries_requested;
  if (ioctl(fd_, GASKET_IOCTL_PARTITION_PAGE_TABLE, &ioctl_buffer) != 0) {
    return util::FailedPreconditionError(StringPrintf(
        "Could not partition page table. : %d (%s)", fd_, strerror(errno)));
  }

  return util::Status();  // OK
}

util::Status KernelMmuMapper::Close() {
  StdMutexLock lock(&mutex_);
  if (fd_ == -1) {
    return util::FailedPreconditionError("Device not open.");
  }

  close(fd_);
  fd_ = -1;

  return util::Status();  // OK
}

// Converts DmaDirection to
// gasket_page_table_ioctl_flags.flags.DMA_DIRECTION flag.
static uint32_t DirectionFlag(DmaDirection direction) {
  switch (direction) {
    case DmaDirection::kBidirectional:
      return DMA_BIDIRECTIONAL;
    case DmaDirection::kToDevice:
      return DMA_TO_DEVICE;
    case DmaDirection::kFromDevice:
      return DMA_FROM_DEVICE;
  }
}

util::Status KernelMmuMapper::DoMap(const void *buffer, int num_pages,
                                    uint64 device_virtual_address,
                                    DmaDirection direction) {
  TRACE_SCOPE("KernelMmuMapper::DoMap");
  StdMutexLock lock(&mutex_);
  if (fd_ == -1) {
    return util::FailedPreconditionError("Device not open.");
  }

  gasket_page_table_ioctl_flags buffer_to_map;
  memset(&buffer_to_map, 0, sizeof(buffer_to_map));
  buffer_to_map.base.page_table_index = 0;
  buffer_to_map.base.host_address = reinterpret_cast<uintptr_t>(buffer);
  buffer_to_map.base.size = num_pages * kHostPageSize;
  buffer_to_map.base.device_address = device_virtual_address;
  buffer_to_map.flags = DirectionFlag(direction)
                        << GASKET_PT_FLAGS_DMA_DIRECTION_SHIFT;

  int ioctl_retval;
  if (map_flags_supported_) {
    ioctl_retval = ioctl(fd_, GASKET_IOCTL_MAP_BUFFER_FLAGS, &buffer_to_map);
    if (ioctl_retval == -EPERM || ioctl_retval == -ENOTTY ||
        ioctl_retval == -EINVAL) {
      VLOG(4) << StringPrintf("Failed to map buffer with flags, error %d",
                              ioctl_retval);
      // This corresponds to an old kernel which doesn't yet support flags.
      // Set member variable to fallback to legacy IOCTL and try again.
      map_flags_supported_ = false;
    }
  }

  if (!map_flags_supported_) {
    ioctl_retval = ioctl(fd_, GASKET_IOCTL_MAP_BUFFER, &buffer_to_map.base);
  }

  if (ioctl_retval != 0) {
    return util::FailedPreconditionError(
        StringPrintf("Could not map pages : %d (%s)", fd_, strerror(errno)));
  }

  if (map_flags_supported_) {
    VLOG(4) << StringPrintf("MmuMapper#Map() : %016" PRIx64 " -> %016" PRIx64
                            " (%d pages) flags=%08" PRIx32 ".",
                            buffer_to_map.base.host_address,
                            buffer_to_map.base.device_address, num_pages,
                            buffer_to_map.flags);
  } else {
    VLOG(4) << StringPrintf("MmuMapper#Map() : %016" PRIx64 " -> %016" PRIx64
                            " (%d pages).",
                            buffer_to_map.base.host_address,
                            buffer_to_map.base.device_address, num_pages);
  }

  return util::Status();  // OK
}

util::Status KernelMmuMapper::DoUnmap(const void *buffer, int num_pages,
                                      uint64 device_virtual_address) {
  TRACE_SCOPE("KernelMmuMapper::DoUnmap");
  StdMutexLock lock(&mutex_);
  if (fd_ == -1) {
    return util::FailedPreconditionError("Device not open.");
  }

  gasket_page_table_ioctl buffer_to_map;
  memset(&buffer_to_map, 0, sizeof(buffer_to_map));
  buffer_to_map.page_table_index = 0;
  buffer_to_map.host_address = reinterpret_cast<uintptr_t>(buffer);
  buffer_to_map.size = num_pages * kHostPageSize;
  buffer_to_map.device_address = device_virtual_address;
  if (ioctl(fd_, GASKET_IOCTL_UNMAP_BUFFER, &buffer_to_map) != 0) {
    return util::FailedPreconditionError(
        StringPrintf("Could not unmap pages : %d (%s)", fd_, strerror(errno)));
  }

  VLOG(4) << StringPrintf(
      "MmuMaper#Unmap() : %016" PRIx64 " -> %016" PRIx64 " (%d pages).",
      buffer_to_map.host_address, buffer_to_map.device_address, num_pages);

  return util::Status();  // OK
}

util::Status KernelMmuMapper::DoMap(int fd, int num_pages,
                                    uint64 device_virtual_address,
                                    DmaDirection direction) {
  TRACE_SCOPE("KernelMmuMapper::DoMap");
  StdMutexLock lock(&mutex_);
  if (fd_ == -1) {
    return util::FailedPreconditionError("Device not open.");
  }

  gasket_page_table_ioctl_dmabuf buffer_to_map = {0};
  buffer_to_map.map = 1;
  buffer_to_map.page_table_index = 0;
  buffer_to_map.num_pages = num_pages;
  buffer_to_map.dmabuf_fd = fd;
  buffer_to_map.device_address = device_virtual_address;
  buffer_to_map.flags = DirectionFlag(direction)
                        << GASKET_PT_FLAGS_DMA_DIRECTION_SHIFT;

  int ioctl_retval = ioctl(fd_, GASKET_IOCTL_MAP_DMABUF, &buffer_to_map);
  if (ioctl_retval != 0) {
    return util::FailedPreconditionError(
        StringPrintf("Could not map pages : %d (%s)", fd_, strerror(errno)));
  }

  VLOG(4) << StringPrintf("MmuMapper#Map() : fd %d -> %016" PRIx64
                          " (%d pages) flags=%08" PRIx32 ".",
                          buffer_to_map.dmabuf_fd, buffer_to_map.device_address,
                          num_pages, buffer_to_map.flags);

  return util::OkStatus();
}

util::Status KernelMmuMapper::DoUnmap(int fd, int num_pages,
                                      uint64 device_virtual_address) {
  TRACE_SCOPE("KernelMmuMapper::DoUnmap");
  StdMutexLock lock(&mutex_);
  if (fd_ == -1) {
    return util::FailedPreconditionError("Device not open.");
  }

  gasket_page_table_ioctl_dmabuf buffer_to_unmap = {0};
  buffer_to_unmap.map = 0;
  buffer_to_unmap.page_table_index = 0;
  buffer_to_unmap.num_pages = num_pages;
  buffer_to_unmap.dmabuf_fd = fd;
  buffer_to_unmap.device_address = device_virtual_address;
  if (ioctl(fd_, GASKET_IOCTL_MAP_DMABUF, &buffer_to_unmap) != 0) {
    return util::FailedPreconditionError(
        StringPrintf("Could not unmap pages : %d (%s)", fd_, strerror(errno)));
  }

  VLOG(4) << StringPrintf(
      "MmuMaper#Unmap() : fd %d -> %016" PRIx64 " (%d pages).",
      buffer_to_unmap.dmabuf_fd, buffer_to_unmap.device_address, num_pages);

  return util::OkStatus();
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
