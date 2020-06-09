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

#ifndef DARWINN_DRIVER_MEMORY_BUDDY_ADDRESS_SPACE_H_
#define DARWINN_DRIVER_MEMORY_BUDDY_ADDRESS_SPACE_H_

#include <stddef.h>

#include <mutex>  // NOLINT

#include "api/buffer.h"
#include "driver/memory/buddy_allocator.h"
#include "driver/memory/dma_direction.h"
#include "driver/memory/mmio_address_space.h"
#include "driver/memory/mmu_mapper.h"
#include "port/integral_types.h"
#include "port/statusor.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// A buddy memory allocator for DarwiNN virtual address space segment.
// https://en.wikipedia.org/wiki/Buddy_memory_allocation
class BuddyAddressSpace final : public MmioAddressSpace {
 public:
  using AddressSpace::MapMemory;  // Allows for proper overload resolution.

  BuddyAddressSpace(uint64 device_virtual_address_start,
                    uint64 device_virtual_address_size_bytes,
                    MmuMapper* mmu_mapper);

  // This class is neither copyable nor movable.
  BuddyAddressSpace(const BuddyAddressSpace&) = delete;
  BuddyAddressSpace& operator=(const BuddyAddressSpace&) = delete;

  ~BuddyAddressSpace() override = default;

  // Maps the given host buffer to the device buffer. Returns the mapped device
  // buffer on success.
  util::StatusOr<DeviceBuffer> MapMemory(const Buffer& buffer,
                                         DmaDirection direction,
                                         MappingTypeHint mapping_type) override
      LOCKS_EXCLUDED(mutex_);

  // Unmaps the given device buffer.
  util::Status UnmapMemory(DeviceBuffer buffer) override LOCKS_EXCLUDED(mutex_);

 private:
  mutable std::mutex mutex_;

  // Allocator that manages address space resources.
  BuddyAllocator allocator_ GUARDED_BY(mutex_);
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_MEMORY_BUDDY_ADDRESS_SPACE_H_
