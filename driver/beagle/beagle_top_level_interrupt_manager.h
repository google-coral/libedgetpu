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

#ifndef DARWINN_DRIVER_BEAGLE_BEAGLE_TOP_LEVEL_INTERRUPT_MANAGER_H_
#define DARWINN_DRIVER_BEAGLE_BEAGLE_TOP_LEVEL_INTERRUPT_MANAGER_H_

#include <memory>

#include "driver/config/apex_csr_offsets.h"
#include "driver/config/chip_config.h"
#include "driver/config/scu_csr_offsets.h"
#include "driver/interrupt/interrupt_controller_interface.h"
#include "driver/interrupt/top_level_interrupt_manager.h"
#include "driver/registers/registers.h"
#include "port/status.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Beagle-specific top level interrupt management.
class BeagleTopLevelInterruptManager : public TopLevelInterruptManager {
 public:
  BeagleTopLevelInterruptManager(
      std::unique_ptr<InterruptControllerInterface> interrupt_controller,
      const config::ChipConfig& config, Registers* registers);
  ~BeagleTopLevelInterruptManager() override = default;

 protected:
  // Implements interfaces.
  util::Status DoEnableInterrupts() override;
  util::Status DoDisableInterrupts() override;
  util::Status DoHandleInterrupt(int id) override;

 private:
  // Implements extra CSR managements to enable top level interrupts.
  util::Status EnableThermalWarningInterrupt();
  util::Status EnableMbistInterrupt();
  util::Status EnablePcieErrorInterrupt();
  util::Status EnableThermalShutdownInterrupt();

  // Implements extra CSR managements to disable top level interrupts.
  util::Status DisableThermalWarningInterrupt();
  util::Status DisableMbistInterrupt();
  util::Status DisablePcieErrorInterrupt();
  util::Status DisableThermalShutdownInterrupt();

  // Implements top level interrupt handling.
  util::Status HandleThermalWarningInterrupt();
  util::Status HandleMbistInterrupt();
  util::Status HandlePcieErrorInterrupt();
  util::Status HandleThermalShutdownInterrupt();

  // Apex CSR offsets.
  const config::ApexCsrOffsets& apex_csr_offsets_;

  // SCU CSR offsets.
  const config::ScuCsrOffsets scu_csr_offsets_;

  // CSR interface.
  Registers* const registers_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_BEAGLE_BEAGLE_TOP_LEVEL_INTERRUPT_MANAGER_H_
