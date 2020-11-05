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

#include "driver/kernel/kernel_interrupt_handler.h"

#include <string>
#include <utility>

#include "driver/interrupt/interrupt_handler.h"
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

KernelInterruptHandler::KernelInterruptHandler(
    std::unique_ptr<KernelEventHandler> event_handler)
    : event_handler_(std::move(event_handler)) {}

util::Status KernelInterruptHandler::Open() { return event_handler_->Open(); }

util::Status KernelInterruptHandler::Close(bool in_error) {
  return event_handler_->Close();
}

util::Status KernelInterruptHandler::Register(Interrupt interrupt,
                                              Handler handler) {
  return event_handler_->RegisterEvent(interrupt, std::move(handler));
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
