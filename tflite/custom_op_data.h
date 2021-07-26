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

#ifndef DARWINN_TFLITE_CUSTOM_OP_DATA_H_
#define DARWINN_TFLITE_CUSTOM_OP_DATA_H_

#include <cstdint>

#include "flatbuffers/flexbuffers.h"
#include "api/chip.h"
#include "tflite/custom_op_wrapped_buffer.h"
#include "flatbuffers/flatbuffers.h"

namespace platforms {
namespace darwinn {
namespace tflite {

// Version 0.
static const int32_t kCustomOpDataVersion = 0;

struct CustomOpData {
  int32_t version;
  // TODO Remove this field and update references in
  // custom_op_data.cc.
  // The field is only used by 1.0.
  CustomOpWrappedBuffer parameter_caching_executable;

  std::vector<CustomOpWrappedBuffer> executables;

  // Execution preference code (currently used by NNAPI only).
  int32_t execution_preference = -1;
};

// Converts the input CustomOpData object into a flexbuffers object that stores
// it in a serializable form.
// This function expects the caller to provide a valid CustomOpData object.
std::unique_ptr<flexbuffers::Builder> SerializeCustomOpData(
    const CustomOpData& custom_op_data);

// Converts the input buffer into an in-memory CustomOpData object.
// Returns nullptr if buffer is null or if length is zero.
std::unique_ptr<CustomOpData> DeserializeCustomOpData(const uint8_t* buffer,
                                                      size_t length);

}  // namespace tflite
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_TFLITE_CUSTOM_OP_DATA_H_
