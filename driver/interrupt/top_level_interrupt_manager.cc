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

#include "driver/interrupt/top_level_interrupt_manager.h"

#include "port/status.h"
#include "port/status_macros.h"

namespace platforms {
namespace darwinn {
namespace driver {

util::Status TopLevelInterruptManager::EnableInterrupts() {
  RETURN_IF_ERROR(interrupt_controller_->EnableInterrupts());
  return DoEnableInterrupts();
}

util::Status TopLevelInterruptManager::DisableInterrupts() {
  RETURN_IF_ERROR(interrupt_controller_->DisableInterrupts());
  return DoDisableInterrupts();
}

util::Status TopLevelInterruptManager::HandleInterrupt(int id) {
  RETURN_IF_ERROR(DoHandleInterrupt(id));
  return interrupt_controller_->ClearInterruptStatus(id);
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
