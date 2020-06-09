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

#ifndef DARWINN_DRIVER_INTERRUPT_INTERRUPT_CONTROLLER_INTERFACE_H_
#define DARWINN_DRIVER_INTERRUPT_INTERRUPT_CONTROLLER_INTERFACE_H_

#include "port/status.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Interface for enabling/disabling interrupts, and clearing interrupt status.
class InterruptControllerInterface {
 public:
  explicit InterruptControllerInterface(int num_interrupts)
      : num_interrupts_(num_interrupts) {}

  // This class is neither copyable nor movable.
  InterruptControllerInterface(const InterruptControllerInterface&) = delete;
  InterruptControllerInterface& operator=(const InterruptControllerInterface&) =
      delete;

  virtual ~InterruptControllerInterface() = default;

  // Enable/disables interrupts.
  virtual util::Status EnableInterrupts() = 0;
  virtual util::Status DisableInterrupts() = 0;

  // Clears interrupt status register to notify that host has received the
  // interrupt.
  virtual util::Status ClearInterruptStatus(int id) = 0;
  util::Status ClearInterruptStatus() { return ClearInterruptStatus(/*id=*/0); }

  // Returns number of interrupts controlled by this interface.
  int NumInterrupts() const { return num_interrupts_; }

 private:
  // Number of interrupts enabled/disabled/cleared by this interface.
  const int num_interrupts_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_INTERRUPT_INTERRUPT_CONTROLLER_INTERFACE_H_
