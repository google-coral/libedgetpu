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

#include "driver/kernel/windows/kernel_registers_windows.h"

#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "driver/kernel/gasket_ioctl.h"
#include "driver/registers/registers.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/statusor.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

KernelRegistersWindows::KernelRegistersWindows(
    const std::string& device_path, const std::vector<MmapRegion>& mmap_region,
    bool read_only)
    : KernelRegisters(device_path, mmap_region, read_only) {}

KernelRegistersWindows::KernelRegistersWindows(const std::string& device_path,
                                               uint64 mmap_offset,
                                               uint64 mmap_size, bool read_only)
    : KernelRegisters(device_path, mmap_offset, mmap_size, read_only) {}

KernelRegistersWindows::~KernelRegistersWindows() { UnmapAllRegions(); }

StatusOr<uint64*> KernelRegistersWindows::MapRegion(
    FileDescriptor fd, const MappedRegisterRegion& region, bool read_only) {
  gasket_address_map_ioctl ioctl = {0, region.offset, region.size, 0,
                                    0, nullptr};
  bool res;

  res = DeviceIoControl(fd, GASKET_IOCTL_MAP_HDW_VIEW, &ioctl, sizeof(ioctl),
                        &ioctl, sizeof(ioctl), NULL, NULL);
  if (!res) {
    return InternalError(StringPrintf(
        "KernelRegisters::MapRegion failed! gle=%d", GetLastError()));
  }

  return static_cast<uint64*>(ioctl.virtaddr);
}

Status KernelRegistersWindows::UnmapRegion(FileDescriptor fd,
                                           const MappedRegisterRegion& region) {
  gasket_address_map_ioctl ioctl = {0, region.offset,   region.size, 0,
                                    0, region.registers};
  bool res;

  res = DeviceIoControl(fd, GASKET_IOCTL_UNMAP_HDW_VIEW, &ioctl, sizeof(ioctl),
                        &ioctl, sizeof(ioctl), NULL, NULL);
  if (!res) {
    FailedPreconditionError(StringPrintf(
        "KernelRegisters::UnmapRegion failed! gle=%d", GetLastError()));
  }

  return OkStatus();
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
