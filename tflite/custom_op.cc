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

#include "tflite/custom_op.h"

#include <algorithm>

#include "absl/strings/match.h"
#include "api/layer_information.h"
#include "driver/package_registry.h"
#include "port/errors.h"
#include "port/ptr_util.h"
#include "port/status_macros.h"
#include "port/stringprintf.h"
#include "tflite/custom_op_data.h"
#include "tflite/public/edgetpu.h"
#include "tensorflow/lite/context.h"

#define RETURN_IF_NOT_EQ(a, b)                                                 \
  do {                                                                         \
    if ((a) != (b)) {                                                          \
      return InternalError(StringPrintf("%s:%d %s != %s (%d != %d)", __FILE__, \
                                        __LINE__, #a, #b, (int)(a),            \
                                        (int)(b)));                            \
    }                                                                          \
  } while (0)

namespace platforms {
namespace darwinn {
namespace tflite {

namespace {

bool IsFloat32ClassifierLayer(const api::OutputLayerInformation* output_layer) {
  return (output_layer->y_dim() == 1 && output_layer->x_dim() == 1 &&
          output_layer->data_type() == platforms::darwinn::DataType_SINGLE);
}

bool IsUint16ClassifierLayer(const api::OutputLayerInformation* output_layer) {
  return (output_layer->y_dim() == 1 && output_layer->x_dim() == 1 &&
          output_layer->data_type() ==
              platforms::darwinn::DataType_FIXED_POINT16);
}

// Returns the number of bytes occupied by a value of the given data type.
// Note that only a subset of data types are currently supported.
StatusOr<int> SizeOfDataType(TfLiteType data_type) {
  switch (data_type) {
    case kTfLiteUInt8:
    case kTfLiteInt8:
      return sizeof(uint8_t);

    case kTfLiteInt16:
      return sizeof(uint16_t);

    case kTfLiteInt32:
      return sizeof(uint32_t);

    case kTfLiteFloat16:
      return 2;

    case kTfLiteFloat32:
      return sizeof(float);

    default:
      return InternalError(StringPrintf(
          "Unsupported data type in custom op handler: %d", data_type));
  }
}

// Checks that the DarwiNN and TFLite types are compatible. For input layers,
// they must match exactly. For output layers, they are compatible if
// ReFormatOutputs is capable of converting from the DarwiNN type to the
// corresponding TFLite type.
// Note that only a subset of data types are currently supported.
Status ValidateDataType(
    TfLiteType tf_lite_type, darwinn::DataType darwinn_type,
    const api::OutputLayerInformation* optional_output_layer) {
  switch (darwinn_type) {
    case DataType_FIXED_POINT8:
      RETURN_IF_NOT_EQ(tf_lite_type, kTfLiteUInt8);
      break;
    case DataType_SIGNED_FIXED_POINT8:
      RETURN_IF_NOT_EQ(tf_lite_type, kTfLiteInt8);
      break;

    case DataType_FIXED_POINT16:
      if (optional_output_layer != nullptr && tf_lite_type == kTfLiteUInt8 &&
          IsUint16ClassifierLayer(optional_output_layer)) {
        return OkStatus();
      }
      RETURN_IF_NOT_EQ(tf_lite_type, kTfLiteInt16);
      break;

    case DataType_SIGNED_FIXED_POINT16:
      RETURN_IF_NOT_EQ(tf_lite_type, kTfLiteInt16);
      break;

    case DataType_SIGNED_FIXED_POINT32:
      RETURN_IF_NOT_EQ(tf_lite_type, kTfLiteInt32);
      break;

    case DataType_SINGLE:
      if (optional_output_layer != nullptr && tf_lite_type == kTfLiteUInt8 &&
          IsFloat32ClassifierLayer(optional_output_layer)) {
        return OkStatus();
      }
      RETURN_IF_NOT_EQ(tf_lite_type, kTfLiteFloat32);
      break;

    case DataType_HALF:
      RETURN_IF_NOT_EQ(tf_lite_type, kTfLiteFloat16);
      break;

    default:
      return InternalError(
          StringPrintf("Unsupported layer data type in custom op handler: %d",
                       darwinn_type));
  }

  return OkStatus();
}

// Validates input and output count, type and sizes against DarwiNN executable.
// Also resizes output tensors to the correct batch size.
Status ValidateInputsAndOutputs(
    TfLiteContext* context, TfLiteNode* node,
    const driver::ExecutableLayersInfo* executable_layers_info) {
  int batches = 0;

  // Validate inputs.
  CustomOpUserData* user_data =
      reinterpret_cast<CustomOpUserData*>(node->user_data);
  RETURN_IF_NOT_EQ(executable_layers_info->NumInputLayers(),
                   user_data->GetInputs(node)->size);

  for (int i = 0; i < executable_layers_info->NumInputLayers(); ++i) {
    const TfLiteTensor* input = GetInput(context, node, i);
    ASSIGN_OR_RETURN(const int size_of_data_type, SizeOfDataType(input->type));
    const int single_input_size =
        executable_layers_info->InputLayerSize(i) * size_of_data_type;

    // Data types must match.
    RETURN_IF_ERROR(ValidateDataType(
        input->type, executable_layers_info->InputLayer(i)->data_type(),
        nullptr));

    // Check for a batch dimension. The batch dimension is always assumed to be
    // the first dimension.
    int input_batches = 1;
    if (input->dims->size >= 1 &&
        input->dims->data[0] * single_input_size == input->bytes) {
      input_batches = input->dims->data[0];
    }

    // All inputs must have the same number of batches.
    if (batches == 0) {
      batches = input_batches;
    } else {
      RETURN_IF_NOT_EQ(batches, input_batches);
    }

    RETURN_IF_NOT_EQ(batches * single_input_size, input->bytes);
  }

  // |batches| == 0 means there were no inputs. Treat that as 1 batch, because
  // we always want to run the model at least once.
  if (batches == 0) {
    batches = 1;
  }

  // Validate outputs.
  for (int i = 0; i < executable_layers_info->NumOutputLayers(); ++i) {
    const std::string kVariableSuffix = "_variable_output";
    if (absl::EndsWith(executable_layers_info->OutputLayer(i)->name(),
                       kVariableSuffix)) {
      continue;
    }
    if (i >= node->outputs->size) {
      return InvalidArgumentError(
          "Execuable has more outputs than TfLite node.");
    }
    TfLiteTensor* output = GetOutput(context, node, i);
    ASSIGN_OR_RETURN(const int size_of_data_type, SizeOfDataType(output->type));
    const int single_output_size =
        executable_layers_info->OutputLayerSize(i) * size_of_data_type;

    // Data types must match.
    RETURN_IF_ERROR(ValidateDataType(
        output->type, executable_layers_info->OutputLayer(i)->data_type(),
        executable_layers_info->OutputLayer(i)));

    // If there's a batch dimension on the output, set it to the correct size.
    // Note that this has to be done even for batches == 1, in case the tensor
    // has to be resized down.
    if (output->dims->size >= 1 &&
        output->dims->data[0] * single_output_size == output->bytes) {
      if (batches != output->dims->data[0]) {
        TfLiteIntArray* output_size = TfLiteIntArrayCreate(output->dims->size);
        output_size->data[0] = batches;
        for (int dim = 1; dim < output->dims->size; ++dim) {
          output_size->data[dim] = output->dims->data[dim];
        }
        context->ResizeTensor(context, output, output_size);
      }
    } else {
      RETURN_IF_NOT_EQ(batches, 1);
    }

    RETURN_IF_NOT_EQ(batches * single_output_size, output->bytes);
  }

  user_data->SetBatches(batches);
  return OkStatus();
}

}  // namespace

CustomOpUserData::~CustomOpUserData() {
  if (inputs_) {
    TfLiteIntArrayFree(inputs_);
    inputs_ = nullptr;
  }
}

const std::string& CustomOpUserData::GetSessionName() const {
  return session_name_;
}

bool CustomOpUserData::GetShouldPopulateCache() const {
  return should_populate_cache_;
}

void CustomOpUserData::SetShouldPopulateCache(bool should_populate_cache) {
  should_populate_cache_ = should_populate_cache;
}

TfLiteIntArray* CustomOpUserData::GetInputs() const { return inputs_; }

TfLiteIntArray* CustomOpUserData::GetInputs(TfLiteNode* node) const {
  if (inputs_) {
    return inputs_;
  } else {
    CHECK_NE(node, nullptr);
    return node->inputs;
  }
}

TfLiteTensor* GetInput(TfLiteContext* context, TfLiteNode* node, int index) {
  CustomOpUserData* user_data =
      reinterpret_cast<CustomOpUserData*>(node->user_data);

  return &context->tensors[user_data->GetInputs(node)->data[index]];
}

TfLiteTensor* GetOutput(TfLiteContext* context, TfLiteNode* node, int index) {
  return &context->tensors[node->outputs->data[index]];
}

TfLiteStatus CustomOpPrepare(TfLiteContext* context, TfLiteNode* node) {
  if (!node->user_data) {
    context->ReportError(context, "Failed to prepare a custom op.");
    return kTfLiteError;
  }
  const auto* user_data = reinterpret_cast<CustomOpUserData*>(node->user_data);
  const auto* executable_layers_info = user_data->GetExecutableLayersInfo();
  CHECK_NE(executable_layers_info, nullptr);

  Status status =
      ValidateInputsAndOutputs(context, node, executable_layers_info);
  if (!status.ok()) {
    context->ReportError(context, status.ToString().c_str());
    return kTfLiteError;
  }
  return kTfLiteOk;
}

void CustomOpFree(TfLiteContext* context, void* buffer) {
  // TODO: Unregister executables, once that is implemented.
  delete reinterpret_cast<CustomOpUserData*>(buffer);
}

Status ReFormatOutputs(TfLiteTensor* output, int output_tensor_offset,
                       int output_tensor_size,
                       const api::OutputLayerInformation* output_layer,
                       const unsigned char* output_data) {
  // Although we have 8-bit classifier now, the following is kept for backwards
  // compatibility with executables which were generated the old way.
  if (output->type == kTfLiteUInt8 && IsFloat32ClassifierLayer(output_layer)) {
    const float* tpu_output = reinterpret_cast<const float*>(output_data);
    for (int j = 0; j < output_tensor_size; ++j) {
      int quantized_int = static_cast<int>(
          (tpu_output[j] / output->params.scale + output->params.zero_point));
      output->data.uint8[j + output_tensor_offset] =
          std::min(std::max(quantized_int, 0), 255);
    }
  } else if (output->type == kTfLiteUInt8 &&
             IsUint16ClassifierLayer(output_layer)) {
    const int16_t* tpu_output = reinterpret_cast<const int16_t*>(output_data);
    for (int j = 0; j < output_tensor_size; ++j) {
      output->data.uint8[j + output_tensor_offset] =
          std::min(std::max(static_cast<int>(tpu_output[j]), 0), 255);
    }
  } else {
    memcpy(output->data.uint8 + output_tensor_offset, output_data,
           output_tensor_size);
  }

  return OkStatus();
}

}  // namespace tflite
}  // namespace darwinn
}  // namespace platforms
