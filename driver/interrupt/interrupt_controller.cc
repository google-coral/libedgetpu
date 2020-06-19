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

#include "driver/interrupt/interrupt_controller.h"

#include "driver/config/register_constants.h"
#include "driver/registers/registers.h"
#include "port/logging.h"

namespace platforms {
namespace darwinn {
namespace driver {

InterruptController::InterruptController(
    const config::InterruptCsrOffsets& csr_offsets, Registers* registers,
    int num_interrupts)
    : InterruptControllerInterface(num_interrupts),
      csr_offsets_(csr_offsets),
      registers_(registers) {
  CHECK(registers != nullptr);
}

util::Status InterruptController::EnableInterrupts() {
  if (csr_offsets_.control != kCsrRegisterSpaceInvalidOffset) {
    const uint64 enable_all = (1ULL << NumInterrupts()) - 1;
    return registers_->Write(csr_offsets_.control, enable_all);
  } else {
    return util::OkStatus();
  }
}

util::Status InterruptController::DisableInterrupts() {
  if (csr_offsets_.control != kCsrRegisterSpaceInvalidOffset) {
    constexpr uint64 kDisableAll = 0;
    return registers_->Write(csr_offsets_.control, kDisableAll);
  } else {
    return util::OkStatus();
  }
}

util::Status InterruptController::ClearInterruptStatus(int id) {
  if (csr_offsets_.status != kCsrRegisterSpaceInvalidOffset) {
    // Interrupt status register has W0C policy meaning that writing 0
    // clears the bit, while writing 1 does not have any effect.
    const uint64 clear_bit = ~(1ULL << id);

    uint64 value = (1ULL << NumInterrupts()) - 1;
    value &= clear_bit;
    return registers_->Write(csr_offsets_.status, value);
  } else {
    return util::OkStatus();
  }
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
