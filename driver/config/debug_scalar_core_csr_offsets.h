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

#ifndef DARWINN_DRIVER_CONFIG_DEBUG_SCALAR_CORE_CSR_OFFSETS_H_
#define DARWINN_DRIVER_CONFIG_DEBUG_SCALAR_CORE_CSR_OFFSETS_H_

#include "port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets that will be dumped as part of the
// driver bug report for scalar core. Members are intentionally named to match
// the GCSR register names.
struct DebugScalarCoreCsrOffsets {
  uint64 topology;
  uint64 scMemoryCapacity;
  uint64 tileMemoryCapacity;
  uint64 scMemoryAccess;
  uint64 scMemoryData;
  uint64 Timeout;
  uint64 Error_ScalarCore;
  uint64 Error_Mask_ScalarCore;
  uint64 Error_Force_ScalarCore;
  uint64 Error_Timestamp_ScalarCore;
  uint64 Error_Info_ScalarCore;

  // Contextual CSRs in Scalar Datapaths
  uint64 scalarCoreRunControl;
  uint64 scalarCoreBreakPoint;
  uint64 currentPc;
  uint64 executeControl;
  // Context 0
  uint64 scalarDatapath_0RunControl;
  uint64 scalarDatapath_0BreakPoint;
  uint64 currentPc_0;
  uint64 executeControl_0;
  // Context 1
  uint64 scalarDatapath_1RunControl;
  uint64 scalarDatapath_1BreakPoint;
  uint64 currentPc_1;
  uint64 executeControl_1;
  // Context 2
  uint64 scalarDatapath_2RunControl;
  uint64 scalarDatapath_2BreakPoint;
  uint64 currentPc_2;
  uint64 executeControl_2;
  // Context 3
  uint64 scalarDatapath_3RunControl;
  uint64 scalarDatapath_3BreakPoint;
  uint64 currentPc_3;
  uint64 executeControl_3;
  // Sync Flag CSRs
  uint64 SyncCounter_AVDATA_POP;
  uint64 SyncCounter_PARAMETER_POP;
  uint64 SyncCounter_AVDATA_INFEED;
  uint64 SyncCounter_PARAMETER_INFEED;
  uint64 SyncCounter_SCALAR_INFEED;
  uint64 SyncCounter_PRODUCER_A;
  uint64 SyncCounter_PRODUCER_B;
  uint64 SyncCounter_RING_OUTFEED;
  // Context 0
  uint64 SyncCounter_AVDATA_POP_0_0;
  uint64 SyncCounter_PARAMETER_POP_0_0;
  uint64 SyncCounter_AVDATA_INFEED_0_0;
  uint64 SyncCounter_PARAMETER_INFEED_0_0;
  uint64 SyncCounter_SCALAR_INFEED_0_0;
  uint64 SyncCounter_PRODUCER_A_0_0;
  uint64 SyncCounter_PRODUCER_B_0_0;
  uint64 SyncCounter_RING_OUTFEED_0_0;
  // Context 1
  uint64 SyncCounter_AVDATA_POP_1_0;
  uint64 SyncCounter_PARAMETER_POP_1_0;
  uint64 SyncCounter_AVDATA_INFEED_1_0;
  uint64 SyncCounter_PARAMETER_INFEED_1_0;
  uint64 SyncCounter_SCALAR_INFEED_1_0;
  uint64 SyncCounter_PRODUCER_A_1_0;
  uint64 SyncCounter_PRODUCER_B_1_0;
  uint64 SyncCounter_RING_OUTFEED_1_0;
  // Context 2
  uint64 SyncCounter_AVDATA_POP_2_0;
  uint64 SyncCounter_PARAMETER_POP_2_0;
  uint64 SyncCounter_AVDATA_INFEED_2_0;
  uint64 SyncCounter_PARAMETER_INFEED_2_0;
  uint64 SyncCounter_SCALAR_INFEED_2_0;
  uint64 SyncCounter_PRODUCER_A_2_0;
  uint64 SyncCounter_PRODUCER_B_2_0;
  uint64 SyncCounter_RING_OUTFEED_2_0;
  // Context 3
  uint64 SyncCounter_AVDATA_POP_3_0;
  uint64 SyncCounter_PARAMETER_POP_3_0;
  uint64 SyncCounter_AVDATA_INFEED_3_0;
  uint64 SyncCounter_PARAMETER_INFEED_3_0;
  uint64 SyncCounter_SCALAR_INFEED_3_0;
  uint64 SyncCounter_PRODUCER_A_3_0;
  uint64 SyncCounter_PRODUCER_B_3_0;
  uint64 SyncCounter_RING_OUTFEED_3_0;

  // Pop Input Control Units
  uint64 avDataPopRunControl;
  uint64 avDataPopBreakPoint;
  uint64 avDataPopRunStatus;
  uint64 avDataPopOverwriteMode;
  uint64 avDataPopEnableTracing;
  uint64 avDataPopStartCycle;
  uint64 avDataPopEndCycle;
  uint64 avDataPopStallCycleCount;
  uint64 avDataPopProgramCounter;
  uint64 avDataPopTtuStateRegFile;
  uint64 avDataPopTrace;
  // Context 0
  uint64 avDataPop_0RunControl;
  uint64 avDataPop_0BreakPoint;
  uint64 avDataPop_0RunStatus;
  uint64 avDataPop_0OverwriteMode;
  uint64 avDataPop_0EnableTracing;
  uint64 avDataPop_0StartCycle;
  uint64 avDataPop_0EndCycle;
  uint64 avDataPop_0StallCycleCount;
  uint64 avDataPop_0ProgramCounter;
  uint64 avDataPop_0TtuStateRegFile;
  uint64 avDataPop_0Trace;
  // Context 1
  uint64 avDataPop_1RunControl;
  uint64 avDataPop_1BreakPoint;
  uint64 avDataPop_1RunStatus;
  uint64 avDataPop_1OverwriteMode;
  uint64 avDataPop_1EnableTracing;
  uint64 avDataPop_1StartCycle;
  uint64 avDataPop_1EndCycle;
  uint64 avDataPop_1StallCycleCount;
  uint64 avDataPop_1ProgramCounter;
  uint64 avDataPop_1TtuStateRegFile;
  uint64 avDataPop_1Trace;
  // Context 2
  uint64 avDataPop_2RunControl;
  uint64 avDataPop_2BreakPoint;
  uint64 avDataPop_2RunStatus;
  uint64 avDataPop_2OverwriteMode;
  uint64 avDataPop_2EnableTracing;
  uint64 avDataPop_2StartCycle;
  uint64 avDataPop_2EndCycle;
  uint64 avDataPop_2StallCycleCount;
  uint64 avDataPop_2ProgramCounter;
  uint64 avDataPop_2TtuStateRegFile;
  uint64 avDataPop_2Trace;
  // Context 3
  uint64 avDataPop_3RunControl;
  uint64 avDataPop_3BreakPoint;
  uint64 avDataPop_3RunStatus;
  uint64 avDataPop_3OverwriteMode;
  uint64 avDataPop_3EnableTracing;
  uint64 avDataPop_3StartCycle;
  uint64 avDataPop_3EndCycle;
  uint64 avDataPop_3StallCycleCount;
  uint64 avDataPop_3ProgramCounter;
  uint64 avDataPop_3TtuStateRegFile;
  uint64 avDataPop_3Trace;

  uint64 parameterPopRunControl;
  uint64 parameterPopBreakPoint;
  uint64 parameterPopRunStatus;
  uint64 parameterPopOverwriteMode;
  uint64 parameterPopEnableTracing;
  uint64 parameterPopStartCycle;
  uint64 parameterPopEndCycle;
  uint64 parameterPopStallCycleCount;
  uint64 parameterPopProgramCounter;
  uint64 parameterPopTtuStateRegFile;
  uint64 parameterPopTrace;
  // Context 0
  uint64 parameterPop_0RunControl;
  uint64 parameterPop_0BreakPoint;
  uint64 parameterPop_0RunStatus;
  uint64 parameterPop_0OverwriteMode;
  uint64 parameterPop_0EnableTracing;
  uint64 parameterPop_0StartCycle;
  uint64 parameterPop_0EndCycle;
  uint64 parameterPop_0StallCycleCount;
  uint64 parameterPop_0ProgramCounter;
  uint64 parameterPop_0TtuStateRegFile;
  uint64 parameterPop_0Trace;
  // Context 1
  uint64 parameterPop_1RunControl;
  uint64 parameterPop_1BreakPoint;
  uint64 parameterPop_1RunStatus;
  uint64 parameterPop_1OverwriteMode;
  uint64 parameterPop_1EnableTracing;
  uint64 parameterPop_1StartCycle;
  uint64 parameterPop_1EndCycle;
  uint64 parameterPop_1StallCycleCount;
  uint64 parameterPop_1ProgramCounter;
  uint64 parameterPop_1TtuStateRegFile;
  uint64 parameterPop_1Trace;
  // Context 2
  uint64 parameterPop_2RunControl;
  uint64 parameterPop_2BreakPoint;
  uint64 parameterPop_2RunStatus;
  uint64 parameterPop_2OverwriteMode;
  uint64 parameterPop_2EnableTracing;
  uint64 parameterPop_2StartCycle;
  uint64 parameterPop_2EndCycle;
  uint64 parameterPop_2StallCycleCount;
  uint64 parameterPop_2ProgramCounter;
  uint64 parameterPop_2TtuStateRegFile;
  uint64 parameterPop_2Trace;
  // Context 3
  uint64 parameterPop_3RunControl;
  uint64 parameterPop_3BreakPoint;
  uint64 parameterPop_3RunStatus;
  uint64 parameterPop_3OverwriteMode;
  uint64 parameterPop_3EnableTracing;
  uint64 parameterPop_3StartCycle;
  uint64 parameterPop_3EndCycle;
  uint64 parameterPop_3StallCycleCount;
  uint64 parameterPop_3ProgramCounter;
  uint64 parameterPop_3TtuStateRegFile;
  uint64 parameterPop_3Trace;
  // Infeed Control Units
  uint64 infeedRunControl;
  uint64 infeedRunStatus;
  uint64 infeedBreakPoint;
  uint64 infeedOverwriteMode;
  uint64 infeedEnableTracing;
  uint64 infeedStartCycle;
  uint64 infeedEndCycle;
  uint64 infeedStallCycleCount;
  uint64 infeedProgramCounter;
  uint64 infeedTtuStateRegFile;
  // Context 0
  uint64 infeed_0_0RunControl;
  uint64 infeed_0_0RunStatus;
  uint64 infeed_0_0BreakPoint;
  uint64 infeed_0_0OverwriteMode;
  uint64 infeed_0_0EnableTracing;
  uint64 infeed_0_0StartCycle;
  uint64 infeed_0_0EndCycle;
  uint64 infeed_0_0StallCycleCount;
  uint64 infeed_0_0ProgramCounter;
  uint64 infeed_0_0TtuStateRegFile;
  uint64 infeed_0_1RunControl;
  uint64 infeed_0_1RunStatus;
  uint64 infeed_0_1BreakPoint;
  uint64 infeed_0_1OverwriteMode;
  uint64 infeed_0_1EnableTracing;
  uint64 infeed_0_1StartCycle;
  uint64 infeed_0_1EndCycle;
  uint64 infeed_0_1StallCycleCount;
  uint64 infeed_0_1ProgramCounter;
  uint64 infeed_0_1TtuStateRegFile;
  // Context 1
  uint64 infeed_1_0RunControl;
  uint64 infeed_1_0RunStatus;
  uint64 infeed_1_0BreakPoint;
  uint64 infeed_1_0OverwriteMode;
  uint64 infeed_1_0EnableTracing;
  uint64 infeed_1_0StartCycle;
  uint64 infeed_1_0EndCycle;
  uint64 infeed_1_0StallCycleCount;
  uint64 infeed_1_0ProgramCounter;
  uint64 infeed_1_0TtuStateRegFile;
  uint64 infeed_1_1RunControl;
  uint64 infeed_1_1RunStatus;
  uint64 infeed_1_1BreakPoint;
  uint64 infeed_1_1OverwriteMode;
  uint64 infeed_1_1EnableTracing;
  uint64 infeed_1_1StartCycle;
  uint64 infeed_1_1EndCycle;
  uint64 infeed_1_1StallCycleCount;
  uint64 infeed_1_1ProgramCounter;
  uint64 infeed_1_1TtuStateRegFile;
  // Context 2
  uint64 infeed_2_0RunControl;
  uint64 infeed_2_0RunStatus;
  uint64 infeed_2_0BreakPoint;
  uint64 infeed_2_0OverwriteMode;
  uint64 infeed_2_0EnableTracing;
  uint64 infeed_2_0StartCycle;
  uint64 infeed_2_0EndCycle;
  uint64 infeed_2_0StallCycleCount;
  uint64 infeed_2_0ProgramCounter;
  uint64 infeed_2_0TtuStateRegFile;
  uint64 infeed_2_1RunControl;
  uint64 infeed_2_1RunStatus;
  uint64 infeed_2_1BreakPoint;
  uint64 infeed_2_1OverwriteMode;
  uint64 infeed_2_1EnableTracing;
  uint64 infeed_2_1StartCycle;
  uint64 infeed_2_1EndCycle;
  uint64 infeed_2_1StallCycleCount;
  uint64 infeed_2_1ProgramCounter;
  uint64 infeed_2_1TtuStateRegFile;
  // Context 3
  uint64 infeed_3_0RunControl;
  uint64 infeed_3_0RunStatus;
  uint64 infeed_3_0BreakPoint;
  uint64 infeed_3_0OverwriteMode;
  uint64 infeed_3_0EnableTracing;
  uint64 infeed_3_0StartCycle;
  uint64 infeed_3_0EndCycle;
  uint64 infeed_3_0StallCycleCount;
  uint64 infeed_3_0ProgramCounter;
  uint64 infeed_3_0TtuStateRegFile;
  uint64 infeed_3_1RunControl;
  uint64 infeed_3_1RunStatus;
  uint64 infeed_3_1BreakPoint;
  uint64 infeed_3_1OverwriteMode;
  uint64 infeed_3_1EnableTracing;
  uint64 infeed_3_1StartCycle;
  uint64 infeed_3_1EndCycle;
  uint64 infeed_3_1StallCycleCount;
  uint64 infeed_3_1ProgramCounter;
  uint64 infeed_3_1TtuStateRegFile;

  // Outfeed Control Units
  uint64 outfeedRunControl;
  uint64 outfeedRunStatus;
  uint64 outfeedBreakPoint;
  uint64 outfeedOverwriteMode;
  uint64 outfeedEnableTracing;
  uint64 outfeedStartCycle;
  uint64 outfeedEndCycle;
  uint64 outfeedStallCycleCount;
  uint64 outfeedProgramCounter;
  uint64 outfeedTtuStateRegFile;
  // Context 0
  uint64 outfeed_0_0RunControl;
  uint64 outfeed_0_0RunStatus;
  uint64 outfeed_0_0BreakPoint;
  uint64 outfeed_0_0OverwriteMode;
  uint64 outfeed_0_0EnableTracing;
  uint64 outfeed_0_0StartCycle;
  uint64 outfeed_0_0EndCycle;
  uint64 outfeed_0_0StallCycleCount;
  uint64 outfeed_0_0ProgramCounter;
  uint64 outfeed_0_0TtuStateRegFile;
  uint64 outfeed_0_1RunControl;
  uint64 outfeed_0_1RunStatus;
  uint64 outfeed_0_1BreakPoint;
  uint64 outfeed_0_1OverwriteMode;
  uint64 outfeed_0_1EnableTracing;
  uint64 outfeed_0_1StartCycle;
  uint64 outfeed_0_1EndCycle;
  uint64 outfeed_0_1StallCycleCount;
  uint64 outfeed_0_1ProgramCounter;
  uint64 outfeed_0_1TtuStateRegFile;
  // Context 1
  uint64 outfeed_1_0RunControl;
  uint64 outfeed_1_0RunStatus;
  uint64 outfeed_1_0BreakPoint;
  uint64 outfeed_1_0OverwriteMode;
  uint64 outfeed_1_0EnableTracing;
  uint64 outfeed_1_0StartCycle;
  uint64 outfeed_1_0EndCycle;
  uint64 outfeed_1_0StallCycleCount;
  uint64 outfeed_1_0ProgramCounter;
  uint64 outfeed_1_0TtuStateRegFile;
  uint64 outfeed_1_1RunControl;
  uint64 outfeed_1_1RunStatus;
  uint64 outfeed_1_1BreakPoint;
  uint64 outfeed_1_1OverwriteMode;
  uint64 outfeed_1_1EnableTracing;
  uint64 outfeed_1_1StartCycle;
  uint64 outfeed_1_1EndCycle;
  uint64 outfeed_1_1StallCycleCount;
  uint64 outfeed_1_1ProgramCounter;
  uint64 outfeed_1_1TtuStateRegFile;
  // Context 2
  uint64 outfeed_2_0RunControl;
  uint64 outfeed_2_0RunStatus;
  uint64 outfeed_2_0BreakPoint;
  uint64 outfeed_2_0OverwriteMode;
  uint64 outfeed_2_0EnableTracing;
  uint64 outfeed_2_0StartCycle;
  uint64 outfeed_2_0EndCycle;
  uint64 outfeed_2_0StallCycleCount;
  uint64 outfeed_2_0ProgramCounter;
  uint64 outfeed_2_0TtuStateRegFile;
  uint64 outfeed_2_1RunControl;
  uint64 outfeed_2_1RunStatus;
  uint64 outfeed_2_1BreakPoint;
  uint64 outfeed_2_1OverwriteMode;
  uint64 outfeed_2_1EnableTracing;
  uint64 outfeed_2_1StartCycle;
  uint64 outfeed_2_1EndCycle;
  uint64 outfeed_2_1StallCycleCount;
  uint64 outfeed_2_1ProgramCounter;
  uint64 outfeed_2_1TtuStateRegFile;
  // Context 3
  uint64 outfeed_3_0RunControl;
  uint64 outfeed_3_0RunStatus;
  uint64 outfeed_3_0BreakPoint;
  uint64 outfeed_3_0OverwriteMode;
  uint64 outfeed_3_0EnableTracing;
  uint64 outfeed_3_0StartCycle;
  uint64 outfeed_3_0EndCycle;
  uint64 outfeed_3_0StallCycleCount;
  uint64 outfeed_3_0ProgramCounter;
  uint64 outfeed_3_0TtuStateRegFile;
  uint64 outfeed_3_1RunControl;
  uint64 outfeed_3_1RunStatus;
  uint64 outfeed_3_1BreakPoint;
  uint64 outfeed_3_1OverwriteMode;
  uint64 outfeed_3_1EnableTracing;
  uint64 outfeed_3_1StartCycle;
  uint64 outfeed_3_1EndCycle;
  uint64 outfeed_3_1StallCycleCount;
  uint64 outfeed_3_1ProgramCounter;
  uint64 outfeed_3_1TtuStateRegFile;

  // Scalar Pipeline
  uint64 scalarCoreRunStatus;
  uint64 scalarCoreRunStatus_0;
  uint64 scalarCoreRunStatus_1;
  uint64 scalarCoreRunStatus_2;
  uint64 scalarCoreRunStatus_3;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_CONFIG_DEBUG_SCALAR_CORE_CSR_OFFSETS_H_
