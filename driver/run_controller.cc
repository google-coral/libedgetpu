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

#include "driver/run_controller.h"

#include "driver/config/common_csr_helper.h"
#include "driver/registers/registers.h"
#include "port/errors.h"
#include "port/logging.h"
#include "port/status_macros.h"

namespace platforms {
namespace darwinn {
namespace driver {

RunController::RunController(const config::ChipConfig& config,
                             Registers* registers)
    : scalar_core_csr_offsets_(config.GetScalarCoreCsrOffsets()),
      tile_config_csr_offsets_(config.GetTileConfigCsrOffsets()),
      tile_csr_offsets_(config.GetTileCsrOffsets()),
      has_thread_csr_offsets_(config.HasThreadCsrOffsets()),
      tile_thread_0_csr_offsets_(config.HasThreadCsrOffsets() ?
        &config.GetTileThread0CsrOffsets() : nullptr),
      tile_thread_1_csr_offsets_(config.HasThreadCsrOffsets() ?
        &config.GetTileThread1CsrOffsets() : nullptr),
      tile_thread_2_csr_offsets_(config.HasThreadCsrOffsets() ?
        &config.GetTileThread2CsrOffsets() : nullptr),
      tile_thread_3_csr_offsets_(config.HasThreadCsrOffsets() ?
        &config.GetTileThread3CsrOffsets() : nullptr),
      tile_thread_4_csr_offsets_(config.HasThreadCsrOffsets() ?
        &config.GetTileThread4CsrOffsets() : nullptr),
      tile_thread_5_csr_offsets_(config.HasThreadCsrOffsets() ?
        &config.GetTileThread5CsrOffsets() : nullptr),
      tile_thread_6_csr_offsets_(config.HasThreadCsrOffsets() ?
        &config.GetTileThread6CsrOffsets() : nullptr),
      tile_thread_7_csr_offsets_(config.HasThreadCsrOffsets() ?
        &config.GetTileThread7CsrOffsets() : nullptr),
      registers_(registers) {
  CHECK(registers != nullptr);
}

util::Status RunController::DoRunControl(RunControl run_state) {
  // Value of offset when register is not present in a project.
  constexpr uint64 kInvalidOffset = static_cast<uint64>(-1);

  const uint64 run_state_value = static_cast<uint64>(run_state);
  if (scalar_core_csr_offsets_.scalarCoreRunControl != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
      scalar_core_csr_offsets_.scalarCoreRunControl, run_state_value));
  } else {
    RETURN_IF_ERROR(registers_->Write(
      scalar_core_csr_offsets_.scalarDatapath_0RunControl, run_state_value));
  }
  if (scalar_core_csr_offsets_.avDataPopRunControl != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
      scalar_core_csr_offsets_.avDataPopRunControl, run_state_value));
  } else {
    RETURN_IF_ERROR(registers_->Write(
      scalar_core_csr_offsets_.avDataPop_0RunControl, run_state_value));
  }
  if (scalar_core_csr_offsets_.parameterPopRunControl != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
      scalar_core_csr_offsets_.parameterPopRunControl, run_state_value));
  } else {
    RETURN_IF_ERROR(registers_->Write(
      scalar_core_csr_offsets_.parameterPop_0RunControl, run_state_value));
  }
  if (scalar_core_csr_offsets_.infeedRunControl != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        scalar_core_csr_offsets_.infeedRunControl, run_state_value));
  } else {
    RETURN_IF_ERROR(registers_->Write(
        scalar_core_csr_offsets_.infeed_0_0RunControl, run_state_value));
  }
  if (scalar_core_csr_offsets_.outfeedRunControl != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        scalar_core_csr_offsets_.outfeedRunControl, run_state_value));
  } else {
    RETURN_IF_ERROR(registers_->Write(
        scalar_core_csr_offsets_.outfeed_0_0RunControl, run_state_value));
  }
  if (scalar_core_csr_offsets_.infeed1RunControl != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        scalar_core_csr_offsets_.infeed1RunControl, run_state_value));
  }
  if (scalar_core_csr_offsets_.infeed_0_1RunControl != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        scalar_core_csr_offsets_.infeed_0_1RunControl, run_state_value));
  }
  if (scalar_core_csr_offsets_.outfeed1RunControl != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        scalar_core_csr_offsets_.outfeed1RunControl, run_state_value));
  }
  if (scalar_core_csr_offsets_.outfeed_0_1RunControl != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        scalar_core_csr_offsets_.outfeed_0_1RunControl, run_state_value));
  }

  // TODO: helper uses 7-bits as defined by CSR. Extract bitwidth
  // automatically for different chips.
  config::registers::TileConfig<7> helper;
  helper.set_broadcast();
  RETURN_IF_ERROR(
      registers_->Write(tile_config_csr_offsets_.tileconfig0, helper.raw()));

  // Wait until tileconfig0 is set correctly. Subsequent writes are going to
  // tiles, but hardware does not guarantee correct ordering with previous
  // write.
  // TODO
  RETURN_IF_ERROR(
      registers_->Poll(tile_config_csr_offsets_.tileconfig0, helper.raw()));
  if (tile_csr_offsets_.opRunControl != kInvalidOffset) {
    RETURN_IF_ERROR(
        registers_->Write(tile_csr_offsets_.opRunControl, run_state_value));
  }
  if (tile_csr_offsets_.opRunControl_0 != kInvalidOffset) {
    RETURN_IF_ERROR(
        registers_->Write(tile_csr_offsets_.opRunControl_0, run_state_value));
  }
  if (tile_csr_offsets_.opRunControl_1 != kInvalidOffset) {
    RETURN_IF_ERROR(
        registers_->Write(tile_csr_offsets_.opRunControl_1, run_state_value));
  }
  if (tile_csr_offsets_.opRunControl_2 != kInvalidOffset) {
    RETURN_IF_ERROR(
        registers_->Write(tile_csr_offsets_.opRunControl_2, run_state_value));
  }
  if (tile_csr_offsets_.opRunControl_3 != kInvalidOffset) {
    RETURN_IF_ERROR(
        registers_->Write(tile_csr_offsets_.opRunControl_3, run_state_value));
  }
  if (tile_csr_offsets_.opRunControl_4 != kInvalidOffset) {
    RETURN_IF_ERROR(
        registers_->Write(tile_csr_offsets_.opRunControl_4, run_state_value));
  }
  if (tile_csr_offsets_.opRunControl_5 != kInvalidOffset) {
    RETURN_IF_ERROR(
        registers_->Write(tile_csr_offsets_.opRunControl_5, run_state_value));
  }
  if (tile_csr_offsets_.opRunControl_6 != kInvalidOffset) {
    RETURN_IF_ERROR(
        registers_->Write(tile_csr_offsets_.opRunControl_6, run_state_value));
  }
  if (tile_csr_offsets_.opRunControl_7 != kInvalidOffset) {
    RETURN_IF_ERROR(
        registers_->Write(tile_csr_offsets_.opRunControl_7, run_state_value));
  }
  if (tile_csr_offsets_.narrowToWideRunControl != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(tile_csr_offsets_.narrowToWideRunControl,
                                      run_state_value));
  }
  if (tile_csr_offsets_.narrowToWideRunControl_0 != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        tile_csr_offsets_.narrowToWideRunControl_0, run_state_value));
  }
  if (tile_csr_offsets_.narrowToWideRunControl_1 != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        tile_csr_offsets_.narrowToWideRunControl_1, run_state_value));
  }
  if (tile_csr_offsets_.narrowToWideRunControl_2 != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        tile_csr_offsets_.narrowToWideRunControl_2, run_state_value));
  }
  if (tile_csr_offsets_.narrowToWideRunControl_3 != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        tile_csr_offsets_.narrowToWideRunControl_3, run_state_value));
  }
  if (tile_csr_offsets_.narrowToWideRunControl_4 != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        tile_csr_offsets_.narrowToWideRunControl_4, run_state_value));
  }
  if (tile_csr_offsets_.narrowToWideRunControl_5 != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        tile_csr_offsets_.narrowToWideRunControl_5, run_state_value));
  }
  if (tile_csr_offsets_.narrowToWideRunControl_6 != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        tile_csr_offsets_.narrowToWideRunControl_6, run_state_value));
  }
  if (tile_csr_offsets_.narrowToWideRunControl_7 != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        tile_csr_offsets_.narrowToWideRunControl_7, run_state_value));
  }
  if (tile_csr_offsets_.wideToNarrowRunControl != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(tile_csr_offsets_.wideToNarrowRunControl,
                                      run_state_value));
  }
  if (tile_csr_offsets_.wideToNarrowRunControl_0 != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        tile_csr_offsets_.wideToNarrowRunControl_0, run_state_value));
  }
  if (tile_csr_offsets_.wideToNarrowRunControl_1 != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        tile_csr_offsets_.wideToNarrowRunControl_1, run_state_value));
  }
  if (tile_csr_offsets_.wideToNarrowRunControl_2 != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        tile_csr_offsets_.wideToNarrowRunControl_2, run_state_value));
  }
  if (tile_csr_offsets_.wideToNarrowRunControl_3 != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        tile_csr_offsets_.wideToNarrowRunControl_3, run_state_value));
  }
  if (tile_csr_offsets_.wideToNarrowRunControl_4 != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        tile_csr_offsets_.wideToNarrowRunControl_4, run_state_value));
  }
  if (tile_csr_offsets_.wideToNarrowRunControl_5 != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        tile_csr_offsets_.wideToNarrowRunControl_5, run_state_value));
  }
  if (tile_csr_offsets_.wideToNarrowRunControl_6 != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        tile_csr_offsets_.wideToNarrowRunControl_6, run_state_value));
  }
  if (tile_csr_offsets_.wideToNarrowRunControl_7 != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        tile_csr_offsets_.wideToNarrowRunControl_7, run_state_value));
  }

  const std::vector<const config::TileThreadCsrOffsets*>
      tile_thread_csr_offsets = {
          tile_thread_0_csr_offsets_, tile_thread_1_csr_offsets_,
          tile_thread_2_csr_offsets_, tile_thread_3_csr_offsets_,
          tile_thread_4_csr_offsets_, tile_thread_5_csr_offsets_,
          tile_thread_6_csr_offsets_, tile_thread_7_csr_offsets_};
  if (has_thread_csr_offsets_) {
    for (const auto* tile_thread_csr_offsets_ : tile_thread_csr_offsets) {
      if (tile_thread_csr_offsets_->opRunControl_0 != kInvalidOffset) {
        RETURN_IF_ERROR(registers_->Write(
          tile_thread_csr_offsets_->opRunControl_0, run_state_value));
      }
      if (tile_thread_csr_offsets_->narrowToWideRunControl_0 !=
          kInvalidOffset) {
        RETURN_IF_ERROR(registers_->Write(
          tile_thread_csr_offsets_->narrowToWideRunControl_0, run_state_value));
      }
      if (tile_thread_csr_offsets_->wideToNarrowRunControl_0 !=
          kInvalidOffset) {
        RETURN_IF_ERROR(registers_->Write(
          tile_thread_csr_offsets_->wideToNarrowRunControl_0, run_state_value));
      }
    }
  }

  RETURN_IF_ERROR(
      registers_->Write(tile_csr_offsets_.meshBus0RunControl, run_state_value));
  RETURN_IF_ERROR(
      registers_->Write(tile_csr_offsets_.meshBus1RunControl, run_state_value));
  RETURN_IF_ERROR(
      registers_->Write(tile_csr_offsets_.meshBus2RunControl, run_state_value));
  RETURN_IF_ERROR(
      registers_->Write(tile_csr_offsets_.meshBus3RunControl, run_state_value));
  RETURN_IF_ERROR(registers_->Write(
      tile_csr_offsets_.ringBusConsumer0RunControl, run_state_value));
  RETURN_IF_ERROR(registers_->Write(
      tile_csr_offsets_.ringBusConsumer1RunControl, run_state_value));
  RETURN_IF_ERROR(registers_->Write(tile_csr_offsets_.ringBusProducerRunControl,
                                    run_state_value));
  if (tile_csr_offsets_.narrowToNarrowRunControl != kInvalidOffset) {
    RETURN_IF_ERROR(registers_->Write(
        tile_csr_offsets_.narrowToNarrowRunControl, run_state_value));
  }

  return util::Status();  // OK
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
