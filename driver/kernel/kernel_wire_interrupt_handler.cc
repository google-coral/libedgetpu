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

#include "driver/kernel/kernel_wire_interrupt_handler.h"

#include <string>

#include "driver/config/wire_csr_offsets.h"
#include "driver/interrupt/wire_interrupt_handler.h"
#include "driver/registers/registers.h"
#include "port/cleanup.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/ptr_util.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

KernelWireInterruptHandler::KernelWireInterruptHandler(
    Registers* registers, const config::WireCsrOffsets& wire_csr_offsets,
    std::unique_ptr<KernelEventHandler> event_handler, int num_wires)
    : wire_handler_(registers, wire_csr_offsets, num_wires),
      event_handler_(std::move(event_handler)),
      num_wires_(num_wires) {}

Status KernelWireInterruptHandler::Open() {
  RETURN_IF_ERROR(wire_handler_.Open());
  auto wire_handler_closer = MakeCleanup(
      [this]() NO_THREAD_SAFETY_ANALYSIS { CHECK_OK(wire_handler_.Close()); });

  RETURN_IF_ERROR(event_handler_->Open());
  auto event_handler_closer = MakeCleanup([this]() NO_THREAD_SAFETY_ANALYSIS {
    CHECK_OK(event_handler_->Close());
  });

  for (int wire = 0; wire < num_wires_; ++wire) {
    RETURN_IF_ERROR(event_handler_->RegisterEvent(wire, [this, wire]() {
      wire_handler_.InvokeAllPendingInterrupts(wire);
    }));
  }

  // All good. Release cleanup functions.
  wire_handler_closer.release();
  event_handler_closer.release();

  return Status();  // OK
}

Status KernelWireInterruptHandler::Close(bool in_error) {
  Status status;
  status.Update(event_handler_->Close());
  status.Update(wire_handler_.Close());
  return status;
}

Status KernelWireInterruptHandler::Register(Interrupt interrupt,
                                            Handler handler) {
  return wire_handler_.Register(interrupt, std::move(handler));
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
