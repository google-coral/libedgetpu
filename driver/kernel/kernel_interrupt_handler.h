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

#ifndef DARWINN_DRIVER_KERNEL_KERNEL_INTERRUPT_HANDLER_H_
#define DARWINN_DRIVER_KERNEL_KERNEL_INTERRUPT_HANDLER_H_

#include <string>

#include "driver/interrupt/interrupt_handler.h"
#include "driver/kernel/kernel_event_handler.h"
#include "port/status.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Kernel implementation of the interrupt handler interface.
class KernelInterruptHandler : public InterruptHandler {
 public:
  // Default close to avoid name hiding.
  using InterruptHandler::Close;

  explicit KernelInterruptHandler(
      std::unique_ptr<KernelEventHandler> event_handler);
  ~KernelInterruptHandler() override = default;

  util::Status Open() override;
  util::Status Close(bool in_error) override;
  util::Status Register(Interrupt interrupt, Handler handler) override;

 private:
  // Backing event handler.
  std::unique_ptr<KernelEventHandler> event_handler_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_KERNEL_KERNEL_INTERRUPT_HANDLER_H_
