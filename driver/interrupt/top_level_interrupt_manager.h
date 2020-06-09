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

#ifndef DARWINN_DRIVER_INTERRUPT_TOP_LEVEL_INTERRUPT_MANAGER_H_
#define DARWINN_DRIVER_INTERRUPT_TOP_LEVEL_INTERRUPT_MANAGER_H_

#include <memory>

#include "driver/interrupt/interrupt_controller_interface.h"
#include "port/status.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Base class for top level interrupt management.
class TopLevelInterruptManager {
 public:
  explicit TopLevelInterruptManager(
      std::unique_ptr<InterruptControllerInterface> interrupt_controller)
      : interrupt_controller_(std::move(interrupt_controller)) {}
  virtual ~TopLevelInterruptManager() = default;

  // Opens/closes the controller.
  virtual util::Status Open() {
    return util::Status();  // OK
  }
  virtual util::Status Close() {
    return util::Status();  // OK
  }

  // Enable/disables interrupts.
  util::Status EnableInterrupts();
  util::Status DisableInterrupts();

  // Handles interrupt.
  util::Status HandleInterrupt(int id);

  // Returns number of top level interrupts.
  int NumInterrupts() const { return interrupt_controller_->NumInterrupts(); }

 protected:
  // Actually enables/disables interrupts, which are system-specific.
  virtual util::Status DoEnableInterrupts() {
    return util::Status();  // OK
  }
  virtual util::Status DoDisableInterrupts() {
    return util::Status();  // OK
  }

  // Actually handles interrupts, which are system-specific.
  virtual util::Status DoHandleInterrupt(int id) {
    return util::Status();  // OK
  }

 private:
  // Interrupt controller.
  std::unique_ptr<InterruptControllerInterface> interrupt_controller_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_INTERRUPT_TOP_LEVEL_INTERRUPT_MANAGER_H_
