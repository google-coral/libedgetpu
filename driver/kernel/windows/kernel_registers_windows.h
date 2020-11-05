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

#ifndef DARWINN_DRIVER_KERNEL_WINDOWS_KERNEL_REGISTERS_WINDOWS_H_
#define DARWINN_DRIVER_KERNEL_WINDOWS_KERNEL_REGISTERS_WINDOWS_H_

#include <mutex>  // NOLINT
#include <string>
#include <vector>

#include "driver/kernel/kernel_registers.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Kernel implementation of the register interface.
class KernelRegistersWindows : public KernelRegisters {
 public:
  KernelRegistersWindows(const std::string& device_path,
                         const std::vector<MmapRegion>& mmap_region,
                         bool read_only);

  KernelRegistersWindows(const std::string& device_path, uint64 mmap_offset,
                         uint64 mmap_size, bool read_only);
  ~KernelRegistersWindows() override;

 protected:
  util::StatusOr<uint64*> MapRegion(FileDescriptor fd,
                                    const MappedRegisterRegion& region,
                                    bool read_only) override;
  util::Status UnmapRegion(FileDescriptor fd,
                           const MappedRegisterRegion& region) override;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_KERNEL_WINDOWS_KERNEL_REGISTERS_WINDOWS_H_
