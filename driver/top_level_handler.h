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

#ifndef DARWINN_DRIVER_TOP_LEVEL_HANDLER_H_
#define DARWINN_DRIVER_TOP_LEVEL_HANDLER_H_

#include "port/status.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Interface for handling resets.
class TopLevelHandler {
 public:
  virtual ~TopLevelHandler() = default;

  // Opens reset handler.
  virtual util::Status Open() {
    return util::Status();  // OK
  }

  // Closes reset handler.
  virtual util::Status Close() {
    return util::Status();  // OK
  }

  // Quits from reset state.
  virtual util::Status QuitReset() {
    return util::Status();  // OK
  }

  // Goes into reset state.
  virtual util::Status EnableReset() {
    return util::Status();  // OK
  }

  // Enables/disables software clock gating - implementation must be idempotent.
  virtual util::Status EnableSoftwareClockGate() {
    return util::Status();  // OK
  }
  virtual util::Status DisableSoftwareClockGate() {
    return util::Status();  // OK
  }

  // Enables/disables hardware clock gating - implementation must be idempotent.
  virtual util::Status EnableHardwareClockGate() {
    return util::Status();  // OK
  }
  virtual util::Status DisableHardwareClockGate() {
    return util::Status();  // OK
  }
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_TOP_LEVEL_HANDLER_H_
