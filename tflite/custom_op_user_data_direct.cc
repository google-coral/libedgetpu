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

#include "tflite/custom_op_user_data_direct.h"

#include "driver/package_registry.h"
#include "port/logging.h"
#include "port/stringprintf.h"
#include "tflite/custom_op_data.h"

namespace platforms {
namespace darwinn {
namespace tflite {

CustomOpUserDataDirect::CustomOpUserDataDirect(const uint8_t* buffer,
                                               size_t length)
    : raw_model_data_(DeserializeCustomOpData(buffer, length)) {}

CustomOpUserDataDirect::~CustomOpUserDataDirect() {
  (void)UnregisterExecutables();
}

util::Status CustomOpUserDataDirect::SetDriver(api::Driver* driver) {
  if (!driver) {
    return util::InvalidArgumentError("Cannot be assigned to nullptr.");
  }

  if (driver_) {
    if (driver == driver_) {
      // It is okay to set to the same driver instance.
      // Prepare could be called multiple times to the same set of operators.
      return util::Status();  // OK.
    } else {
      return util::FailedPreconditionError(
          "Custom op already assigned to a different TPU.");
    }
  }
  driver_ = driver;

  if (!raw_model_data_) {
    return util::FailedPreconditionError("Missing raw model data.");
  }

  // Register the executable.
  ASSIGN_OR_RETURN(executable_, driver_->RegisterExecutableSerialized(
                                    raw_model_data_->executable.data,
                                    raw_model_data_->executable.length));

  // Gets the executable layer info from the executable binary.
  // TODO: Merge the ExecutableLayersInfo and api::PackageReference
  ASSIGN_OR_RETURN(
      auto executable_layers_info_unique_ptr,
      driver::PackageRegistry::GetMainExecutableLayersInfoFromBinary(
          raw_model_data_->executable.data,
          raw_model_data_->executable.length));

  // The executable layer info will stay alive till it's deleted in unregister.
  executable_layers_info_ = executable_layers_info_unique_ptr.release();

  raw_model_data_.reset();

  return util::Status();  // OK.
}

util::Status CustomOpUserDataDirect::UnregisterExecutables() {
  if (!driver_) {
    return util::Status();  // OK.
  }

  if (executable_) {
    (void)driver_->UnregisterExecutable(executable_);
    executable_ = nullptr;
  }

  if (executable_layers_info_) {
    delete executable_layers_info_;
    executable_layers_info_ = nullptr;
  }

  return util::Status();  // OK.
}

}  // namespace tflite
}  // namespace darwinn
}  // namespace platforms
