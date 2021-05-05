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

#ifndef DARWINN_DRIVER_CONFIG_SCALAR_CORE_CSR_OFFSETS_H_
#define DARWINN_DRIVER_CONFIG_SCALAR_CORE_CSR_OFFSETS_H_

#include "port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for scalar core.
// Members are intentionally named to match the GCSR register names.
struct ScalarCoreCsrOffsets {
  // RunControls.
  // Legacy
  uint64 scalarCoreRunControl;
  uint64 executeControl;
  uint64 avDataPopRunControl;
  uint64 parameterPopRunControl;
  uint64 scalarCoreRunStatus;
  uint64 avDataPopRunStatus;
  uint64 parameterPopRunStatus;

  // Context 0
  uint64 scalarDatapath_0RunControl;
  uint64 executeControl_0;
  uint64 avDataPop_0RunControl;
  uint64 parameterPop_0RunControl;
  uint64 scalarDatapath_0RunStatus;
  uint64 avDataPop_0RunStatus;
  uint64 parameterPop_0RunStatus;

  // Context 1
  uint64 scalarDatapath_1RunControl;
  uint64 executeControl_1;
  uint64 avDataPop_1RunControl;
  uint64 parameterPop_1RunControl;
  uint64 scalarDatapath_1RunStatus;
  uint64 avDataPop_1RunStatus;
  uint64 parameterPop_1RunStatus;

  // Context 2
  uint64 scalarDatapath_2RunControl;
  uint64 executeControl_2;
  uint64 avDataPop_2RunControl;
  uint64 parameterPop_2RunControl;
  uint64 scalarDatapath_2RunStatus;
  uint64 avDataPop_2RunStatus;
  uint64 parameterPop_2RunStatus;

  // Context 3
  uint64 scalarDatapath_3RunControl;
  uint64 executeControl_3;
  uint64 avDataPop_3RunControl;
  uint64 parameterPop_3RunControl;
  uint64 scalarDatapath_3RunStatus;
  uint64 avDataPop_3RunStatus;
  uint64 parameterPop_3RunStatus;

  // Legacy
  uint64 infeedRunControl;
  uint64 outfeedRunControl;
  uint64 infeedRunStatus;
  uint64 outfeedRunStatus;
  uint64 infeed1RunControl;
  uint64 outfeed1RunControl;

  // Context Switching
  uint64 contextControl;
  uint64 contextStatus;
  uint64 contextSwitchCount;
  uint64 contextSwitchTimeoutCount;

  // Multi Cluster mode switch related CSRs.
  uint64 clusterPortControl;
  uint64 clusterPortStatus;

  // Context 0
  uint64 infeed_0_0RunControl;
  uint64 outfeed_0_0RunControl;
  uint64 infeed_0_1RunControl;
  uint64 outfeed_0_1RunControl;
  uint64 infeed_0_0RunStatus;
  uint64 outfeed_0_0RunStatus;

  // Context 1
  uint64 infeed_1_0RunControl;
  uint64 outfeed_1_0RunControl;
  uint64 infeed_1_1RunControl;
  uint64 outfeed_1_1RunControl;
  uint64 infeed_1_0RunStatus;
  uint64 outfeed_1_0RunStatus;

  // Context 2
  uint64 infeed_2_0RunControl;
  uint64 outfeed_2_0RunControl;
  uint64 infeed_2_1RunControl;
  uint64 outfeed_2_1RunControl;
  uint64 infeed_2_0RunStatus;
  uint64 outfeed_2_0RunStatus;

  // Context 3
  uint64 infeed_3_0RunControl;
  uint64 outfeed_3_0RunControl;
  uint64 infeed_3_1RunControl;
  uint64 outfeed_3_1RunControl;
  uint64 infeed_3_0RunStatus;
  uint64 outfeed_3_0RunStatus;

  // Ring bus credit control
  // Context 0
  uint64 ScalarCoreRingBusCreditSender_0Reset;
  uint64 ScalarCoreRingBusCreditReceiver_0Reset;
  // Context 1
  uint64 ScalarCoreRingBusCreditSender_1Reset;
  uint64 ScalarCoreRingBusCreditReceiver_1Reset;
  // Context 2
  uint64 ScalarCoreRingBusCreditSender_2Reset;
  uint64 ScalarCoreRingBusCreditReceiver_2Reset;
  // Context 3
  uint64 ScalarCoreRingBusCreditSender_3Reset;
  uint64 ScalarCoreRingBusCreditReceiver_3Reset;

  // Power related.
  uint64 TilePowerInterval;
  uint64 peakPowerSampleInterval;
  uint64 tdpPowerSampleInterval;
  uint64 didtPowerSampleInterval;
  uint64 peakSampleAccumulator;
  uint64 tdpSampleAccumulator;
  uint64 didtSampleAccumulator;
  uint64 peakThreshold0;
  uint64 peakThreshold1;
  uint64 peakThreshold2;
  uint64 peakThreshold3;
  uint64 tdpThreshold0;
  uint64 tdpThreshold1;
  uint64 tdpThreshold2;
  uint64 tdpThreshold3;
  uint64 didtThreshold0;
  uint64 peakActionTable;
  uint64 tdpActionTable;
  uint64 didtActionTable;
  uint64 peakRunningSum;
  uint64 peakRunningSumInterval;
  uint64 tdpRunningSum;
  uint64 tdpRunningSumInterval;
  uint64 didtRunningSum;
  uint64 didtRunningSumInterval;
  uint64 didtDifference;
  uint64 packageTdpAction;
  uint64 ThrottleStallCounter;

  // Scalar core cycle count. This could be used to synchronize timestamp
  // between host and the TPU
  uint64 cycleCount;

  // Error Registers
  uint64 Error_ScalarCore;

  uint64 Error_ScalarCoreDatapath_0;
  uint64 Error_Mask_ScalarCoreDatapath_0;
  uint64 Error_Force_ScalarCoreDatapath_0;
  uint64 Error_Timestamp_ScalarCoreDatapath_0;
  uint64 Error_Info_ScalarCoreDatapath_0;

  uint64 Error_ScalarCoreDatapath_1;
  uint64 Error_Mask_ScalarCoreDatapath_1;
  uint64 Error_Force_ScalarCoreDatapath_1;
  uint64 Error_Timestamp_ScalarCoreDatapath_1;
  uint64 Error_Info_ScalarCoreDatapath_1;

  uint64 Error_ScalarCoreDatapath_2;
  uint64 Error_Mask_ScalarCoreDatapath_2;
  uint64 Error_Force_ScalarCoreDatapath_2;
  uint64 Error_Timestamp_ScalarCoreDatapath_2;
  uint64 Error_Info_ScalarCoreDatapath_2;

  uint64 Error_ScalarCoreDatapath_3;
  uint64 Error_Mask_ScalarCoreDatapath_3;
  uint64 Error_Force_ScalarCoreDatapath_3;
  uint64 Error_Timestamp_ScalarCoreDatapath_3;
  uint64 Error_Info_ScalarCoreDatapath_3;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_CONFIG_SCALAR_CORE_CSR_OFFSETS_H_
