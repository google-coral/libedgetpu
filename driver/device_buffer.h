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

#ifndef DARWINN_DRIVER_DEVICE_BUFFER_H_
#define DARWINN_DRIVER_DEVICE_BUFFER_H_

#include <stddef.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "port/integral_types.h"
#include "port/logging.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Abstracts a device addressable buffer. Movable and copyable.
class DeviceBuffer {
 public:
  // Convenience structure for keeping track of named array of DeviceBuffers.
  using NamedMap = std::unordered_map<std::string, std::vector<DeviceBuffer>>;

  // Default constructor. Defaults to an invalid non-existent buffer.
  DeviceBuffer() = default;

  // Constructors for a device accessible buffer.
  DeviceBuffer(uint64 device_address, size_t size_bytes);

  // This type is copyable, with default implementations.
  DeviceBuffer(const DeviceBuffer&) = default;
  DeviceBuffer& operator=(const DeviceBuffer&) = default;

  // This type is movable.
  DeviceBuffer(DeviceBuffer&& other);
  DeviceBuffer& operator=(DeviceBuffer&& other);

  // Destructors.
  ~DeviceBuffer() = default;

  // Size of this buffer in bytes.
  size_t size_bytes() const { return size_bytes_; }

  // Returns true if buffer is valid.
  bool IsValid() const { return type_ != Type::kInvalid; }

  // Returns the device address.
  uint64 device_address() const { return device_address_; }

  // Equality operators.
  bool operator==(const DeviceBuffer& rhs) const;
  bool operator!=(const DeviceBuffer& rhs) const;

  // Returns a DeviceBuffer that starts from the "byte_offset" and consumes
  // "size_bytes". Internally fails if created DeviceBuffer accesses outside of
  // current DeviceBuffer and "allow_overflow" is false (the default).
  DeviceBuffer Slice(uint64 byte_offset, size_t size_bytes,
                     bool allow_overflow = false) const;

 private:
  // Type for the buffer.
  enum class Type {
    // Invalid.
    kInvalid = 0,

    // Default device buffer (only one type for now.)
    kDefault = 1,
  };

  // Clears all variables.
  void Clear();

  // Type for the buffer.
  Type type_{Type::kInvalid};

  // Size of the buffer.
  size_t size_bytes_{0};

  // Points to device addressable buffer. Valid when type is kDeviceBuffer.
  uint64 device_address_{0};
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_DEVICE_BUFFER_H_
