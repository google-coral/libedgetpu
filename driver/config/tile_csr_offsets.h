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

#ifndef DARWINN_DRIVER_CONFIG_TILE_CSR_OFFSETS_H_
#define DARWINN_DRIVER_CONFIG_TILE_CSR_OFFSETS_H_

#include "port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for tiles. Members are intentionally
// named to match the GCSR register names.
struct TileCsrOffsets {
  // RunControls to change run state.
  uint64 opRunControl;
  uint64 narrowToNarrowRunControl;
  uint64 narrowToWideRunControl;
  uint64 wideToNarrowRunControl;
  // When we enable the wider thread issue feature, we get multiple
  // of these run controls per pipeline for the opcontrol, narrow to wide
  // and wide to narrow. We're using 8 of these as a maximum issue width
  // at this point. The driver will only use the registers that are valid
  // for any given configuration.
  // TODO
  uint64 opRunControl_0;
  uint64 narrowToWideRunControl_0;
  uint64 wideToNarrowRunControl_0;
  uint64 opRunControl_1;
  uint64 narrowToWideRunControl_1;
  uint64 wideToNarrowRunControl_1;
  uint64 opRunControl_2;
  uint64 narrowToWideRunControl_2;
  uint64 wideToNarrowRunControl_2;
  uint64 opRunControl_3;
  uint64 narrowToWideRunControl_3;
  uint64 wideToNarrowRunControl_3;
  uint64 opRunControl_4;
  uint64 narrowToWideRunControl_4;
  uint64 wideToNarrowRunControl_4;
  uint64 opRunControl_5;
  uint64 narrowToWideRunControl_5;
  uint64 wideToNarrowRunControl_5;
  uint64 opRunControl_6;
  uint64 narrowToWideRunControl_6;
  uint64 wideToNarrowRunControl_6;
  uint64 opRunControl_7;
  uint64 narrowToWideRunControl_7;
  uint64 wideToNarrowRunControl_7;
  uint64 ringBusConsumer0RunControl;
  uint64 ringBusConsumer1RunControl;
  uint64 ringBusProducerRunControl;
  uint64 meshBus0RunControl;
  uint64 meshBus1RunControl;
  uint64 meshBus2RunControl;
  uint64 meshBus3RunControl;

  // Deep sleep register to control power state.
  uint64 deepSleep;

  // Narrow memory retention and isolation.
  uint64 narrowMemoryIsolation;
  uint64 narrowMemoryRetention;

  // Power related.
  uint64 EnergyTable;
  uint64 didtSampleInterval;
  uint64 didtRunningSumInterval;
  uint64 opAccumulateRegister;
  uint64 didtRunningSumRegister;
  uint64 didtThreshold0;

  // Narrow memory base and bound of virtual contexts.
  uint64 narrowMemoryContext_0;
  uint64 narrowMemoryContext_1;
  uint64 narrowMemoryContext_2;
  uint64 narrowMemoryContext_3;

  // Ring bus credit control
  uint64 TileRingBusCreditSenderReset;
  uint64 TileRingBusCreditReceiverReset;

  // Error Tile
  uint64 Error_Tile;
  uint64 Error_Mask_Tile;
  uint64 Error_Force_Tile;
  uint64 Error_Timestamp_Tile;
  uint64 Error_ContextId_Tile;
  uint64 Error_Info_Tile;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_CONFIG_TILE_CSR_OFFSETS_H_
