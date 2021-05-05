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

#ifndef DARWINN_DRIVER_MEMORY_NOP_ADDRESS_SPACE_H_
#define DARWINN_DRIVER_MEMORY_NOP_ADDRESS_SPACE_H_

#include <stddef.h>
#include <memory>

#include "api/buffer.h"
#include "driver/device_buffer.h"
#include "driver/memory/address_space.h"
#include "driver/memory/dma_direction.h"
#include "port/integral_types.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {
namespace driver {

// No-op address space implementation. MapMemory and UnmapMemory is no-op. Host
// address equals to device virtual address.
class NopAddressSpace : public AddressSpace {
 public:
  using AddressSpace::MapMemory;  // Allows for proper overload resolution.

  NopAddressSpace() = default;

  // This class is neither copyable nor movable.
  NopAddressSpace(const NopAddressSpace&) = delete;
  NopAddressSpace& operator=(const NopAddressSpace&) = delete;

  virtual ~NopAddressSpace() = default;

  // Maps the given host buffer to the device buffer. Returns the mapped device
  // buffer on success.
  StatusOr<DeviceBuffer> MapMemory(const Buffer& buffer, DmaDirection direction,
                                   MappingTypeHint mapping_type) override;

  // Unmaps the given device address range.
  Status UnmapMemory(DeviceBuffer buffer) override {
    return Status();  // OK
  }

  // Translates device buffer to host buffer.
  StatusOr<const Buffer> Translate(const DeviceBuffer& buffer) const;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_MEMORY_NOP_ADDRESS_SPACE_H_
