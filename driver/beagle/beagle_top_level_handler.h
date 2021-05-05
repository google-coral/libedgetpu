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

#ifndef DARWINN_DRIVER_BEAGLE_BEAGLE_TOP_LEVEL_HANDLER_H_
#define DARWINN_DRIVER_BEAGLE_BEAGLE_TOP_LEVEL_HANDLER_H_

#include "api/driver_options_generated.h"
#include "driver/config/cb_bridge_csr_offsets.h"
#include "driver/config/chip_config.h"
#include "driver/config/hib_user_csr_offsets.h"
#include "driver/config/misc_csr_offsets.h"
#include "driver/config/scalar_core_csr_offsets.h"
#include "driver/config/scu_csr_offsets.h"
#include "driver/config/tile_config_csr_offsets.h"
#include "driver/config/tile_csr_offsets.h"
#include "driver/registers/registers.h"
#include "driver/top_level_handler.h"
#include "port/status.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Handles beagle resets. Only used in remote driver as this will be handled in
// kernel space in kernel driver.
class BeagleTopLevelHandler : public TopLevelHandler {
 public:
  BeagleTopLevelHandler(const config::ChipConfig& config, Registers* registers,
                        bool use_usb, api::PerformanceExpectation performance);
  ~BeagleTopLevelHandler() override = default;

  // Implements ResetHandler interface.
  Status Open() override;
  Status QuitReset() override;
  Status EnableReset() override;
  Status EnableHardwareClockGate() override;
  Status DisableHardwareClockGate() override;

 private:
  // CSR offsets.
  const config::CbBridgeCsrOffsets& cb_bridge_offsets_;
  const config::HibUserCsrOffsets& hib_user_offsets_;
  const config::MiscCsrOffsets& misc_offsets_;
  const config::ScuCsrOffsets& reset_offsets_;
  const config::ScalarCoreCsrOffsets& scalar_core_offsets_;
  const config::TileConfigCsrOffsets& tile_config_offsets_;
  const config::TileCsrOffsets& tile_offsets_;

  // CSR interface.
  Registers* const registers_;

  // Select clock combinations for performance.
  const api::PerformanceExpectation performance_;

  // Whether USB is used for Beagle.
  const bool use_usb_;

  // True if clock gated. Starts at non-clock gated mode.
  bool software_clock_gated_{false};
  bool hardware_clock_gated_{false};
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_BEAGLE_BEAGLE_TOP_LEVEL_HANDLER_H_
