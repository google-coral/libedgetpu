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

#ifndef DARWINN_DRIVER_CONFIG_DEBUG_TILE_CSR_OFFSETS_H_
#define DARWINN_DRIVER_CONFIG_DEBUG_TILE_CSR_OFFSETS_H_

#include "port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets that will be dumped as part of the
// driver bug report for tiles. Members are intentionally named to match
// the GCSR register names.
struct DebugTileCsrOffsets {
  uint64 TileClockControl;
  uint64 tileid;
  uint64 scratchpad;
  uint64 memoryAccess;
  uint64 memoryData;
  uint64 narrowMemoryContext_0;
  uint64 narrowMemoryContext_1;
  uint64 narrowMemoryContext_2;
  uint64 narrowMemoryContext_3;
  uint64 deepSleep;
  uint64 SyncCounter_AVDATA;
  uint64 SyncCounter_PARAMETERS;
  uint64 SyncCounter_PARTIAL_SUMS;
  uint64 SyncCounter_MESH_NORTH_IN;
  uint64 SyncCounter_MESH_EAST_IN;
  uint64 SyncCounter_MESH_SOUTH_IN;
  uint64 SyncCounter_MESH_WEST_IN;
  uint64 SyncCounter_MESH_NORTH_OUT;
  uint64 SyncCounter_MESH_EAST_OUT;
  uint64 SyncCounter_MESH_SOUTH_OUT;
  uint64 SyncCounter_MESH_WEST_OUT;
  uint64 SyncCounter_WIDE_TO_NARROW;
  uint64 SyncCounter_WIDE_TO_SCALING;
  uint64 SyncCounter_NARROW_TO_WIDE;
  uint64 SyncCounter_RING_READ_A;
  uint64 SyncCounter_RING_READ_B;
  uint64 SyncCounter_RING_WRITE;
  uint64 SyncCounter_RING_PRODUCER_A;
  uint64 SyncCounter_RING_PRODUCER_B;
  uint64 opRunControl;
  uint64 PowerSaveData;
  uint64 opBreakPoint;
  uint64 StallCounter;
  uint64 opRunStatus;
  uint64 OpOverwriteMode;
  uint64 OpEnableTracing;
  uint64 OpStartCycle;
  uint64 OpEndCycle;
  uint64 OpStallCycleCount;
  uint64 OpProgramCounter;
  uint64 wideToNarrowRunControl;
  uint64 wideToNarrowRunStatus;
  uint64 wideToNarrowBreakPoint;
  uint64 dmaWideToNarrowOverwriteMode;
  uint64 dmaWideToNarrowEnableTracing;
  uint64 dmaWideToNarrowStartCycle;
  uint64 dmaWideToNarrowEndCycle;
  uint64 dmaWideToNarrowStallCycleCount;
  uint64 dmaWideToNarrowProgramCounter;
  uint64 narrowToWideRunControl;
  uint64 narrowToWideRunStatus;
  uint64 narrowToWideBreakPoint;
  uint64 dmaNarrowToWideOverwriteMode;
  uint64 dmaNarrowToWideEnableTracing;
  uint64 dmaNarrowToWideStartCycle;
  uint64 dmaNarrowToWideEndCycle;
  uint64 dmaNarrowToWideStallCycleCount;
  uint64 dmaNarrowToWideProgramCounter;
  uint64 ringBusConsumer0RunControl;
  uint64 ringBusConsumer0RunStatus;
  uint64 ringBusConsumer0BreakPoint;
  uint64 dmaRingBusConsumer0OverwriteMode;
  uint64 dmaRingBusConsumer0EnableTracing;
  uint64 dmaRingBusConsumer0StartCycle;
  uint64 dmaRingBusConsumer0EndCycle;
  uint64 dmaRingBusConsumer0StallCycleCount;
  uint64 dmaRingBusConsumer0ProgramCounter;
  uint64 ringBusConsumer1RunControl;
  uint64 ringBusConsumer1RunStatus;
  uint64 ringBusConsumer1BreakPoint;
  uint64 dmaRingBusConsumer1OverwriteMode;
  uint64 dmaRingBusConsumer1EnableTracing;
  uint64 dmaRingBusConsumer1StartCycle;
  uint64 dmaRingBusConsumer1EndCycle;
  uint64 dmaRingBusConsumer1StallCycleCount;
  uint64 dmaRingBusConsumer1ProgramCounter;
  uint64 ringBusProducerRunControl;
  uint64 ringBusProducerRunStatus;
  uint64 ringBusProducerBreakPoint;
  uint64 dmaRingBusProducerOverwriteMode;
  uint64 dmaRingBusProducerEnableTracing;
  uint64 dmaRingBusProducerStartCycle;
  uint64 dmaRingBusProducerEndCycle;
  uint64 dmaRingBusProducerStallCycleCount;
  uint64 dmaRingBusProducerProgramCounter;
  uint64 meshBus0RunControl;
  uint64 meshBus0RunStatus;
  uint64 meshBus0BreakPoint;
  uint64 dmaMeshBus0OverwriteMode;
  uint64 dmaMeshBus0EnableTracing;
  uint64 dmaMeshBus0StartCycle;
  uint64 dmaMeshBus0EndCycle;
  uint64 dmaMeshBus0StallCycleCount;
  uint64 dmaMeshBus0ProgramCounter;
  uint64 meshBus1RunControl;
  uint64 meshBus1RunStatus;
  uint64 meshBus1BreakPoint;
  uint64 dmaMeshBus1OverwriteMode;
  uint64 dmaMeshBus1EnableTracing;
  uint64 dmaMeshBus1StartCycle;
  uint64 dmaMeshBus1EndCycle;
  uint64 dmaMeshBus1StallCycleCount;
  uint64 dmaMeshBus1ProgramCounter;
  uint64 meshBus2RunControl;
  uint64 meshBus2RunStatus;
  uint64 meshBus2BreakPoint;
  uint64 dmaMeshBus2OverwriteMode;
  uint64 dmaMeshBus2EnableTracing;
  uint64 dmaMeshBus2StartCycle;
  uint64 dmaMeshBus2EndCycle;
  uint64 dmaMeshBus2StallCycleCount;
  uint64 dmaMeshBus2ProgramCounter;
  uint64 meshBus3RunControl;
  uint64 meshBus3RunStatus;
  uint64 meshBus3BreakPoint;
  uint64 dmaMeshBus3OverwriteMode;
  uint64 dmaMeshBus3EnableTracing;
  uint64 dmaMeshBus3StartCycle;
  uint64 dmaMeshBus3EndCycle;
  uint64 dmaMeshBus3StallCycleCount;
  uint64 dmaMeshBus3ProgramCounter;
  uint64 Error_Tile;
  uint64 Error_Mask_Tile;
  uint64 Error_Force_Tile;
  uint64 Error_Timestamp_Tile;
  uint64 Error_Info_Tile;
  uint64 Timeout;
  uint64 opTtuStateRegFile;
  uint64 OpTrace;
  uint64 wideToNarrowTtuStateRegFile;
  uint64 dmaWideToNarrowTrace;
  uint64 narrowToWideTtuStateRegFile;
  uint64 dmaNarrowToWideTrace;
  uint64 ringBusConsumer0TtuStateRegFile;
  uint64 dmaRingBusConsumer0Trace;
  uint64 ringBusConsumer1TtuStateRegFile;
  uint64 dmaRingBusConsumer1Trace;
  uint64 ringBusProducerTtuStateRegFile;
  uint64 dmaRingBusProducerTrace;
  uint64 meshBus0TtuStateRegFile;
  uint64 dmaMeshBus0Trace;
  uint64 meshBus1TtuStateRegFile;
  uint64 dmaMeshBus1Trace;
  uint64 meshBus2TtuStateRegFile;
  uint64 dmaMeshBus2Trace;
  uint64 meshBus3TtuStateRegFile;
  uint64 dmaMeshBus3Trace;
  uint64 narrowMemoryIsolation;
  uint64 narrowMemoryRetention;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_CONFIG_DEBUG_TILE_CSR_OFFSETS_H_
