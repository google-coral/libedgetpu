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

#ifndef DARWINN_TFLITE_CUSTOM_OP_H_
#define DARWINN_TFLITE_CUSTOM_OP_H_

#include "driver/package_registry.h"
#include "tflite/custom_op_data.h"
#include "tensorflow/lite/context.h"

namespace platforms {
namespace darwinn {
namespace tflite {

// This structure encapsulates the data needed to run DarwiNN executables
// after they have been registered with the DarwiNN driver.
class CustomOpUserData {
 public:
  virtual ~CustomOpUserData();

  // Session name determines which hardware/service to use, as well as
  // management of cache shared by all models running on the same hardware.
  // TODO: add session manager to map between name and
  // settings/status/control.
  const std::string& GetSessionName() const;

  const driver::ExecutableLayersInfo* GetExecutableLayersInfo() const {
    return executable_layers_info_;
  }

  TfLiteIntArray* GetInputs() const;
  TfLiteIntArray* GetInputs(TfLiteNode* node) const;
  void SetInputs(TfLiteIntArray* inputs) { inputs_ = inputs; }

  bool GetShouldPopulateCache() const;
  void SetShouldPopulateCache(bool should_populate_cache);

  int GetBatches() const { return batches_; }
  void SetBatches(int batches) { batches_ = batches; }

 protected:
  CustomOpUserData() = default;

  std::string session_name_;
  bool should_populate_cache_{true};

  int batches_{1};

  // Pointer to the layer info of the executable binary;
  const driver::ExecutableLayersInfo* executable_layers_info_{nullptr};

  // When we use the custom-op implementation to run a delegate op, we can't
  // look at the node's inputs to find all the input activation tensors to this
  // delegate op (since the node's inputs include all the bias/parameter
  // tensors as well). Instead we will look at this array.
  TfLiteIntArray* inputs_{nullptr};
};

// Returns input tensor from node.
TfLiteTensor* GetInput(TfLiteContext* context, TfLiteNode* node, int index);

// Returns output tensor from node.
TfLiteTensor* GetOutput(TfLiteContext* context, TfLiteNode* node, int index);

// Re-format output data.
util::Status ReFormatOutputs(TfLiteTensor* output, int output_tensor_offset,
                             int output_tensor_size,
                             const api::OutputLayerInformation* output_layer,
                             const unsigned char* output_data);

// Prepares custom-op for operation.
TfLiteStatus CustomOpPrepare(TfLiteContext* context, TfLiteNode* node);

// Deallocates the custom-op data object (CustomOpUserData). The lifetime of
// this object is managed by the TfLite interpreter, which calls this function
// to deallocate this object.
void CustomOpFree(TfLiteContext* context, void* buffer);

}  // namespace tflite
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_TFLITE_CUSTOM_OP_H_
