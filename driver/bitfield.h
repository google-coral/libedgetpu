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

// Helper class to get/set command status register (CSR) fields. Assumes 64-bit
// CSR registers.
//
// Usage:
//   Bitfield<2, 3> field;  // Read and write to 0b000XXX00 part of a byte
//
//   // Access entire value with raw_value or read/write individual fields.
//   // Unused bitfield can be left out. Bitfields are uninitialized because
//   // they are expected to be used within union.
//   union {
//     uint64 raw_value;
//     Bitfield<0, 1> enable;
//     Bitfield<1, 9> status;
//   } reg;
//
//   // Explicitly assign default value if exists.
//   reg.enable = 1;
//   reg.status = 0xF;
//
// LSB_POSITION defines the starting bit of the field specified from the LSB.
// BITS defines the length of the field. Writes to the field that have bits set
// outside of BITS length will cause an error. The values passed in for setting
// and returned from reading will be right aligned (BITS bits starting from the
// LSB).

#ifndef DARWINN_DRIVER_BITFIELD_H_
#define DARWINN_DRIVER_BITFIELD_H_

#include <limits.h>
#include <stddef.h>

#include <limits>

#include "port/integral_types.h"
#include "port/logging.h"

namespace platforms {
namespace darwinn {
namespace driver {

template <int LSB_POSITION, int NUM_BITS>
class Bitfield {
 public:
  // Sets the bitfield to |value|. |value| is right aligned, and should set bits
  // in the range of NUM_BITS.
  Bitfield& operator=(uint64 value) {
    CHECK_EQ(value & kMask, value);

    // Since Bitfield is expected to be used with unions, other bits from value
    // must be preserved.
    uint64 preserved_bits = value_ & ~(kMask << LSB_POSITION);
    value_ = preserved_bits | (value << LSB_POSITION);
    return *this;
  }

  // Returns the value in a right aligned form.
  constexpr uint64 operator()() const {
    return (value_ >> LSB_POSITION) & kMask;
  }

  // Returns mask for Bitfield.
  constexpr uint64 mask() const {
    return kMask;
  }

 private:
  // Supported bits in the underlying value.
  static constexpr size_t kMaxBits = sizeof(uint64) * CHAR_BIT;
  static_assert(NUM_BITS > 0, "Bitfield must use at least 1 bit");
  static_assert(NUM_BITS <= kMaxBits,
                "Bitfield cannot have more bits than 64 bits");
  static_assert(LSB_POSITION < kMaxBits,
                "Bitfield cannot start at LSB position higher than 63-bit");
  static_assert(LSB_POSITION + NUM_BITS <= kMaxBits,
                "Bitfield cannot have its MSB position past 64-bit");

  // Any attempt to write outside of kMask will cause an error.
  static constexpr uint64 kMask =
      (NUM_BITS == kMaxBits) ? (std::numeric_limits<uint64>::max())
                             : (1ULL << NUM_BITS) - 1;

  // Underlying value for Bitfield.
  uint64 value_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_BITFIELD_H_
