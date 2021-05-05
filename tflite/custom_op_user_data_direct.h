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

#ifndef DARWINN_TFLITE_CUSTOM_OP_USER_DATA_DIRECT_H_
#define DARWINN_TFLITE_CUSTOM_OP_USER_DATA_DIRECT_H_

#include <memory>

#include "absl/container/flat_hash_map.h"
#include "api/driver.h"
#include "driver/package_registry.h"
#include "port/errors.h"
#include "port/ptr_util.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/statusor.h"
#include "tflite/custom_op.h"

namespace platforms {
namespace darwinn {
namespace tflite {

// Node-and-Interpreter-specific custom data. This is allocated by OpInit,
// which is called at Interpreter creation time, per custom op node.
// This object is non-threadsafe, for Interpreter itself is single threaded.
// Pointer to this object instance is needed to de-allocate resources
// associated with this custom op instance in the interpreter context, as
// pointer to upper level node instance is not passed to
// TfLiteRegistration::free callback.
class CustomOpUserDataDirect : public CustomOpUserData {
 public:
  CustomOpUserDataDirect(const uint8_t* buffer, size_t length);

  ~CustomOpUserDataDirect();

  // Binds to a driver instance, and registers executables with this driver.
  Status SetDriver(api::Driver* driver);

  // Returns the reference to the executable binary.
  const api::PackageReference* GetExecutable() const { return executable_; }

  // Returns a map from output tensor index to input tensor index, indicating
  // whether the executable output should be written to one of the input TfLite
  // buffers. This is the case for LSTM models where hidden states are stored in
  // variable tensors.
  const absl::flat_hash_map<int, int>& GetVariableOutputDestination() {
    return variable_output_destination_;
  }

 private:
  // Unregisters executables with the associated driver.
  Status UnregisterExecutables();

  // Raw data parsed from tflite model file.
  std::unique_ptr<CustomOpData> raw_model_data_;

  // Pointer to the driver instance associated with this custom op node.
  // Note that a driver instance can be shared by many custom op nodes, and
  // execution of all of these nodes would be serialized in
  // EdgeTpuContextDirect::InvokeExecutable. Thread safty is guaranteed by
  // Driver.
  api::Driver* driver_{nullptr};

  // Pointer to the reference of the executable binary;
  const api::PackageReference* executable_{nullptr};

  // A map from output tensor index to input tensor index where output tensor
  // content should be written to input tensor's TfLite buffer.
  absl::flat_hash_map<int, int> variable_output_destination_;
};

}  // namespace tflite
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_TFLITE_CUSTOM_OP_USER_DATA_DIRECT_H_
