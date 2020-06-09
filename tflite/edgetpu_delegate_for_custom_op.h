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

#ifndef DARWINN_TFLITE_EDGETPU_DELEGATE_FOR_CUSTOM_OP_H_
#define DARWINN_TFLITE_EDGETPU_DELEGATE_FOR_CUSTOM_OP_H_

#include <memory>

#include "tflite/public/edgetpu.h"

namespace platforms {
namespace darwinn {
namespace tflite {

// Creates delegate instance which enables `tflite::Interpreter` to support
// edge TPU custom op. Returns `nullptr` if context contains `nullptr`.
TfLiteDelegate* CreateEdgeTpuDelegateForCustomOp(
    std::shared_ptr<edgetpu::EdgeTpuContext> context);

// Deletes created delegate instance, `delegate` may be `nullptr`.
void FreeEdgeTpuDelegateForCustomOp(TfLiteDelegate* delegate);

}  // namespace tflite
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_TFLITE_EDGETPU_DELEGATE_FOR_CUSTOM_OP_H_
