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

// Utility functions for working with executable.fbs.

#include "driver/executable_util.h"

#include <limits.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "api/buffer.h"
#include "executable/executable_generated.h"
#include "port/array_slice.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/logging.h"
#include "port/macros.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/statusor.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace {

using ::flatbuffers::Offset;
using ::flatbuffers::Vector;

// Align n to the nearest multiple of n, where n is a power of 2.
int AlignNext(int value, int n) {
  CHECK_EQ(0, n & (n - 1));  // must be power of 2.
  return (value + n) & ~(n - 1);
}

// Copy the low |num_bits| from |src| into |dst| at offset |dst_offset_bits|.
// |dst_offset_bits + num_bits| must be less than or equal to 8.
// Returns the original |src| but with the low |num_bits| bits shifted out.
uint32 CopyUint8LowBits(uint32 src, int dst_offset_bit, int num_bits,
                        uint8* dst) {
  CHECK_LE(dst_offset_bit + num_bits, CHAR_BIT);

  // Mask the low |num_bits| bits from |src| and assign it to |dst| at offset
  // |dst_offset_bit|.
  const uint8 src_mask = (1 << num_bits) - 1;
  const uint8 dst_mask = src_mask << dst_offset_bit;

  *dst = (*dst & ~dst_mask) | (src & src_mask) << dst_offset_bit;

  return src >> num_bits;  // shift out bits already set.
}

void LinkBatchedAddress(Description target, const std::string& name,
                        const std::vector<uint64>& addresses,
                        const Vector<Offset<FieldOffset>>* field_offsets,
                        gtl::MutableArraySlice<uint8> encoded_buffer) {
  if (field_offsets == nullptr) {
    return;
  }

  for (const auto& field_offset : *field_offsets) {
    const auto& meta = field_offset->meta();
    if (meta->desc() != target) {
      continue;
    }

    if (meta->name()->str() != name) {
      continue;
    }

    const int batch = meta->batch();
    CHECK(batch < addresses.size());
    const uint64 link_address = addresses[batch];

    uint32 immediate_value;
    if (meta->position() == Position_LOWER_32BIT) {
      VLOG(3) << StringPrintf(
          "Linking %s[%d]: 0x%016llx", name.c_str(), batch,
          static_cast<unsigned long long>(  // NOLINT(runtime/int)
              link_address));
      immediate_value = link_address & kuint32max;
    } else {
      CHECK_EQ(meta->position(), Position_UPPER_32BIT);
      immediate_value = (link_address >> 32) & kuint32max;
    }
    ExecutableUtil::CopyUint32(encoded_buffer, field_offset->offset_bit(),
                               immediate_value);
  }
}

}  // namespace

void ExecutableUtil::CopyUint32(gtl::MutableArraySlice<uint8> buffer,
                                int offset_bit, uint32 original_value) {
  // Track current destination bit offset.
  int next_dst_offset_bit = offset_bit;

  // Tracks remaining bits that needs to be set.
  int remaining_bits = sizeof(original_value) * CHAR_BIT;

  // Value that needs to be set, bits that are set are shifted out.
  int next_value = original_value;

  while (remaining_bits > 0) {
    // Sets enough bits to align to the next 8 bit boundary.
    int num_bits_to_set =
        std::min(AlignNext(next_dst_offset_bit, CHAR_BIT) - next_dst_offset_bit,
                 remaining_bits);

    // Offset byte and bit offset with in the byte.
    int dst_byte = next_dst_offset_bit / CHAR_BIT;
    int dst_bit = next_dst_offset_bit % CHAR_BIT;

    // Copy lower |num_bits_to_set| from next_value into the destination byte
    // at the specified offset.
    next_value = CopyUint8LowBits(next_value, dst_bit, num_bits_to_set,
                                  &buffer[dst_byte]);

    remaining_bits -= num_bits_to_set;
    next_dst_offset_bit += num_bits_to_set;
  }
}

void ExecutableUtil::LinkScratchAddress(
    uint64 scratch_address, const Vector<Offset<FieldOffset>>* field_offsets,
    gtl::MutableArraySlice<uint8> encoded_buffer) {
  if (field_offsets == nullptr) {
    return;
  }

  for (const auto& field_offset : *field_offsets) {
    const auto& meta = field_offset->meta();
    if (meta->desc() != Description_BASE_ADDRESS_SCRATCH) {
      continue;
    }

    // TODO: Add support for batch.
    CHECK_EQ(meta->batch(), 0);

    uint32 immediate_value;
    if (meta->position() == Position_LOWER_32BIT) {
      VLOG(3) << StringPrintf(
          "Linking Scratch: 0x%016llx",
          static_cast<unsigned long long>(  // NOLINT(runtime/int)
              scratch_address));
      immediate_value = scratch_address & kuint32max;
    } else {
      CHECK_EQ(meta->position(), Position_UPPER_32BIT);
      immediate_value = (scratch_address >> 32) & kuint32max;
    }

    CopyUint32(encoded_buffer, field_offset->offset_bit(), immediate_value);
  }
}

void ExecutableUtil::LinkParameterAddress(
    uint64 parameter_address, const Vector<Offset<FieldOffset>>* field_offsets,
    gtl::MutableArraySlice<uint8> encoded_buffer) {
  if (field_offsets == nullptr) {
    return;
  }

  for (const auto& field_offset : *field_offsets) {
    const auto& meta = field_offset->meta();
    if (meta->desc() != Description_BASE_ADDRESS_PARAMETER) {
      continue;
    }

    uint32 immediate_value;
    if (meta->position() == Position_LOWER_32BIT) {
      VLOG(3) << StringPrintf(
          "Linking Parameter: 0x%016llx",
          static_cast<unsigned long long>(  // NOLINT(runtime/int)
              parameter_address));
      immediate_value = parameter_address & kuint32max;
    } else {
      CHECK_EQ(meta->position(), Position_UPPER_32BIT);
      immediate_value = (parameter_address >> 32) & kuint32max;
    }

    CopyUint32(encoded_buffer, field_offset->offset_bit(), immediate_value);
  }
}

void ExecutableUtil::LinkInputAddress(
    const std::string& input_name, const std::vector<uint64>& input_addresses,
    const Vector<Offset<FieldOffset>>* field_offsets,
    gtl::MutableArraySlice<uint8> encoded_buffer) {
  LinkBatchedAddress(Description_BASE_ADDRESS_INPUT_ACTIVATION, input_name,
                     input_addresses, field_offsets, encoded_buffer);
}

void ExecutableUtil::LinkOutputAddress(
    const std::string& output_name, const std::vector<uint64>& output_addresses,
    const Vector<Offset<FieldOffset>>* field_offsets,
    gtl::MutableArraySlice<uint8> encoded_buffer) {
  LinkBatchedAddress(Description_BASE_ADDRESS_OUTPUT_ACTIVATION, output_name,
                     output_addresses, field_offsets, encoded_buffer);
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
