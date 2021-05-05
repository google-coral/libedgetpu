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
  virtual Status Open() {
    return Status();  // OK
  }

  // Closes reset handler.
  virtual Status Close() {
    return Status();  // OK
  }

  // Quits from reset state.
  virtual Status QuitReset() {
    return Status();  // OK
  }

  // Goes into reset state.
  virtual Status EnableReset() {
    return Status();  // OK
  }

  // Enables/disables software clock gating - implementation must be idempotent.
  virtual Status EnableSoftwareClockGate() {
    return Status();  // OK
  }
  virtual Status DisableSoftwareClockGate() {
    return Status();  // OK
  }

  // Enables/disables hardware clock gating - implementation must be idempotent.
  virtual Status EnableHardwareClockGate() {
    return Status();  // OK
  }
  virtual Status DisableHardwareClockGate() {
    return Status();  // OK
  }

  virtual Status LpmCoreToActive() {
    return Status();  // OK
  }

  virtual Status LpmCoreToRailGate() {
    return Status();  // OK
  }
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_TOP_LEVEL_HANDLER_H_
