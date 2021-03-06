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

#ifndef DARWINN_DRIVER_INTERRUPT_INTERRUPT_CONTROLLER_H_
#define DARWINN_DRIVER_INTERRUPT_INTERRUPT_CONTROLLER_H_

#include "driver/config/interrupt_csr_offsets.h"
#include "driver/interrupt/interrupt_controller_interface.h"
#include "driver/registers/registers.h"
#include "port/status.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Helper class for enabling/disabling interrupts, and clearing interrupt
// status.
class InterruptController : public InterruptControllerInterface {
 public:
  InterruptController(const config::InterruptCsrOffsets& csr_offsets,
                      Registers* registers, int num_interrupts = 1);

  // This class is neither copyable nor movable.
  InterruptController(const InterruptController&) = delete;
  InterruptController& operator=(const InterruptController&) = delete;

  ~InterruptController() = default;

  // Enable/disables interrupts.
  Status EnableInterrupts() override;
  Status DisableInterrupts() override;

  // Clears interrupt status register to notify that host has received the
  // interrupt.
  Status ClearInterruptStatus(int id) override;

 private:
  // CSR offsets.
  const config::InterruptCsrOffsets& csr_offsets_;

  // CSR interface.
  Registers* const registers_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_INTERRUPT_INTERRUPT_CONTROLLER_H_
