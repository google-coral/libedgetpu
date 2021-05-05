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

#ifndef DARWINN_DRIVER_KERNEL_KERNEL_WIRE_INTERRUPT_HANDLER_H_
#define DARWINN_DRIVER_KERNEL_KERNEL_WIRE_INTERRUPT_HANDLER_H_

#include <mutex>  // NOLINT
#include <string>

#include "driver/config/wire_csr_offsets.h"
#include "driver/interrupt/interrupt_handler.h"
#include "driver/interrupt/wire_interrupt_handler.h"
#include "driver/kernel/kernel_event_handler.h"
#include "driver/registers/registers.h"
#include "port/status.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Wire Interrupt handler implementation that reads and processes the pending
// bit array on a single wire interrupt in userspace.
class KernelWireInterruptHandler : public InterruptHandler {
 public:
  // Default close to avoid name hiding.
  using InterruptHandler::Close;

  KernelWireInterruptHandler(Registers* registers,
                             const config::WireCsrOffsets& wire_csr_offsets,
                             std::unique_ptr<KernelEventHandler> event_handler,
                             int num_wires);
  ~KernelWireInterruptHandler() override = default;

  // This class is neither copyable nor movable.
  KernelWireInterruptHandler(const KernelWireInterruptHandler&) = delete;
  KernelWireInterruptHandler& operator=(const KernelWireInterruptHandler&) =
      delete;

  Status Open() override;
  Status Close(bool in_error) override;
  Status Register(Interrupt interrupt, Handler handler) override;

 private:
  // Backing wire interrupt handler.
  WireInterruptHandler wire_handler_;

  // KernelEventHandler
  std::unique_ptr<KernelEventHandler> event_handler_;

  // Number of wires.
  const int num_wires_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_KERNEL_KERNEL_WIRE_INTERRUPT_HANDLER_H_
