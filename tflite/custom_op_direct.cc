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

#include <algorithm>
#include <utility>

#include "api/driver.h"
#include "port/errors.h"
#include "port/ptr_util.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/statusor.h"
#include "port/stringprintf.h"
#include "tflite/custom_op.h"
#include "tflite/custom_op_data.h"
#include "tflite/custom_op_user_data_direct.h"
#include "tflite/edgetpu_context_direct.h"
#include "tflite/edgetpu_manager_direct.h"
#include "tflite/public/edgetpu.h"
#include "tensorflow/lite/context.h"

namespace platforms {
namespace darwinn {
namespace tflite {

namespace {

using edgetpu::EdgeTpuManager;

// Various initializations steps for a DarwiNN custom op.
void* CustomOpInit(TfLiteContext* context, const char* buffer, size_t length) {
  // Create new operator-specific user data.
  // Note this data is different from interpreter-specific data recorded in
  // context->GetExternalContext, which is probably not set yet when
  // this function is called.
  return new CustomOpUserDataDirect(reinterpret_cast<const uint8_t*>(buffer),
                                    length);
}

// Returns either the associated TPU context.
EdgeTpuContextDirect* GetTpuContext(TfLiteContext* context) {
  // Down-cast from TfLiteExternalContext* to EdgeTpuContextDirect*
  return static_cast<EdgeTpuContextDirect*>(
      context->GetExternalContext(context, kTfLiteEdgeTpuContext));
}

// This function is called only when Interpreter believes it's needed, when
// within the call to Intrepreter::AllocateTensor.
TfLiteStatus CustomOpPrepareDirect(TfLiteContext* context, TfLiteNode* node) {
  CustomOpUserDataDirect* user_data =
      reinterpret_cast<CustomOpUserDataDirect*>(node->user_data);

  if (!user_data) {
    context->ReportError(context, "Null custom op data.");
    return kTfLiteError;
  }

  auto* interpreter_context = GetTpuContext(context);
  if (!interpreter_context) {
    context->ReportError(context, "Failed to retrieve TPU context.");
    return kTfLiteError;
  }

  // Binds this custom op instance with a particular driver instance.
  // It actually registers the model with the driver specified in interpreter
  // context.
  auto result = user_data->SetDriver(
      interpreter_context->GetDriverWrapper()->GetDriver());
  if (!result.ok()) {
    context->ReportError(context, "Failed to prepare for TPU. %s",
                         result.ToString().c_str());
    return kTfLiteError;
  }

  return CustomOpPrepare(context, node);
}

// De-allocates the per-node-and-Interpreter custom data.
void CustomOpFreeDirect(TfLiteContext* context, void* buffer) {
  // TODO: Remove the whole function after the new cache mechanism is
  // ready. Use CustomOpFree instead.
  CustomOpUserDataDirect* user_data =
      reinterpret_cast<CustomOpUserDataDirect*>(buffer);

  if (!user_data) {
    context->ReportError(context, "Null custom op data.");
    return;
  }

  // Deleting user_data un-registers the model from the driver, if it has ever
  // been registered.
  delete user_data;
}

TfLiteStatus CustomOpInvoke(TfLiteContext* context, TfLiteNode* node) {
  CustomOpUserDataDirect* user_data =
      reinterpret_cast<CustomOpUserDataDirect*>(node->user_data);

  if (!user_data) {
    context->ReportError(context, "Null custom op data.");
    return kTfLiteError;
  }

  auto* interpreter_context = GetTpuContext(context);
  if (!interpreter_context) {
    context->ReportError(context, "Failed to retrieve TPU context.");
    return kTfLiteError;
  }

  auto result =
      interpreter_context->GetDriverWrapper()->InvokeExecutable(context, node);
  if (!result.ok()) {
    context->ReportError(context, StringPrintf("Failed to execute request. %s",
                                               result.error_message().c_str())
                                      .c_str());
    return kTfLiteError;
  }

  return kTfLiteOk;
}

}  // namespace
}  // namespace tflite
}  // namespace darwinn
}  // namespace platforms

namespace edgetpu {

TfLiteRegistration* RegisterCustomOp() {
  static TfLiteRegistration registration = {
      platforms::darwinn::tflite::CustomOpInit,
      platforms::darwinn::tflite::CustomOpFreeDirect,
      platforms::darwinn::tflite::CustomOpPrepareDirect,
      platforms::darwinn::tflite::CustomOpInvoke,
  };
  return &registration;
}

EdgeTpuManager* EdgeTpuManager::GetSingleton() {
  return platforms::darwinn::tflite::EdgeTpuManagerDirect::GetSingleton();
}

std::ostream& operator<<(std::ostream& out, edgetpu::DeviceType device_type) {
  out << platforms::darwinn::tflite::EdgeTpuDriverWrapper::GetDeviceTypeName(
      device_type);
  return out;
}

}  // namespace edgetpu
