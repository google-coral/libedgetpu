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

#ifndef DARWINN_DRIVER_SCALAR_CORE_CONTROLLER_H_
#define DARWINN_DRIVER_SCALAR_CORE_CONTROLLER_H_

#include <mutex>  // NOLINT
#include <vector>

#include "driver/config/chip_config.h"
#include "driver/config/hib_user_csr_offsets.h"
#include "driver/interrupt/interrupt_controller.h"
#include "driver/registers/registers.h"
#include "port/statusor.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Controls scalar core.
class ScalarCoreController {
 public:
  ScalarCoreController(const config::ChipConfig& config, Registers* registers);

  // This class is neither copyable nor movable.
  ScalarCoreController(const ScalarCoreController&) = delete;
  ScalarCoreController& operator=(const ScalarCoreController&) = delete;

  virtual ~ScalarCoreController() = default;

  // Opens/closes the controller.
  virtual util::Status Open() LOCKS_EXCLUDED(mutex_);
  virtual util::Status Close() LOCKS_EXCLUDED(mutex_);

  // Enable/disables interrupts.
  util::Status EnableInterrupts() LOCKS_EXCLUDED(mutex_);
  util::Status DisableInterrupts() LOCKS_EXCLUDED(mutex_);

  // Clears interrupt status register to notify that host has received the
  // interrupt.
  util::Status ClearInterruptStatus(int id) LOCKS_EXCLUDED(mutex_);

  // Reads and returns scalar core interrupt count register for given |id|. Read
  // is destructive in the sense that the second read to the same |id| will
  // return 0 assuming that there was no change in the CSR from the hardware
  // side.
  virtual util::StatusOr<uint64> CheckInterruptCounts(int id)
      LOCKS_EXCLUDED(mutex_);

 private:
  // Returns an error if not |open|.
  util::Status ValidateOpenState(bool open) const SHARED_LOCKS_REQUIRED(mutex_);

  // CSR offsets.
  const config::HibUserCsrOffsets& hib_user_csr_offsets_;

  // CSR interface.
  Registers* const registers_;

  // Interrupt controller.
  InterruptController interrupt_controller_;

  // Counted interrupts from scalar core.
  std::vector<uint64> interrupt_counts_;

  // Guard/track the open status of ScalarCoreController.
  std::mutex mutex_;
  bool open_ GUARDED_BY(mutex_) {false};
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_SCALAR_CORE_CONTROLLER_H_
