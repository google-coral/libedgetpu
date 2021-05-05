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

#include "driver/memory/nop_address_space.h"

#include "api/buffer.h"
#include "port/errors.h"
#include "port/ptr_util.h"

namespace platforms {
namespace darwinn {
namespace driver {

StatusOr<DeviceBuffer> NopAddressSpace::MapMemory(
    const Buffer& buffer, DmaDirection direction,
    MappingTypeHint mapping_type) {
  if (!buffer.IsValid()) {
    return InvalidArgumentError("Invalid buffer.");
  }

  return DeviceBuffer(reinterpret_cast<uint64>(buffer.ptr()),
                      buffer.size_bytes());
}

StatusOr<const Buffer> NopAddressSpace::Translate(
    const DeviceBuffer& buffer) const {
  return Buffer(reinterpret_cast<uint8*>(buffer.device_address()),
                buffer.size_bytes());
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
