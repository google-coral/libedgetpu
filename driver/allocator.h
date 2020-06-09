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

#ifndef DARWINN_DRIVER_ALLOCATOR_H_
#define DARWINN_DRIVER_ALLOCATOR_H_

#include <stddef.h>

#include "api/buffer.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Interface for a class that can allocate host memory.
class Allocator {
 public:
  virtual ~Allocator() = default;

  // Allocates buffer of specified size.
  virtual void* Allocate(size_t size) = 0;

  // Frees a previous allocated buffer.
  virtual void Free(void* buffer) = 0;

  // Allocates and returns a buffer of the specified size. The lifecycle of the
  // the returned buffer is tied to the Allocator instance. It is thus important
  // to ensure that the allocator class outlives the returned buffer instances.
  Buffer MakeBuffer(size_t size_bytes);
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_ALLOCATOR_H_
