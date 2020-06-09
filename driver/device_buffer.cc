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

#include "driver/device_buffer.h"

#include <stddef.h>

#include "port/integral_types.h"
#include "port/logging.h"

namespace platforms {
namespace darwinn {
namespace driver {

void DeviceBuffer::Clear() {
  type_ = Type::kInvalid;
  size_bytes_ = 0;
  device_address_ = 0;
}

DeviceBuffer::DeviceBuffer(uint64 device_address, size_t size_bytes)
    : type_(Type::kDefault),
      size_bytes_(size_bytes),
      device_address_(device_address) {}

bool DeviceBuffer::operator==(const DeviceBuffer& rhs) const {
  return type_ == rhs.type_ && size_bytes_ == rhs.size_bytes_ &&
         device_address_ == rhs.device_address_;
}

bool DeviceBuffer::operator!=(const DeviceBuffer& rhs) const {
  return !(*this == rhs);
}

DeviceBuffer::DeviceBuffer(DeviceBuffer&& other)
    : type_(other.type_),
      size_bytes_(other.size_bytes_),
      device_address_(other.device_address_) {
  other.Clear();
}

DeviceBuffer& DeviceBuffer::operator=(DeviceBuffer&& other) {
  if (this != &other) {
    type_ = other.type_;
    size_bytes_ = other.size_bytes_;
    device_address_ = other.device_address_;

    other.Clear();
  }
  return *this;
}

DeviceBuffer DeviceBuffer::Slice(uint64 byte_offset, size_t size_bytes,
                                 bool allow_overflow) const {
  if (!allow_overflow) {
    CHECK_LE(byte_offset + size_bytes, size_bytes_)
        << "Overflowed underlying DeviceBuffer";
  }
  const uint64 new_device_address = device_address_ + byte_offset;
  return DeviceBuffer(new_device_address, size_bytes);
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
