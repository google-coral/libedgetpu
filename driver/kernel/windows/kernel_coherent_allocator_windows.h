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

#ifndef DARWINN_DRIVER_KERNEL_WINDOWS_KERNEL_COHERENT_ALLOCATOR_WINDOWS_H_
#define DARWINN_DRIVER_KERNEL_WINDOWS_KERNEL_COHERENT_ALLOCATOR_WINDOWS_H_

#include "driver/kernel/kernel_coherent_allocator.h"
#include "port/fileio.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Functions to allocate coherent memory that is DMA-able by a Darwinn device.
class KernelCoherentAllocatorWindows : public KernelCoherentAllocator {
 public:
  KernelCoherentAllocatorWindows(const std::string &device_path,
                                 int alignment_bytes, size_t size_bytes);

 private:
  // Maps and unmaps kernel allocated memory block to user space.
  StatusOr<char *> Map(FileDescriptor fd, size_t size_bytes,
                       uint64 dma_address) override;
  Status Unmap(FileDescriptor fd, char *mem_base, size_t size_bytes) override;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_KERNEL_WINDOWS_KERNEL_COHERENT_ALLOCATOR_WINDOWS_H_
