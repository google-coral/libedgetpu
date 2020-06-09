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

#include "driver/interrupt/grouped_interrupt_controller.h"

#include "driver/interrupt/interrupt_controller_interface.h"
#include "port/errors.h"
#include "port/logging.h"
#include "port/status_macros.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

GroupedInterruptController::GroupedInterruptController(
    std::vector<std::unique_ptr<InterruptControllerInterface>>*
        interrupt_controllers)
    : InterruptControllerInterface(interrupt_controllers->size()),
      interrupt_controllers_([interrupt_controllers]() {
        CHECK(interrupt_controllers != nullptr);
        return std::move(*interrupt_controllers);
      }()) {}

util::Status GroupedInterruptController::EnableInterrupts() {
  for (auto& interrupt_controller : interrupt_controllers_) {
    RETURN_IF_ERROR(interrupt_controller->EnableInterrupts());
  }
  return util::Status();  // OK
}

util::Status GroupedInterruptController::DisableInterrupts() {
  for (auto& interrupt_controller : interrupt_controllers_) {
    RETURN_IF_ERROR(interrupt_controller->DisableInterrupts());
  }
  return util::Status();  // OK
}

util::Status GroupedInterruptController::ClearInterruptStatus(int id) {
  if (id < interrupt_controllers_.size()) {
    return interrupt_controllers_[id]->ClearInterruptStatus();
  }
  return util::FailedPreconditionError(
      StringPrintf("Unknown interrupt id: %d", id));
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
