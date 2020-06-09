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

#ifndef DARWINN_DRIVER_ALIGNED_ALLOCATOR_H_
#define DARWINN_DRIVER_ALIGNED_ALLOCATOR_H_

#include <memory>

#include "driver/allocator.h"
#include "port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Convenience class to allocate aligned buffers.
class AlignedAllocator : public Allocator {
 public:
  // All allocated buffers will be aligned to |alignment_bytes| with a size
  // granulairy of |alignment_bytes|.
  explicit AlignedAllocator(uint64 alignment_bytes);
  ~AlignedAllocator() = default;

  // This class is neither copyable nor movable.
  AlignedAllocator(const AlignedAllocator&) = delete;
  AlignedAllocator& operator=(const AlignedAllocator&) = delete;

  void* Allocate(size_t size) override;
  void Free(void* aligned_memory) override;

 private:
  // Alignment
  const uint64 alignment_bytes_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_ALIGNED_ALLOCATOR_H_
