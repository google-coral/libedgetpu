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

#include "driver/aligned_allocator.h"

#include <functional>
#include <memory>

#include "port/aligned_malloc.h"
#include "port/integral_types.h"
#include "port/logging.h"

namespace platforms {
namespace darwinn {
namespace driver {

AlignedAllocator::AlignedAllocator(uint64 alignment_bytes)
    : alignment_bytes_(alignment_bytes) {
  // Check for power of 2, since we use arithmetic that relies on it elsewhere.
  CHECK_EQ((alignment_bytes - 1) & alignment_bytes, 0);
}

void* AlignedAllocator::Allocate(size_t size) {
  int aligned_size = (size + alignment_bytes_ - 1) & ~(alignment_bytes_ - 1);
  return aligned_malloc(aligned_size, alignment_bytes_);
}

void AlignedAllocator::Free(void* aligned_memory) {
  aligned_free(aligned_memory);
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
