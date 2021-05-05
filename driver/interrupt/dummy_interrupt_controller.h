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

#ifndef DARWINN_DRIVER_INTERRUPT_DUMMY_INTERRUPT_CONTROLLER_H_
#define DARWINN_DRIVER_INTERRUPT_DUMMY_INTERRUPT_CONTROLLER_H_

#include "driver/interrupt/interrupt_controller_interface.h"
#include "port/status.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Dummy class that does nothing upon interrupt related requests.
class DummyInterruptController : public InterruptControllerInterface {
 public:
  explicit DummyInterruptController(int num_interrupts)
      : InterruptControllerInterface(num_interrupts) {}

  // This class is neither copyable nor movable.
  DummyInterruptController(const DummyInterruptController&) = delete;
  DummyInterruptController& operator=(const DummyInterruptController&) = delete;

  ~DummyInterruptController() = default;

  Status EnableInterrupts() override {
    return Status();  // OK
  }

  Status DisableInterrupts() override {
    return Status();  // OK
  }

  Status ClearInterruptStatus(int id) override {
    return Status();  // OK
  }
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_INTERRUPT_DUMMY_INTERRUPT_CONTROLLER_H_
