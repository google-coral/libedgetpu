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

#ifndef DARWINN_DRIVER_MEMORY_ADDRESS_SPACE_H_
#define DARWINN_DRIVER_MEMORY_ADDRESS_SPACE_H_

#include <stddef.h>
#include <memory>

#include "api/buffer.h"
#include "driver/device_buffer.h"
#include "driver/memory/dma_direction.h"
#include "port/integral_types.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {
namespace driver {

// A hint that the implementation should use a particular type of address
// space mapping, for systems that have multiple mapping types.
enum class MappingTypeHint {
  // No preference. Most mappings should be of this type.
  kAny,

  // Use simple address space mappings, if the hardware is capable.
  kSimple,

  // Use extended address space mappings, if the hardware is capable.
  kExtended,
};

// An interface for managing a DarwiNN virtual address space segment.
class AddressSpace {
 public:
  AddressSpace() = default;

  // This class is neither copyable nor movable.
  AddressSpace(const AddressSpace&) = delete;
  AddressSpace& operator=(const AddressSpace&) = delete;

  virtual ~AddressSpace() = default;

  // Maps the buffer to the device buffer. Returns the mapped device
  // buffer on success.
  StatusOr<DeviceBuffer> MapMemory(const Buffer& buffer) {
    return MapMemory(buffer, DmaDirection::kBidirectional,
                     MappingTypeHint::kAny);
  }

  // Same as above but with a hint indicating the buffer transfer direction and
  // a hint indicating whether to use simple or extended mappings.
  virtual StatusOr<DeviceBuffer> MapMemory(const Buffer& buffer,
                                           DmaDirection direction,
                                           MappingTypeHint mapping_type) = 0;

  // Same as above but for coherent memory, which may be mapped differently.
  virtual StatusOr<DeviceBuffer> MapCoherentMemory(
      const Buffer& buffer, DmaDirection direction,
      MappingTypeHint mapping_type) {
    return MapMemory(buffer, direction, mapping_type);
  }

  // Unmaps the given device buffer.
  virtual Status UnmapMemory(DeviceBuffer buffer) = 0;

  // Same as above but for coherent memory, which may be handled differently.
  virtual Status UnmapCoherentMemory(DeviceBuffer buffer) {
    return UnmapMemory(buffer);
  }
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_MEMORY_ADDRESS_SPACE_H_
