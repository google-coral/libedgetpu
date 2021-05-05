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

#ifndef DARWINN_DRIVER_MEMORY_DUAL_ADDRESS_SPACE_H_
#define DARWINN_DRIVER_MEMORY_DUAL_ADDRESS_SPACE_H_

#include "driver/config/chip_structures.h"
#include "driver/memory/address_space.h"
#include "driver/memory/mmu_mapper.h"

namespace platforms {
namespace darwinn {
namespace driver {

// An address space implementation that works with a split simple/extended page
// table.
class DualAddressSpace final : public AddressSpace {
 public:
  using AddressSpace::MapMemory;  // Allows for proper overload resolution.

  DualAddressSpace(const config::ChipStructures& chip_structures,
                   MmuMapper* mmu_mapper);

  // This class is neither copyable nor movable.
  DualAddressSpace(const DualAddressSpace&) = delete;
  DualAddressSpace& operator=(const DualAddressSpace&) = delete;

  virtual ~DualAddressSpace() = default;

  // Maps the buffer to the device buffer. Returns the mapped device
  // buffer on success.
  StatusOr<DeviceBuffer> MapMemory(const Buffer& buffer, DmaDirection direction,
                                   MappingTypeHint mapping_type) override;

  // Unmaps the given device buffer.
  Status UnmapMemory(DeviceBuffer buffer) override;

 private:
  // Determines which address space the device buffer was allocated from.
  AddressSpace* DetermineSource(const DeviceBuffer& device_buffer) const;

  // Underlying simple address space.
  std::unique_ptr<AddressSpace> simple_;

  // Underlying extended address space.
  std::unique_ptr<AddressSpace> extended_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_MEMORY_DUAL_ADDRESS_SPACE_H_
