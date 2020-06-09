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

#include "driver/memory/dual_address_space.h"

#include "driver/hardware_structures.h"
#include "driver/memory/buddy_address_space.h"
#include "port/ptr_util.h"

namespace platforms {
namespace darwinn {
namespace driver {

DualAddressSpace::DualAddressSpace(
    const config::ChipStructures& chip_structures, MmuMapper* mmu_mapper) {
  const int num_simple_entries =
      GetNumSimplePageTableEntries(chip_structures.num_page_table_entries);

  simple_ = gtl::MakeUnique<BuddyAddressSpace>(
      0, kHostPageSize * num_simple_entries, mmu_mapper);

  extended_ = gtl::MakeUnique<BuddyAddressSpace>(
      kExtendedAddressSpaceStart,
      kExtendedPageTableEntryAddressableBytes *
          GetNumExtendedPageTableEntries(
              chip_structures.num_page_table_entries),
      mmu_mapper);
}

util::StatusOr<DeviceBuffer> DualAddressSpace::MapMemory(
    const Buffer& buffer, DmaDirection direction,
    MappingTypeHint mapping_type) {
  switch (mapping_type) {
    case MappingTypeHint::kSimple:
      return simple_->MapMemory(buffer, direction, mapping_type);

    case MappingTypeHint::kExtended:
    case MappingTypeHint::kAny:
      return extended_->MapMemory(buffer, direction, mapping_type);
  }
}

util::Status DualAddressSpace::UnmapMemory(DeviceBuffer buffer) {
  return DetermineSource(buffer)->UnmapMemory(buffer);
}

AddressSpace* DualAddressSpace::DetermineSource(
    const DeviceBuffer& device_buffer) const {
  if (device_buffer.device_address() & kExtendedVirtualAddressBit) {
    return extended_.get();
  } else {
    return simple_.get();
  }
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
