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

#ifndef DARWINN_DRIVER_MEMORY_BUDDY_ALLOCATOR_H_
#define DARWINN_DRIVER_MEMORY_BUDDY_ALLOCATOR_H_

#include <map>
#include <mutex>  // NOLINT(build/c++11)
#include <set>
#include <vector>

#include "driver/memory/address_space_allocator.h"
#include "port/integral_types.h"
#include "port/status.h"
#include "port/statusor.h"
#include "port/thread_annotations.h"


namespace platforms {
namespace darwinn {
namespace driver {

// A buddy address space allocator.
// https://en.wikipedia.org/wiki/Buddy_memory_allocation
//
// Note that allocations in this buddy allocator are made on 4KB aligned
// boundaries and are 4KB granular in size, even if the requested size is not
// 4KB granular.
//
// Class is thread unsafe.
class BuddyAllocator : public AddressSpaceAllocator {
 public:
  // Constructs an allocator that will allocate from a contiguous address range
  // starting with |address_space_start| and of size |address_space_size_bytes|.
  // Allocations are always aligned on 4KB boundaries and are increments of 4KB
  // in size.
  BuddyAllocator(uint64 address_space_start, uint64 address_space_size_bytes);

  ~BuddyAllocator() override = default;

  //////////////////////////////////////////////////////////////////////////////
  // Implementation of Allocator interface
  //
  util::StatusOr<uint64> Allocate(uint64 size_bytes) override
      LOCKS_EXCLUDED(mutex_);
  util::Status Free(uint64 address, uint64 size_bytes) override
      LOCKS_EXCLUDED(mutex_);

 private:
  // Starting address of the space being allocated from.
  const uint64 address_space_start_;

  // Sets of free blocks, indexed by order.
  std::vector<std::set<uint64>> free_blocks_ GUARDED_BY(mutex_);

  // Sets of allocated blocks, indexed by order.
  std::vector<std::set<uint64>> allocated_blocks_ GUARDED_BY(mutex_);

  mutable std::mutex mutex_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_MEMORY_BUDDY_ALLOCATOR_H_
