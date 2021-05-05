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

#include "driver/kernel/linux/kernel_registers_linux.h"

#include <sys/mman.h>
#include <unistd.h>

#include "port/errors.h"
#include "port/integral_types.h"
#include "port/status.h"
#include "port/statusor.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

KernelRegistersLinux::KernelRegistersLinux(
    const std::string& device_path, const std::vector<MmapRegion>& mmap_region,
    bool read_only)
    : KernelRegisters(device_path, mmap_region, read_only) {}

KernelRegistersLinux::KernelRegistersLinux(const std::string& device_path,
                                           uint64 mmap_offset, uint64 mmap_size,
                                           bool read_only)
    : KernelRegisters(device_path, mmap_offset, mmap_size, read_only) {}

KernelRegistersLinux::~KernelRegistersLinux() { UnmapAllRegions(); }

StatusOr<uint64*> KernelRegistersLinux::MapRegion(
    FileDescriptor fd, const MappedRegisterRegion& region, bool read_only) {
  int protections = PROT_READ | PROT_WRITE;
  if (read_only) {
    protections = PROT_READ;
  }

  void* result = mmap(nullptr, region.size, protections,
                      MAP_SHARED, fd, region.offset);
  if (result == MAP_FAILED) {
    return InternalError(StringPrintf("Could not mmap: %s", strerror(errno)));
  }

  return static_cast<uint64*>(result);
}

Status KernelRegistersLinux::UnmapRegion(FileDescriptor fd,
                                         const MappedRegisterRegion& region) {
  const int ret = munmap(region.registers, region.size);
  if (ret != 0) {
    return InternalError(
        StringPrintf("Error unmapping registers: %s", strerror(errno)));
  }
  return OkStatus();
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
