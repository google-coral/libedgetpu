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

#ifndef DARWINN_DRIVER_KERNEL_KERNEL_COHERENT_ALLOCATOR_INTERNAL_H_
#define DARWINN_DRIVER_KERNEL_KERNEL_COHERENT_ALLOCATOR_INTERNAL_H_

#include <fcntl.h>
#include <stddef.h>
#include <sys/types.h>

#include <string>

#include "driver/mmio/coherent_allocator.h"
#include "port/errors.h"
#include "port/fileio.h"
#include "port/integral_types.h"
#include "port/logging.h"
#include "port/statusor.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Functions to allocate coherent memory that is DMA-able by a Darwinn device.
class KernelCoherentAllocator : public CoherentAllocator {
 public:
  KernelCoherentAllocator(const std::string &device_path, int alignment_bytes,
                          size_t size_bytes);
  virtual ~KernelCoherentAllocator() = default;

 protected:
  // Maps and unmaps kernel allocated memory block to user space.
  virtual StatusOr<char *> Map(FileDescriptor fd, size_t size_bytes,
                               uint64 dma_address) = 0;
  virtual Status Unmap(FileDescriptor fd, char *mem_base,
                       size_t size_bytes) = 0;

 private:
  // Implements Open.
  StatusOr<char *> DoOpen(size_t size_bytes) override;

  // Implements close.
  Status DoClose(char *mem_base, size_t size_bytes) override;

  // File descriptor of the opened device.
  FileDescriptor fd_{INVALID_FD_VALUE};

  // Device specific DMA address of the coherent memory block.
  uint64 dma_address_{0};

  // Device path.
  const std::string device_path_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_KERNEL_KERNEL_COHERENT_ALLOCATOR_INTERNAL_H_
