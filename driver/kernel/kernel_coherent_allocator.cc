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

#include "driver/kernel/kernel_coherent_allocator.h"

#include <fcntl.h>
#include <stddef.h>
#include <sys/types.h>

#include <cinttypes>  // for PRI*64
#include <map>

#include "driver/hardware_structures.h"
#include "driver/kernel/gasket_ioctl.h"
#include "port/cleanup.h"
#include "port/integral_types.h"
#include "port/logging.h"
#include "port/math_util.h"
#include "port/status_macros.h"

namespace platforms {
namespace darwinn {
namespace driver {

KernelCoherentAllocator::KernelCoherentAllocator(const std::string &device_path,
                                                 int alignment_bytes,
                                                 size_t size_bytes)
    : CoherentAllocator(alignment_bytes, size_bytes),
      device_path_(device_path) {}

util::StatusOr<char *> KernelCoherentAllocator::DoOpen(size_t size_bytes) {
  if (fd_ != INVALID_FD_VALUE) {
    return util::FailedPreconditionError("Device already open.");
  }

  fd_ = open(device_path_.c_str(), O_RDWR);
  if (fd_ == INVALID_FD_VALUE) {
    return util::FailedPreconditionError(
        StringPrintf("Device open failed : %d (%s)", fd_, strerror(errno)));
  }

  auto fd_closer = MakeCleanup([this] {
    close(fd_);
    fd_ = INVALID_FD_VALUE;
  });

  // Enable the allocator and request the memory region
  // Note: only one region is supported by kernel driver.
  // The kernel coherent allocator returns zero'ed memory.
  gasket_coherent_alloc_config_ioctl ioctl_buffer;
  memset(&ioctl_buffer, 0, sizeof(ioctl_buffer));
  ioctl_buffer.page_table_index = 0;
  ioctl_buffer.enable = 1;
  ioctl_buffer.size = size_bytes;
  if (ioctl(fd_, GASKET_IOCTL_CONFIG_COHERENT_ALLOCATOR, &ioctl_buffer) != 0) {
    return util::FailedPreconditionError(StringPrintf(
        "Could not enable coherent allocator size %" PRIu64 ". : fd=%d (%s)",
        ioctl_buffer.size, fd_, strerror(errno)));
  }

  dma_address_ = ioctl_buffer.dma_address;

  // Map the memory range so as it can be accessed by user.
  char *mem_base;
  auto statusor = Map(fd_, size_bytes, dma_address_);
  if (!statusor.ok()) {
    // Release the memory block.
    ioctl_buffer.page_table_index = 0;
    ioctl_buffer.enable = 0;
    ioctl_buffer.size = size_bytes;
    if (ioctl(fd_, GASKET_IOCTL_CONFIG_COHERENT_ALLOCATOR, &ioctl_buffer) !=
        0) {
      VLOG(1) << StringPrintf("mmap_failed and couldn't free memory : %s.\n",
                              strerror(errno));
    }
    return statusor.status();
  }
  mem_base = std::move(statusor.ValueOrDie());

  fd_closer.release();
  return mem_base;
}

util::Status KernelCoherentAllocator::DoClose(char *mem_base,
                                              size_t size_bytes) {
  if (fd_ == INVALID_FD_VALUE) {
    return util::FailedPreconditionError("Device not open.");
  }
  util::Status status = Unmap(fd_, mem_base, size_bytes);

  // Release the memory block.
  gasket_coherent_alloc_config_ioctl ioctl_buffer;
  memset(&ioctl_buffer, 0, sizeof(ioctl_buffer));
  ioctl_buffer.page_table_index = 0;
  ioctl_buffer.enable = 0;
  ioctl_buffer.dma_address = dma_address_;
  ioctl_buffer.size = size_bytes;
  if (ioctl(fd_, GASKET_IOCTL_CONFIG_COHERENT_ALLOCATOR, &ioctl_buffer) != 0) {
    status.Update(util::FailedPreconditionError(StringPrintf(
        "Could not disable coherent allocator size %" PRIu64 ". : %d (%s)",
        ioctl_buffer.size, fd_, strerror(errno))));
    return status;
  }

  close(fd_);
  fd_ = INVALID_FD_VALUE;
  dma_address_ = 0;
  return util::Status();  // OK
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
