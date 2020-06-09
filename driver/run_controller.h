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

#ifndef DARWINN_DRIVER_RUN_CONTROLLER_H_
#define DARWINN_DRIVER_RUN_CONTROLLER_H_

#include "driver/config/chip_config.h"
#include "driver/config/scalar_core_csr_offsets.h"
#include "driver/config/tile_config_csr_offsets.h"
#include "driver/config/tile_csr_offsets.h"
#include "driver/hardware_structures.h"
#include "driver/registers/registers.h"
#include "port/status.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Controls run states of both scalar core and tiles.
class RunController {
 public:
  RunController(const config::ChipConfig& config, Registers* registers);
  virtual ~RunController() = default;

  // This class is neither copyable nor movable.
  RunController(const RunController&) = delete;
  RunController& operator=(const RunController&) = delete;

  // Performs run control.
  virtual util::Status DoRunControl(RunControl run_state);

 private:
  // CSR offsets.
  const config::ScalarCoreCsrOffsets& scalar_core_csr_offsets_;
  const config::TileConfigCsrOffsets& tile_config_csr_offsets_;
  const config::TileCsrOffsets& tile_csr_offsets_;
  bool has_thread_csr_offsets_;
  const config::TileThreadCsrOffsets* const tile_thread_0_csr_offsets_;
  const config::TileThreadCsrOffsets* const tile_thread_1_csr_offsets_;
  const config::TileThreadCsrOffsets* const tile_thread_2_csr_offsets_;
  const config::TileThreadCsrOffsets* const tile_thread_3_csr_offsets_;
  const config::TileThreadCsrOffsets* const tile_thread_4_csr_offsets_;
  const config::TileThreadCsrOffsets* const tile_thread_5_csr_offsets_;
  const config::TileThreadCsrOffsets* const tile_thread_6_csr_offsets_;
  const config::TileThreadCsrOffsets* const tile_thread_7_csr_offsets_;

  // CSR interface.
  Registers* const registers_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_RUN_CONTROLLER_H_
