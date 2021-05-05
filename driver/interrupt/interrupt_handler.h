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

#ifndef DARWINN_DRIVER_INTERRUPT_INTERRUPT_HANDLER_H_
#define DARWINN_DRIVER_INTERRUPT_INTERRUPT_HANDLER_H_

#include <functional>

#include "port/status.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Interrupt identifiers.
enum Interrupt {
  DW_INTERRUPT_INSTR_QUEUE = 0,
  DW_INTERRUPT_INPUT_ACTV_QUEUE = 1,
  DW_INTERRUPT_PARAM_QUEUE = 2,
  DW_INTERRUPT_OUTPUT_ACTV_QUEUE = 3,
  DW_INTERRUPT_SC_HOST_0 = 4,
  DW_INTERRUPT_SC_HOST_1 = 5,
  DW_INTERRUPT_SC_HOST_2 = 6,
  DW_INTERRUPT_SC_HOST_3 = 7,
  DW_INTERRUPT_TOP_LEVEL_0 = 8,
  DW_INTERRUPT_TOP_LEVEL_1 = 9,
  DW_INTERRUPT_TOP_LEVEL_2 = 10,
  DW_INTERRUPT_TOP_LEVEL_3 = 11,
  DW_INTERRUPT_FATAL_ERR = 12,
  DW_INTERRUPT_COUNT = 13,

  // Aliases.
  DW_INTERRUPT_SC_HOST_BASE = DW_INTERRUPT_SC_HOST_0,
  DW_INTERRUPT_TOP_LEVEL_BASE = DW_INTERRUPT_TOP_LEVEL_0,
};

// Interface for handling interrupts.
class InterruptHandler {
 public:
  using Handler = std::function<void()>;

  virtual ~InterruptHandler() = default;

  // Open / Close the interrupt handler.
  virtual Status Open() = 0;
  virtual Status Close(bool in_error) = 0;
  Status Close() { return Close(/*in_error=*/false); }

  // Registers interrupt.
  virtual Status Register(Interrupt interrupt, Handler handler) = 0;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_INTERRUPT_INTERRUPT_HANDLER_H_
