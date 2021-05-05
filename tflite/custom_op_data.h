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

namespace platforms {
namespace darwinn {
namespace tflite {

// Version 0.
static const int32_t kCustomOpDataVersion = 0;

struct WrappedBuffer {
  const char* data;
  size_t length;

  // Optional field. Use to specify the target chip of the wrapped executable
  // when there are multiple exectuables in `CustomOpData`.
  api::Chip chip = api::Chip::kUnknown;
};

struct CustomOpData {
  int32_t version;
  // TODO Remove this field and update references in
  // custom_op_data.cc.
  // The field is only used by 1.0.
  WrappedBuffer parameter_caching_executable;

  std::vector<WrappedBuffer> executables;

  // Execution preference code (currently used by NNAPI only). For more
  // information, please see PreferenceCode in:
  // https://cs.corp.google.com/android/frameworks/ml/nn/runtime/include/NeuralNetworks.h
  // -1 results in NNAPI's default value (FAST_SINGLE_ANSWER=1), it is used as
  // default here in order to help identify older custom ops or ones that are
  // created in a code path that does not set execution preference.
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
