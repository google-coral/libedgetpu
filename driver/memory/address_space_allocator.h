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

#ifndef DARWINN_DRIVER_MEMORY_ADDRESS_SPACE_ALLOCATOR_H_
#define DARWINN_DRIVER_MEMORY_ADDRESS_SPACE_ALLOCATOR_H_

#include "port/integral_types.h"
#include "port/status.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Performs allocations within an address space.
class AddressSpaceAllocator {
 public:
  virtual ~AddressSpaceAllocator() = default;

  // Allocates |size_bytes| bytes of address space and returns the base address
  // of the allocation.
  virtual util::StatusOr<uint64> Allocate(uint64 size_bytes) = 0;

  // Frees the allocation with base address |address| and of size |size_bytes|.
  virtual util::Status Free(uint64 address, uint64 size_bytes) = 0;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_MEMORY_ADDRESS_SPACE_ALLOCATOR_H_
