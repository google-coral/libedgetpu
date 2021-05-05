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

#ifndef DARWINN_DRIVER_MEMORY_DRAM_ALLOCATOR_H_
#define DARWINN_DRIVER_MEMORY_DRAM_ALLOCATOR_H_

#include "api/dram_buffer.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {
namespace driver {

// An abstract class for DRAM allocator. Each chip will have a concrete
// implementation.
class DramAllocator {
 public:
  DramAllocator() = default;
  virtual ~DramAllocator() = default;

  // This class is neither copyable nor movable.
  DramAllocator(const DramAllocator&) = delete;
  DramAllocator& operator=(const DramAllocator&) = delete;

  // Open and close the allocator. Buffer allocation can happen even when the
  // allocator is closed but thos buffers should not be used when allocator is
  // closed.
  virtual Status Open() = 0;
  virtual Status Close() = 0;

  // Allocates and returns a DRAM buffer of requested size. It returns an error
  // if there is not enough space.
  virtual StatusOr<std::shared_ptr<DramBuffer>> AllocateBuffer(
      size_t size_bytes) = 0;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_MEMORY_DRAM_ALLOCATOR_H_
