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

#ifndef DARWINN_TFLITE_CUSTOM_OP_WRAPPED_BUFFER_H_
#define DARWINN_TFLITE_CUSTOM_OP_WRAPPED_BUFFER_H_

#include "api/chip.h"

namespace platforms {
namespace darwinn {
namespace tflite {

// Convenient structure for holding the executable buffer inside a TfLite
// custom op.
struct CustomOpWrappedBuffer {
  const char* data;
  size_t length;

  // Optional field. Use to specify the target chip of the wrapped executable
  // when there are multiple exectuables in `CustomOpData` or `CustomOp2`.
  api::Chip chip = api::Chip::kUnknown;
};

}  // namespace tflite
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_TFLITE_CUSTOM_OP_WRAPPED_BUFFER_H_
