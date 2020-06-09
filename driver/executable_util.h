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

#ifndef DARWINN_DRIVER_EXECUTABLE_UTIL_H_
#define DARWINN_DRIVER_EXECUTABLE_UTIL_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "api/buffer.h"
#include "executable/executable_generated.h"
#include "port/array_slice.h"
#include "port/integral_types.h"
#include "port/status.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Utility functions for working with executable.fbs.
class ExecutableUtil {
 public:
  // Processes the input instruction stream and generates an output instruction
  // stream with the meta fields populated with the given scratch address. Due
  // to the way flatbuffers are packed, field_offsets can be nullptr which is
  // treated the same as empty vector in this function.
  static void LinkScratchAddress(
      uint64 scratch_address,
      const flatbuffers::Vector<flatbuffers::Offset<darwinn::FieldOffset>>*
          field_offsets,
      gtl::MutableArraySlice<uint8> encoded_buffer);

  // Processes the input instruction stream and generates an output instruction
  // stream with the meta fields populated with the given host addresses. Due
  // to the way flatbuffers are packed, field_offsets can be nullptr which is
  // treated the same as empty vector in this function.
  static void LinkParameterAddress(
      uint64 parameter_address,
      const flatbuffers::Vector<flatbuffers::Offset<darwinn::FieldOffset>>*
          field_offsets,
      gtl::MutableArraySlice<uint8> encoded_buffer);

  static void LinkInputAddress(
      const std::string& input_name, const std::vector<uint64>& input_addresses,
      const flatbuffers::Vector<flatbuffers::Offset<darwinn::FieldOffset>>*
          field_offsets,
      gtl::MutableArraySlice<uint8> encoded_buffer);

  static void LinkOutputAddress(
      const std::string& output_name,
      const std::vector<uint64>& output_addresses,
      const flatbuffers::Vector<flatbuffers::Offset<darwinn::FieldOffset>>*
          field_offsets,
      gtl::MutableArraySlice<uint8> encoded_buffer);

  // Convenience function to set a uint32 value on the specified bitoffset.
  static void CopyUint32(gtl::MutableArraySlice<uint8> buffer, int offset_bit,
                         uint32 value);

 private:
  // Static class.
  ExecutableUtil();
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_EXECUTABLE_UTIL_H_
