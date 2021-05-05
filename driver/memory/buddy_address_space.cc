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

#include "driver/memory/buddy_address_space.h"

#include "api/buffer.h"
#include "driver/memory/address_utilities.h"
#include "driver/memory/mmio_address_space.h"
#include "driver/memory/mmu_mapper.h"
#include "port/cleanup.h"
#include "port/errors.h"
#include "port/logging.h"
#include "port/ptr_util.h"
#include "port/status_macros.h"
#include "port/statusor.h"
#include "port/std_mutex_lock.h"
#include "port/tracing.h"

namespace platforms {
namespace darwinn {
namespace driver {

BuddyAddressSpace::BuddyAddressSpace(uint64 device_virtual_address_start,
                                     uint64 device_virtual_address_size_bytes,
                                     MmuMapper* mmu_mapper)
    : MmioAddressSpace(device_virtual_address_start,
                       device_virtual_address_size_bytes, mmu_mapper),
      allocator_(device_virtual_address_start,
                 device_virtual_address_size_bytes) {}

StatusOr<DeviceBuffer> BuddyAddressSpace::MapMemory(
    const Buffer& buffer, DmaDirection direction,
    MappingTypeHint mapping_type) {
  TRACE_SCOPE("BuddyAddressSpace::MapMemory");
  const void* ptr = buffer.IsPtrType() ? buffer.ptr() : nullptr;
  if (!ptr && buffer.IsPtrType()) {
    return InvalidArgumentError(
        "Cannot map an invalid host-memory-backed Buffer.");
  }

  const size_t size_bytes = buffer.size_bytes();
  if (size_bytes == 0) {
    return InvalidArgumentError("Cannot map 0 bytes.");
  }

  auto num_requested_pages = GetNumberPages(ptr, size_bytes);
  const uint64 allocation_size = num_requested_pages * kHostPageSize;

  StdMutexLock lock(&mutex_);
  ASSIGN_OR_RETURN(uint64 device_va, allocator_.Allocate(allocation_size));

  // Make sure the allocation if freed upon error.
  auto cleanup = MakeCleanup(
      [this, device_va, allocation_size]() EXCLUSIVE_LOCKS_REQUIRED(mutex_) {
        CHECK_OK(allocator_.Free(device_va, allocation_size));
      });
  RETURN_IF_ERROR(Map(buffer, device_va, direction));

  cleanup.release();
  return DeviceBuffer(device_va + GetPageOffset(ptr), size_bytes);
}

Status BuddyAddressSpace::UnmapMemory(DeviceBuffer buffer) {
  TRACE_SCOPE("BuddyAddressSpace::UnmapMemory");
  StdMutexLock lock(&mutex_);
  const uint64 device_address = buffer.device_address();
  const size_t size_bytes = buffer.size_bytes();

  auto num_pages = GetNumberPages(device_address, size_bytes);
  const uint64 allocation_size = num_pages * kHostPageSize;
  const uint64 device_aligned_va = GetPageAddress(device_address);

  RETURN_IF_ERROR(Unmap(device_aligned_va, num_pages));
  RETURN_IF_ERROR(allocator_.Free(device_aligned_va, allocation_size));

  return Status();  // OK.
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
