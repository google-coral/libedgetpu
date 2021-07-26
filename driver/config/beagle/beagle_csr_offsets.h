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

// AUTO GENERATED FILE.


#ifndef DARWINN_DRIVER_CONFIG_BEAGLE_BEAGLE_CSR_OFFSETS_H_
#define DARWINN_DRIVER_CONFIG_BEAGLE_BEAGLE_CSR_OFFSETS_H_

#include "driver/config/apex_csr_offsets.h"
#include "driver/config/breakpoint_csr_offsets.h"
#include "driver/config/cb_bridge_csr_offsets.h"
#include "driver/config/debug_hib_user_csr_offsets.h"
#include "driver/config/debug_scalar_core_csr_offsets.h"
#include "driver/config/debug_tile_csr_offsets.h"
#include "driver/config/hib_kernel_csr_offsets.h"
#include "driver/config/hib_user_csr_offsets.h"
#include "driver/config/interrupt_csr_offsets.h"
#include "driver/config/memory_csr_offsets.h"
#include "driver/config/misc_csr_offsets.h"
#include "driver/config/msix_csr_offsets.h"
#include "driver/config/queue_csr_offsets.h"
#include "driver/config/register_constants.h"
#include "driver/config/register_file_csr_offsets.h"
#include "driver/config/scalar_core_csr_offsets.h"
#include "driver/config/scu_csr_offsets.h"
#include "driver/config/sync_flag_csr_offsets.h"
#include "driver/config/tile_config_csr_offsets.h"
#include "driver/config/tile_csr_offsets.h"
#include "driver/config/trace_csr_offsets.h"
#include "driver/config/usb_csr_offsets.h"
#include "driver/config/wire_csr_offsets.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

const InterruptCsrOffsets kBeagleFatalErrIntInterruptCsrOffsets = {
    0x486c0,  // NOLINT: fatal_err_int_control
    0x486c8,  // NOLINT: fatal_err_int_status
};

const InterruptCsrOffsets kBeagleScHostIntInterruptCsrOffsets = {
    0x486a0,  // NOLINT: sc_host_int_control
    0x486a8,  // NOLINT: sc_host_int_status
};

const InterruptCsrOffsets kBeagleTopLevelIntInterruptCsrOffsets = {
    0x486b0,  // NOLINT: top_level_int_control
    0x486b8,  // NOLINT: top_level_int_status
};

const BreakpointCsrOffsets kBeagleAvdatapopBreakpointCsrOffsets = {
    0x44158,  // NOLINT: avDataPopRunControl
    0x44168,  // NOLINT: avDataPopRunStatus
    0x44160,  // NOLINT: avDataPopBreakPoint
};

const BreakpointCsrOffsets kBeagleInfeedBreakpointCsrOffsets = {
    0x441d8,  // NOLINT: infeedRunControl
    0x441e0,  // NOLINT: infeedRunStatus
    0x441e8,  // NOLINT: infeedBreakPoint
};

const BreakpointCsrOffsets kBeagleOutfeedBreakpointCsrOffsets = {
    0x44218,  // NOLINT: outfeedRunControl
    0x44220,  // NOLINT: outfeedRunStatus
    0x44228,  // NOLINT: outfeedBreakPoint
};

const BreakpointCsrOffsets kBeagleParameterpopBreakpointCsrOffsets = {
    0x44198,  // NOLINT: parameterPopRunControl
    0x441a8,  // NOLINT: parameterPopRunStatus
    0x441a0,  // NOLINT: parameterPopBreakPoint
};

const BreakpointCsrOffsets kBeagleScalarcoreBreakpointCsrOffsets = {
    0x44018,  // NOLINT: scalarCoreRunControl
    0x44258,  // NOLINT: scalarCoreRunStatus
    0x44020,  // NOLINT: scalarCoreBreakPoint
};

const RegisterFileCsrOffsets kBeaglePredicateRegisterFileCsrOffsets = {
    0x44500,  // NOLINT: predicateRegisterFile
};

const RegisterFileCsrOffsets kBeagleScalarRegisterFileCsrOffsets = {
    0x44400,  // NOLINT: scalarRegisterFile
};

const SyncFlagCsrOffsets kBeagleAvdataInfeedSyncFlagCsrOffsets = {
    0x44060,  // NOLINT: SyncCounter_AVDATA_INFEED
};

const SyncFlagCsrOffsets kBeagleAvdataPopSyncFlagCsrOffsets = {
    0x44050,  // NOLINT: SyncCounter_AVDATA_POP
};

const SyncFlagCsrOffsets kBeagleParameterInfeedSyncFlagCsrOffsets = {
    0x44068,  // NOLINT: SyncCounter_PARAMETER_INFEED
};

const SyncFlagCsrOffsets kBeagleParameterPopSyncFlagCsrOffsets = {
    0x44058,  // NOLINT: SyncCounter_PARAMETER_POP
};

const SyncFlagCsrOffsets kBeagleProducerASyncFlagCsrOffsets = {
    0x44078,  // NOLINT: SyncCounter_PRODUCER_A
};

const SyncFlagCsrOffsets kBeagleProducerBSyncFlagCsrOffsets = {
    0x44080,  // NOLINT: SyncCounter_PRODUCER_B
};

const SyncFlagCsrOffsets kBeagleRingOutfeedSyncFlagCsrOffsets = {
    0x44088,  // NOLINT: SyncCounter_RING_OUTFEED
};

const SyncFlagCsrOffsets kBeagleScalarInfeedSyncFlagCsrOffsets = {
    0x44070,  // NOLINT: SyncCounter_SCALAR_INFEED
};

const SyncFlagCsrOffsets kBeagleScalarPipelineSyncFlagCsrOffsets = {
    0x44090,  // NOLINT: SyncCounter_SCALAR_PIPELINE
};

const TraceCsrOffsets kBeagleAvdatapopTraceCsrOffsets = {
    0x44170,                         // NOLINT: avDataPopOverwriteMode
    0x44178,                         // NOLINT: avDataPopEnableTracing
    0x442c0,                         // NOLINT: avDataPopTrace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPopTimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPopStallCauseSelect
};

const TraceCsrOffsets kBeagleInfeedTraceCsrOffsets = {
    0x441f0,                         // NOLINT: infeedOverwriteMode
    0x441f8,                         // NOLINT: infeedEnableTracing
    0x44340,                         // NOLINT: infeedTrace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeedTimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeedStallCauseSelect
};

const TraceCsrOffsets kBeagleIrqcompletionbufferTraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, irqCompletionBufferOverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, irqCompletionBufferEnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, irqCompletionBufferTrace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, irqCompletionBufferTimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // irqCompletionBufferStallCauseSelect
};

const TraceCsrOffsets kBeagleOutfeedTraceCsrOffsets = {
    0x44230,                         // NOLINT: outfeedOverwriteMode
    0x44238,                         // NOLINT: outfeedEnableTracing
    0x44380,                         // NOLINT: outfeedTrace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeedTimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeedStallCauseSelect
};

const TraceCsrOffsets kBeagleParameterpopTraceCsrOffsets = {
    0x441b0,                         // NOLINT: parameterPopOverwriteMode
    0x441b8,                         // NOLINT: parameterPopEnableTracing
    0x44300,                         // NOLINT: parameterPopTrace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPopTimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPopStallCauseSelect
};

const BreakpointCsrOffsets kBeagleMeshbus0BreakpointCsrOffsets = {
    0x42250,  // NOLINT: meshBus0RunControl
    0x42258,  // NOLINT: meshBus0RunStatus
    0x42260,  // NOLINT: meshBus0BreakPoint
};

const BreakpointCsrOffsets kBeagleMeshbus1BreakpointCsrOffsets = {
    0x42298,  // NOLINT: meshBus1RunControl
    0x422a0,  // NOLINT: meshBus1RunStatus
    0x422a8,  // NOLINT: meshBus1BreakPoint
};

const BreakpointCsrOffsets kBeagleMeshbus2BreakpointCsrOffsets = {
    0x422e0,  // NOLINT: meshBus2RunControl
    0x422e8,  // NOLINT: meshBus2RunStatus
    0x422f0,  // NOLINT: meshBus2BreakPoint
};

const BreakpointCsrOffsets kBeagleMeshbus3BreakpointCsrOffsets = {
    0x42328,  // NOLINT: meshBus3RunControl
    0x42330,  // NOLINT: meshBus3RunStatus
    0x42338,  // NOLINT: meshBus3BreakPoint
};

const BreakpointCsrOffsets kBeagleNarrowtowideBreakpointCsrOffsets = {
    0x42150,  // NOLINT: narrowToWideRunControl
    0x42158,  // NOLINT: narrowToWideRunStatus
    0x42160,  // NOLINT: narrowToWideBreakPoint
};

const BreakpointCsrOffsets kBeagleOpBreakpointCsrOffsets = {
    0x420c0,  // NOLINT: opRunControl
    0x420e0,  // NOLINT: opRunStatus
    0x420d0,  // NOLINT: opBreakPoint
};

const BreakpointCsrOffsets kBeagleRingbusconsumer0BreakpointCsrOffsets = {
    0x42190,  // NOLINT: ringBusConsumer0RunControl
    0x42198,  // NOLINT: ringBusConsumer0RunStatus
    0x421a0,  // NOLINT: ringBusConsumer0BreakPoint
};

const BreakpointCsrOffsets kBeagleRingbusconsumer1BreakpointCsrOffsets = {
    0x421d0,  // NOLINT: ringBusConsumer1RunControl
    0x421d8,  // NOLINT: ringBusConsumer1RunStatus
    0x421e0,  // NOLINT: ringBusConsumer1BreakPoint
};

const BreakpointCsrOffsets kBeagleRingbusproducerBreakpointCsrOffsets = {
    0x42210,  // NOLINT: ringBusProducerRunControl
    0x42218,  // NOLINT: ringBusProducerRunStatus
    0x42220,  // NOLINT: ringBusProducerBreakPoint
};

const BreakpointCsrOffsets kBeagleWidetonarrowBreakpointCsrOffsets = {
    0x42110,  // NOLINT: wideToNarrowRunControl
    0x42118,  // NOLINT: wideToNarrowRunStatus
    0x42120,  // NOLINT: wideToNarrowBreakPoint
};

const SyncFlagCsrOffsets kBeagleAvdataSyncFlagCsrOffsets = {
    0x42028,  // NOLINT: SyncCounter_AVDATA
};

const SyncFlagCsrOffsets kBeagleMeshEastInSyncFlagCsrOffsets = {
    0x42048,  // NOLINT: SyncCounter_MESH_EAST_IN
};

const SyncFlagCsrOffsets kBeagleMeshEastOutSyncFlagCsrOffsets = {
    0x42068,  // NOLINT: SyncCounter_MESH_EAST_OUT
};

const SyncFlagCsrOffsets kBeagleMeshNorthInSyncFlagCsrOffsets = {
    0x42040,  // NOLINT: SyncCounter_MESH_NORTH_IN
};

const SyncFlagCsrOffsets kBeagleMeshNorthOutSyncFlagCsrOffsets = {
    0x42060,  // NOLINT: SyncCounter_MESH_NORTH_OUT
};

const SyncFlagCsrOffsets kBeagleMeshSouthInSyncFlagCsrOffsets = {
    0x42050,  // NOLINT: SyncCounter_MESH_SOUTH_IN
};

const SyncFlagCsrOffsets kBeagleMeshSouthOutSyncFlagCsrOffsets = {
    0x42070,  // NOLINT: SyncCounter_MESH_SOUTH_OUT
};

const SyncFlagCsrOffsets kBeagleMeshWestInSyncFlagCsrOffsets = {
    0x42058,  // NOLINT: SyncCounter_MESH_WEST_IN
};

const SyncFlagCsrOffsets kBeagleMeshWestOutSyncFlagCsrOffsets = {
    0x42078,  // NOLINT: SyncCounter_MESH_WEST_OUT
};

const SyncFlagCsrOffsets kBeagleNarrowToWideSyncFlagCsrOffsets = {
    0x42090,  // NOLINT: SyncCounter_NARROW_TO_WIDE
};

const SyncFlagCsrOffsets kBeagleParametersSyncFlagCsrOffsets = {
    0x42030,  // NOLINT: SyncCounter_PARAMETERS
};

const SyncFlagCsrOffsets kBeaglePartialSumsSyncFlagCsrOffsets = {
    0x42038,  // NOLINT: SyncCounter_PARTIAL_SUMS
};

const SyncFlagCsrOffsets kBeagleRingProducerASyncFlagCsrOffsets = {
    0x420b0,  // NOLINT: SyncCounter_RING_PRODUCER_A
};

const SyncFlagCsrOffsets kBeagleRingProducerBSyncFlagCsrOffsets = {
    0x420b8,  // NOLINT: SyncCounter_RING_PRODUCER_B
};

const SyncFlagCsrOffsets kBeagleRingReadASyncFlagCsrOffsets = {
    0x42098,  // NOLINT: SyncCounter_RING_READ_A
};

const SyncFlagCsrOffsets kBeagleRingReadBSyncFlagCsrOffsets = {
    0x420a0,  // NOLINT: SyncCounter_RING_READ_B
};

const SyncFlagCsrOffsets kBeagleRingWriteSyncFlagCsrOffsets = {
    0x420a8,  // NOLINT: SyncCounter_RING_WRITE
};

const SyncFlagCsrOffsets kBeagleWideToNarrowSyncFlagCsrOffsets = {
    0x42080,  // NOLINT: SyncCounter_WIDE_TO_NARROW
};

const SyncFlagCsrOffsets kBeagleWideToScalingSyncFlagCsrOffsets = {
    0x42088,  // NOLINT: SyncCounter_WIDE_TO_SCALING
};

const TraceCsrOffsets kBeagleDmameshbus0TraceCsrOffsets = {
    0x42270,                         // NOLINT: dmaMeshBus0OverwriteMode
    0x42278,                         // NOLINT: dmaMeshBus0EnableTracing
    0x42740,                         // NOLINT: dmaMeshBus0Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaMeshBus0TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaMeshBus0StallCauseSelect
};

const TraceCsrOffsets kBeagleDmameshbus1TraceCsrOffsets = {
    0x422b8,                         // NOLINT: dmaMeshBus1OverwriteMode
    0x422c0,                         // NOLINT: dmaMeshBus1EnableTracing
    0x427c0,                         // NOLINT: dmaMeshBus1Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaMeshBus1TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaMeshBus1StallCauseSelect
};

const TraceCsrOffsets kBeagleDmameshbus2TraceCsrOffsets = {
    0x42300,                         // NOLINT: dmaMeshBus2OverwriteMode
    0x42308,                         // NOLINT: dmaMeshBus2EnableTracing
    0x42840,                         // NOLINT: dmaMeshBus2Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaMeshBus2TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaMeshBus2StallCauseSelect
};

const TraceCsrOffsets kBeagleDmameshbus3TraceCsrOffsets = {
    0x42348,                         // NOLINT: dmaMeshBus3OverwriteMode
    0x42350,                         // NOLINT: dmaMeshBus3EnableTracing
    0x428c0,                         // NOLINT: dmaMeshBus3Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaMeshBus3TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaMeshBus3StallCauseSelect
};

const TraceCsrOffsets kBeagleDmanarrowtonarrowTraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToNarrowOverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToNarrowEnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToNarrowTrace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToNarrowTimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaNarrowToNarrowStallCauseSelect
};

const TraceCsrOffsets kBeagleDmanarrowtowide0TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_0OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_0EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_0Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_0TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaNarrowToWide_0StallCauseSelect
};

const TraceCsrOffsets kBeagleDmanarrowtowide1TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_1OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_1EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_1Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_1TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaNarrowToWide_1StallCauseSelect
};

const TraceCsrOffsets kBeagleDmanarrowtowide2TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_2OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_2EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_2Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_2TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaNarrowToWide_2StallCauseSelect
};

const TraceCsrOffsets kBeagleDmanarrowtowide3TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_3OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_3EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_3Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_3TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaNarrowToWide_3StallCauseSelect
};

const TraceCsrOffsets kBeagleDmanarrowtowide4TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_4OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_4EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_4Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_4TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaNarrowToWide_4StallCauseSelect
};

const TraceCsrOffsets kBeagleDmanarrowtowide5TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_5OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_5EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_5Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_5TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaNarrowToWide_5StallCauseSelect
};

const TraceCsrOffsets kBeagleDmanarrowtowide6TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_6OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_6EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_6Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_6TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaNarrowToWide_6StallCauseSelect
};

const TraceCsrOffsets kBeagleDmanarrowtowide7TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_7OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_7EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_7Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWide_7TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaNarrowToWide_7StallCauseSelect
};

const TraceCsrOffsets kBeagleDmanarrowtowideTraceCsrOffsets = {
    0x42168,                         // NOLINT: dmaNarrowToWideOverwriteMode
    0x42170,                         // NOLINT: dmaNarrowToWideEnableTracing
    0x42600,                         // NOLINT: dmaNarrowToWideTrace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWideTimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWideStallCauseSelect
};

const TraceCsrOffsets kBeagleDmaringbusconsumer0TraceCsrOffsets = {
    0x421a8,                         // NOLINT: dmaRingBusConsumer0OverwriteMode
    0x421b0,                         // NOLINT: dmaRingBusConsumer0EnableTracing
    0x42640,                         // NOLINT: dmaRingBusConsumer0Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaRingBusConsumer0TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaRingBusConsumer0StallCauseSelect
};

const TraceCsrOffsets kBeagleDmaringbusconsumer1TraceCsrOffsets = {
    0x421e8,                         // NOLINT: dmaRingBusConsumer1OverwriteMode
    0x421f0,                         // NOLINT: dmaRingBusConsumer1EnableTracing
    0x42680,                         // NOLINT: dmaRingBusConsumer1Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaRingBusConsumer1TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaRingBusConsumer1StallCauseSelect
};

const TraceCsrOffsets kBeagleDmaringbusproducerTraceCsrOffsets = {
    0x42228,                         // NOLINT: dmaRingBusProducerOverwriteMode
    0x42230,                         // NOLINT: dmaRingBusProducerEnableTracing
    0x426c0,                         // NOLINT: dmaRingBusProducerTrace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaRingBusProducerTimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaRingBusProducerStallCauseSelect
};

const TraceCsrOffsets kBeagleDmawidetonarrow0TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_0OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_0EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_0Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_0TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaWideToNarrow_0StallCauseSelect
};

const TraceCsrOffsets kBeagleDmawidetonarrow1TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_1OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_1EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_1Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_1TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaWideToNarrow_1StallCauseSelect
};

const TraceCsrOffsets kBeagleDmawidetonarrow2TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_2OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_2EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_2Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_2TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaWideToNarrow_2StallCauseSelect
};

const TraceCsrOffsets kBeagleDmawidetonarrow3TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_3OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_3EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_3Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_3TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaWideToNarrow_3StallCauseSelect
};

const TraceCsrOffsets kBeagleDmawidetonarrow4TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_4OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_4EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_4Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_4TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaWideToNarrow_4StallCauseSelect
};

const TraceCsrOffsets kBeagleDmawidetonarrow5TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_5OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_5EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_5Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_5TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaWideToNarrow_5StallCauseSelect
};

const TraceCsrOffsets kBeagleDmawidetonarrow6TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_6OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_6EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_6Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_6TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaWideToNarrow_6StallCauseSelect
};

const TraceCsrOffsets kBeagleDmawidetonarrow7TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_7OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_7EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_7Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrow_7TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaWideToNarrow_7StallCauseSelect
};

const TraceCsrOffsets kBeagleDmawidetonarrowTraceCsrOffsets = {
    0x42128,                         // NOLINT: dmaWideToNarrowOverwriteMode
    0x42130,                         // NOLINT: dmaWideToNarrowEnableTracing
    0x42500,                         // NOLINT: dmaWideToNarrowTrace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrowTimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrowStallCauseSelect
};

const TraceCsrOffsets kBeagleOp0TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_0OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_0EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_0Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_0TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_0StallCauseSelect
};

const TraceCsrOffsets kBeagleOp1TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_1OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_1EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_1Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_1TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_1StallCauseSelect
};

const TraceCsrOffsets kBeagleOp2TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_2OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_2EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_2Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_2TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_2StallCauseSelect
};

const TraceCsrOffsets kBeagleOp3TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_3OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_3EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_3Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_3TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_3StallCauseSelect
};

const TraceCsrOffsets kBeagleOp4TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_4OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_4EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_4Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_4TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_4StallCauseSelect
};

const TraceCsrOffsets kBeagleOp5TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_5OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_5EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_5Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_5TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_5StallCauseSelect
};

const TraceCsrOffsets kBeagleOp6TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_6OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_6EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_6Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_6TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_6StallCauseSelect
};

const TraceCsrOffsets kBeagleOp7TraceCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_7OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_7EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_7Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_7TimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Op_7StallCauseSelect
};

const TraceCsrOffsets kBeagleOpTraceCsrOffsets = {
    0x420e8,                         // NOLINT: OpOverwriteMode
    0x420f0,                         // NOLINT: OpEnableTracing
    0x42400,                         // NOLINT: OpTrace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, OpTimeStampUnit
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, OpStallCauseSelect
};

const DebugHibUserCsrOffsets kBeagleDebugHibUserCsrOffsets = {
    0x48010,  // NOLINT: instruction_inbound_queue_total_occupancy
    0x48018,  // NOLINT: instruction_inbound_queue_threshold_counter
    0x48020,  // NOLINT: instruction_inbound_queue_insertion_counter
    0x48028,  // NOLINT: instruction_inbound_queue_full_counter
    0x48030,  // NOLINT: input_actv_inbound_queue_total_occupancy
    0x48038,  // NOLINT: input_actv_inbound_queue_threshold_counter
    0x48040,  // NOLINT: input_actv_inbound_queue_insertion_counter
    0x48048,  // NOLINT: input_actv_inbound_queue_full_counter
    0x48050,  // NOLINT: param_inbound_queue_total_occupancy
    0x48058,  // NOLINT: param_inbound_queue_threshold_counter
    0x48060,  // NOLINT: param_inbound_queue_insertion_counter
    0x48068,  // NOLINT: param_inbound_queue_full_counter
    0x48070,  // NOLINT: output_actv_inbound_queue_total_occupancy
    0x48078,  // NOLINT: output_actv_inbound_queue_threshold_counter
    0x48080,  // NOLINT: output_actv_inbound_queue_insertion_counter
    0x48088,  // NOLINT: output_actv_inbound_queue_full_counter
    0x48090,  // NOLINT: status_block_write_inbound_queue_total_occupancy
    0x48098,  // NOLINT: status_block_write_inbound_queue_threshold_counter
    0x480a0,  // NOLINT: status_block_write_inbound_queue_insertion_counter
    0x480a8,  // NOLINT: status_block_write_inbound_queue_full_counter
    0x480b0,  // NOLINT: queue_fetch_inbound_queue_total_occupancy
    0x480b8,  // NOLINT: queue_fetch_inbound_queue_threshold_counter
    0x480c0,  // NOLINT: queue_fetch_inbound_queue_insertion_counter
    0x480c8,  // NOLINT: queue_fetch_inbound_queue_full_counter
    0x480d0,  // NOLINT: instruction_outbound_queue_total_occupancy
    0x480d8,  // NOLINT: instruction_outbound_queue_threshold_counter
    0x480e0,  // NOLINT: instruction_outbound_queue_insertion_counter
    0x480e8,  // NOLINT: instruction_outbound_queue_full_counter
    0x480f0,  // NOLINT: input_actv_outbound_queue_total_occupancy
    0x480f8,  // NOLINT: input_actv_outbound_queue_threshold_counter
    0x48100,  // NOLINT: input_actv_outbound_queue_insertion_counter
    0x48108,  // NOLINT: input_actv_outbound_queue_full_counter
    0x48110,  // NOLINT: param_outbound_queue_total_occupancy
    0x48118,  // NOLINT: param_outbound_queue_threshold_counter
    0x48120,  // NOLINT: param_outbound_queue_insertion_counter
    0x48128,  // NOLINT: param_outbound_queue_full_counter
    0x48130,  // NOLINT: output_actv_outbound_queue_total_occupancy
    0x48138,  // NOLINT: output_actv_outbound_queue_threshold_counter
    0x48140,  // NOLINT: output_actv_outbound_queue_insertion_counter
    0x48148,  // NOLINT: output_actv_outbound_queue_full_counter
    0x48150,  // NOLINT: status_block_write_outbound_queue_total_occupancy
    0x48158,  // NOLINT: status_block_write_outbound_queue_threshold_counter
    0x48160,  // NOLINT: status_block_write_outbound_queue_insertion_counter
    0x48168,  // NOLINT: status_block_write_outbound_queue_full_counter
    0x48170,  // NOLINT: queue_fetch_outbound_queue_total_occupancy
    0x48178,  // NOLINT: queue_fetch_outbound_queue_threshold_counter
    0x48180,  // NOLINT: queue_fetch_outbound_queue_insertion_counter
    0x48188,  // NOLINT: queue_fetch_outbound_queue_full_counter
    0x48190,  // NOLINT: page_table_request_outbound_queue_total_occupancy
    0x48198,  // NOLINT: page_table_request_outbound_queue_threshold_counter
    0x481a0,  // NOLINT: page_table_request_outbound_queue_insertion_counter
    0x481a8,  // NOLINT: page_table_request_outbound_queue_full_counter
    0x481b0,  // NOLINT: read_tracking_fifo_total_occupancy
    0x481b8,  // NOLINT: read_tracking_fifo_threshold_counter
    0x481c0,  // NOLINT: read_tracking_fifo_insertion_counter
    0x481c8,  // NOLINT: read_tracking_fifo_full_counter
    0x481d0,  // NOLINT: write_tracking_fifo_total_occupancy
    0x481d8,  // NOLINT: write_tracking_fifo_threshold_counter
    0x481e0,  // NOLINT: write_tracking_fifo_insertion_counter
    0x481e8,  // NOLINT: write_tracking_fifo_full_counter
    0x481f0,  // NOLINT: read_buffer_total_occupancy
    0x481f8,  // NOLINT: read_buffer_threshold_counter
    0x48200,  // NOLINT: read_buffer_insertion_counter
    0x48208,  // NOLINT: read_buffer_full_counter
    0x48210,  // NOLINT: axi_aw_credit_shim_total_occupancy
    0x48218,  // NOLINT: axi_aw_credit_shim_threshold_counter
    0x48220,  // NOLINT: axi_aw_credit_shim_insertion_counter
    0x48228,  // NOLINT: axi_aw_credit_shim_full_counter
    0x48230,  // NOLINT: axi_ar_credit_shim_total_occupancy
    0x48238,  // NOLINT: axi_ar_credit_shim_threshold_counter
    0x48240,  // NOLINT: axi_ar_credit_shim_insertion_counter
    0x48248,  // NOLINT: axi_ar_credit_shim_full_counter
    0x48250,  // NOLINT: axi_w_credit_shim_total_occupancy
    0x48258,  // NOLINT: axi_w_credit_shim_threshold_counter
    0x48260,  // NOLINT: axi_w_credit_shim_insertion_counter
    0x48268,  // NOLINT: axi_w_credit_shim_full_counter
    0x48270,  // NOLINT: instruction_inbound_queue_empty_cycles_count
    0x48278,  // NOLINT: input_actv_inbound_queue_empty_cycles_count
    0x48280,  // NOLINT: param_inbound_queue_empty_cycles_count
    0x48288,  // NOLINT: output_actv_inbound_queue_empty_cycles_count
    0x48290,  // NOLINT: status_block_write_inbound_queue_empty_cycles_count
    0x48298,  // NOLINT: queue_fetch_inbound_queue_empty_cycles_count
    0x482a0,  // NOLINT: instruction_outbound_queue_empty_cycles_count
    0x482a8,  // NOLINT: input_actv_outbound_queue_empty_cycles_count
    0x482b0,  // NOLINT: param_outbound_queue_empty_cycles_count
    0x482b8,  // NOLINT: output_actv_outbound_queue_empty_cycles_count
    0x482c0,  // NOLINT: status_block_write_outbound_queue_empty_cycles_count
    0x482c8,  // NOLINT: queue_fetch_outbound_queue_empty_cycles_count
    0x482d0,  // NOLINT: page_table_request_outbound_queue_empty_cycles_count
    0x482d8,  // NOLINT: read_tracking_fifo_empty_cycles_count
    0x482e0,  // NOLINT: write_tracking_fifo_empty_cycles_count
    0x482e8,  // NOLINT: read_buffer_empty_cycles_count
    0x482f0,  // NOLINT: read_request_arbiter_instruction_request_cycles
    0x482f8,  // NOLINT: read_request_arbiter_instruction_blocked_cycles
    0x48300,  // NOLINT:
              // read_request_arbiter_instruction_blocked_by_arbitration_cycles
    0x48308,  // NOLINT:
              // read_request_arbiter_instruction_cycles_blocked_over_threshold
    0x48310,  // NOLINT: read_request_arbiter_input_actv_request_cycles
    0x48318,  // NOLINT: read_request_arbiter_input_actv_blocked_cycles
    0x48320,  // NOLINT:
              // read_request_arbiter_input_actv_blocked_by_arbitration_cycles
    0x48328,  // NOLINT:
              // read_request_arbiter_input_actv_cycles_blocked_over_threshold
    0x48330,  // NOLINT: read_request_arbiter_param_request_cycles
    0x48338,  // NOLINT: read_request_arbiter_param_blocked_cycles
    0x48340,  // NOLINT:
              // read_request_arbiter_param_blocked_by_arbitration_cycles
    0x48348,  // NOLINT:
              // read_request_arbiter_param_cycles_blocked_over_threshold
    0x48350,  // NOLINT: read_request_arbiter_queue_fetch_request_cycles
    0x48358,  // NOLINT: read_request_arbiter_queue_fetch_blocked_cycles
    0x48360,  // NOLINT:
              // read_request_arbiter_queue_fetch_blocked_by_arbitration_cycles
    0x48368,  // NOLINT:
              // read_request_arbiter_queue_fetch_cycles_blocked_over_threshold
    0x48370,  // NOLINT: read_request_arbiter_page_table_request_request_cycles
    0x48378,  // NOLINT: read_request_arbiter_page_table_request_blocked_cycles
    0x48380,  // NOLINT:
              // read_request_arbiter_page_table_request_blocked_by_arbitration_cycles
    0x48388,  // NOLINT:
              // read_request_arbiter_page_table_request_cycles_blocked_over_threshold
    0x48390,  // NOLINT: write_request_arbiter_output_actv_request_cycles
    0x48398,  // NOLINT: write_request_arbiter_output_actv_blocked_cycles
    0x483a0,  // NOLINT:
              // write_request_arbiter_output_actv_blocked_by_arbitration_cycles
    0x483a8,  // NOLINT:
              // write_request_arbiter_output_actv_cycles_blocked_over_threshold
    0x483b0,  // NOLINT: write_request_arbiter_status_block_write_request_cycles
    0x483b8,  // NOLINT: write_request_arbiter_status_block_write_blocked_cycles
    0x483c0,  // NOLINT:
              // write_request_arbiter_status_block_write_blocked_by_arbitration_cycles
    0x483c8,  // NOLINT:
              // write_request_arbiter_status_block_write_cycles_blocked_over_threshold
    0x483d0,  // NOLINT: address_translation_arbiter_instruction_request_cycles
    0x483d8,  // NOLINT: address_translation_arbiter_instruction_blocked_cycles
    0x483e0,  // NOLINT:
              // address_translation_arbiter_instruction_blocked_by_arbitration_cycles
    0x483e8,  // NOLINT:
              // address_translation_arbiter_instruction_cycles_blocked_over_threshold
    0x483f0,  // NOLINT: address_translation_arbiter_input_actv_request_cycles
    0x483f8,  // NOLINT: address_translation_arbiter_input_actv_blocked_cycles
    0x48400,  // NOLINT:
              // address_translation_arbiter_input_actv_blocked_by_arbitration_cycles
    0x48408,  // NOLINT:
              // address_translation_arbiter_input_actv_cycles_blocked_over_threshold
    0x48410,  // NOLINT: address_translation_arbiter_param_request_cycles
    0x48418,  // NOLINT: address_translation_arbiter_param_blocked_cycles
    0x48420,  // NOLINT:
              // address_translation_arbiter_param_blocked_by_arbitration_cycles
    0x48428,  // NOLINT:
              // address_translation_arbiter_param_cycles_blocked_over_threshold
    0x48430,  // NOLINT:
              // address_translation_arbiter_status_block_write_request_cycles
    0x48438,  // NOLINT:
              // address_translation_arbiter_status_block_write_blocked_cycles
    0x48440,  // NOLINT:
              // address_translation_arbiter_status_block_write_blocked_by_arbitration_cycles
    0x48448,  // NOLINT:
              // address_translation_arbiter_status_block_write_cycles_blocked_over_threshold
    0x48450,  // NOLINT: address_translation_arbiter_output_actv_request_cycles
    0x48458,  // NOLINT: address_translation_arbiter_output_actv_blocked_cycles
    0x48460,  // NOLINT:
              // address_translation_arbiter_output_actv_blocked_by_arbitration_cycles
    0x48468,  // NOLINT:
              // address_translation_arbiter_output_actv_cycles_blocked_over_threshold
    0x48470,  // NOLINT: address_translation_arbiter_queue_fetch_request_cycles
    0x48478,  // NOLINT: address_translation_arbiter_queue_fetch_blocked_cycles
    0x48480,  // NOLINT:
              // address_translation_arbiter_queue_fetch_blocked_by_arbitration_cycles
    0x48488,  // NOLINT:
              // address_translation_arbiter_queue_fetch_cycles_blocked_over_threshold
    0x48490,  // NOLINT: issued_interrupt_count
    0x48498,  // NOLINT: data_read_16byte_count
    0x484a0,  // NOLINT: waiting_for_tag_cycles
    0x484a8,  // NOLINT: waiting_for_axi_cycles
    0x484b0,  // NOLINT: simple_translations
    0x484c8,  // NOLINT: instruction_credits_per_cycle_sum
    0x484d0,  // NOLINT: input_actv_credits_per_cycle_sum
    0x484d8,  // NOLINT: param_credits_per_cycle_sum
    0x484e0,  // NOLINT: output_actv_credits_per_cycle_sum
    0x484e8,  // NOLINT: status_block_write_credits_per_cycle_sum
    0x484f0,  // NOLINT: queue_fetch_credits_per_cycle_sum
    0x484f8,  // NOLINT: page_table_request_credits_per_cycle_sum
    0x48500,  // NOLINT: output_actv_queue_control
    0x48508,  // NOLINT: output_actv_queue_status
    0x48510,  // NOLINT: output_actv_queue_descriptor_size
    0x48518,  // NOLINT: output_actv_queue_minimum_size
    0x48520,  // NOLINT: output_actv_queue_maximum_size
    0x48528,  // NOLINT: output_actv_queue_base
    0x48530,  // NOLINT: output_actv_queue_status_block_base
    0x48538,  // NOLINT: output_actv_queue_size
    0x48540,  // NOLINT: output_actv_queue_tail
    0x48548,  // NOLINT: output_actv_queue_fetched_head
    0x48550,  // NOLINT: output_actv_queue_completed_head
    0x48558,  // NOLINT: output_actv_queue_int_control
    0x48560,  // NOLINT: output_actv_queue_int_status
    0x48568,  // NOLINT: instruction_queue_control
    0x48570,  // NOLINT: instruction_queue_status
    0x48578,  // NOLINT: instruction_queue_descriptor_size
    0x48580,  // NOLINT: instruction_queue_minimum_size
    0x48588,  // NOLINT: instruction_queue_maximum_size
    0x48590,  // NOLINT: instruction_queue_base
    0x48598,  // NOLINT: instruction_queue_status_block_base
    0x485a0,  // NOLINT: instruction_queue_size
    0x485a8,  // NOLINT: instruction_queue_tail
    0x485b0,  // NOLINT: instruction_queue_fetched_head
    0x485b8,  // NOLINT: instruction_queue_completed_head
    0x485c0,  // NOLINT: instruction_queue_int_control
    0x485c8,  // NOLINT: instruction_queue_int_status
    0x485d0,  // NOLINT: input_actv_queue_control
    0x485d8,  // NOLINT: input_actv_queue_status
    0x485e0,  // NOLINT: input_actv_queue_descriptor_size
    0x485e8,  // NOLINT: input_actv_queue_minimum_size
    0x485f0,  // NOLINT: input_actv_queue_maximum_size
    0x485f8,  // NOLINT: input_actv_queue_base
    0x48600,  // NOLINT: input_actv_queue_status_block_base
    0x48608,  // NOLINT: input_actv_queue_size
    0x48610,  // NOLINT: input_actv_queue_tail
    0x48618,  // NOLINT: input_actv_queue_fetched_head
    0x48620,  // NOLINT: input_actv_queue_completed_head
    0x48628,  // NOLINT: input_actv_queue_int_control
    0x48630,  // NOLINT: input_actv_queue_int_status
    0x48638,  // NOLINT: param_queue_control
    0x48640,  // NOLINT: param_queue_status
    0x48648,  // NOLINT: param_queue_descriptor_size
    0x48650,  // NOLINT: param_queue_minimum_size
    0x48658,  // NOLINT: param_queue_maximum_size
    0x48660,  // NOLINT: param_queue_base
    0x48668,  // NOLINT: param_queue_status_block_base
    0x48670,  // NOLINT: param_queue_size
    0x48678,  // NOLINT: param_queue_tail
    0x48680,  // NOLINT: param_queue_fetched_head
    0x48688,  // NOLINT: param_queue_completed_head
    0x48690,  // NOLINT: param_queue_int_control
    0x48698,  // NOLINT: param_queue_int_status
    0x486a0,  // NOLINT: sc_host_int_control
    0x486a8,  // NOLINT: sc_host_int_status
    0x486b0,  // NOLINT: top_level_int_control
    0x486b8,  // NOLINT: top_level_int_status
    0x486c0,  // NOLINT: fatal_err_int_control
    0x486c8,  // NOLINT: fatal_err_int_status
    0x486d0,  // NOLINT: sc_host_int_count
    0x486d8,  // NOLINT: dma_pause
    0x486e0,  // NOLINT: dma_paused
    0x486e8,  // NOLINT: status_block_update
    0x486f0,  // NOLINT: hib_error_status
    0x486f8,  // NOLINT: hib_error_mask
    0x48700,  // NOLINT: hib_first_error_status
    0x48708,  // NOLINT: hib_first_error_timestamp
    0x48710,  // NOLINT: hib_inject_error
    0x48718,  // NOLINT: read_request_arbiter
    0x48720,  // NOLINT: write_request_arbiter
    0x48728,  // NOLINT: address_translation_arbiter
    0x48730,  // NOLINT: sender_queue_threshold
    0x48738,  // NOLINT: page_fault_address
    0x48740,  // NOLINT: instruction_credits
    0x48748,  // NOLINT: input_actv_credits
    0x48750,  // NOLINT: param_credits
    0x48758,  // NOLINT: output_actv_credits
    0x48760,  // NOLINT: pause_state
    0x48768,  // NOLINT: snapshot
    0x48770,  // NOLINT: idle_assert
    0x48778,  // NOLINT: wire_int_pending_bit_array
    0x48788,  // NOLINT: tileconfig0
    0x48790,  // NOLINT: tileconfig1
};

const DebugScalarCoreCsrOffsets kBeagleDebugScalarCoreCsrOffsets = {
    0x44000,                         // NOLINT: topology
    0x44008,                         // NOLINT: scMemoryCapacity
    0x44010,                         // NOLINT: tileMemoryCapacity
    0x44040,                         // NOLINT: scMemoryAccess
    0x44048,                         // NOLINT: scMemoryData
    0x44288,                         // NOLINT: Timeout
    0x44260,                         // NOLINT: Error_ScalarCore
    0x44268,                         // NOLINT: Error_Mask_ScalarCore
    0x44270,                         // NOLINT: Error_Force_ScalarCore
    0x44278,                         // NOLINT: Error_Timestamp_ScalarCore
    0x44280,                         // NOLINT: Error_Info_ScalarCore
    0x44018,                         // NOLINT: scalarCoreRunControl
    0x44020,                         // NOLINT: scalarCoreBreakPoint
    0x44028,                         // NOLINT: currentPc
    0x44038,                         // NOLINT: executeControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarDatapath_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarDatapath_0BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, currentPc_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, executeControl_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarDatapath_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarDatapath_1BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, currentPc_1
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, executeControl_1
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarDatapath_2RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarDatapath_2BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, currentPc_2
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, executeControl_2
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarDatapath_3RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarDatapath_3BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, currentPc_3
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, executeControl_3
    0x44050,                         // NOLINT: SyncCounter_AVDATA_POP
    0x44058,                         // NOLINT: SyncCounter_PARAMETER_POP
    0x44060,                         // NOLINT: SyncCounter_AVDATA_INFEED
    0x44068,                         // NOLINT: SyncCounter_PARAMETER_INFEED
    0x44070,                         // NOLINT: SyncCounter_SCALAR_INFEED
    0x44078,                         // NOLINT: SyncCounter_PRODUCER_A
    0x44080,                         // NOLINT: SyncCounter_PRODUCER_B
    0x44088,                         // NOLINT: SyncCounter_RING_OUTFEED
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_AVDATA_POP_0_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_PARAMETER_POP_0_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_AVDATA_INFEED_0_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_PARAMETER_INFEED_0_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_SCALAR_INFEED_0_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_PRODUCER_A_0_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_PRODUCER_B_0_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_RING_OUTFEED_0_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_AVDATA_POP_1_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_PARAMETER_POP_1_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_AVDATA_INFEED_1_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_PARAMETER_INFEED_1_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_SCALAR_INFEED_1_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_PRODUCER_A_1_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_PRODUCER_B_1_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_RING_OUTFEED_1_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_AVDATA_POP_2_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_PARAMETER_POP_2_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_AVDATA_INFEED_2_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_PARAMETER_INFEED_2_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_SCALAR_INFEED_2_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_PRODUCER_A_2_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_PRODUCER_B_2_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_RING_OUTFEED_2_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_AVDATA_POP_3_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_PARAMETER_POP_3_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_AVDATA_INFEED_3_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_PARAMETER_INFEED_3_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_SCALAR_INFEED_3_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_PRODUCER_A_3_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_PRODUCER_B_3_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, SyncCounter_RING_OUTFEED_3_0
    0x44158,                         // NOLINT: avDataPopRunControl
    0x44160,                         // NOLINT: avDataPopBreakPoint
    0x44168,                         // NOLINT: avDataPopRunStatus
    0x44170,                         // NOLINT: avDataPopOverwriteMode
    0x44178,                         // NOLINT: avDataPopEnableTracing
    0x44180,                         // NOLINT: avDataPopStartCycle
    0x44188,                         // NOLINT: avDataPopEndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPopStallCycleCount
    0x44190,                         // NOLINT: avDataPopProgramCounter
    0x442a0,                         // NOLINT: avDataPopTtuStateRegFile
    0x442c0,                         // NOLINT: avDataPopTrace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_0BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_0OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_0EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_0StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_0EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_0StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_0ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_0TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_0Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_1BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_1RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_1OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_1EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_1StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_1EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_1StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_1ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_1TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_1Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_2RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_2BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_2RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_2OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_2EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_2StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_2EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_2StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_2ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_2TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_2Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_3RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_3BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_3RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_3OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_3EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_3StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_3EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_3StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_3ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_3TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_3Trace
    0x44198,                         // NOLINT: parameterPopRunControl
    0x441a0,                         // NOLINT: parameterPopBreakPoint
    0x441a8,                         // NOLINT: parameterPopRunStatus
    0x441b0,                         // NOLINT: parameterPopOverwriteMode
    0x441b8,                         // NOLINT: parameterPopEnableTracing
    0x441c0,                         // NOLINT: parameterPopStartCycle
    0x441c8,                         // NOLINT: parameterPopEndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPopStallCycleCount
    0x441d0,                         // NOLINT: parameterPopProgramCounter
    0x442e0,                         // NOLINT: parameterPopTtuStateRegFile
    0x44300,                         // NOLINT: parameterPopTrace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_0BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_0OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_0EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_0StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_0EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_0StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_0ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_0TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_0Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_1BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_1RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_1OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_1EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_1StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_1EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_1StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_1ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_1TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_1Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_2RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_2BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_2RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_2OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_2EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_2StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_2EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_2StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_2ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_2TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_2Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_3RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_3BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_3RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_3OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_3EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_3StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_3EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_3StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_3ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_3TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_3Trace
    0x441d8,                         // NOLINT: infeedRunControl
    0x441e0,                         // NOLINT: infeedRunStatus
    0x441e8,                         // NOLINT: infeedBreakPoint
    0x441f0,                         // NOLINT: infeedOverwriteMode
    0x441f8,                         // NOLINT: infeedEnableTracing
    0x44200,                         // NOLINT: infeedStartCycle
    0x44208,                         // NOLINT: infeedEndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeedStallCycleCount
    0x44210,                         // NOLINT: infeedProgramCounter
    0x44320,                         // NOLINT: infeedTtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_0BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_0OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_0EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_0StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_0EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_0StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_0ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_0TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_1RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_1BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_1OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_1EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_1StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_1EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_1StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_1ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_1TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_0BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_0OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_0EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_0StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_0EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_0StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_0ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_0TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_1RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_1BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_1OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_1EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_1StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_1EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_1StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_1ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_1TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_0BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_0OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_0EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_0StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_0EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_0StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_0ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_0TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_1RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_1BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_1OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_1EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_1StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_1EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_1StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_1ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_1TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_0BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_0OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_0EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_0StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_0EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_0StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_0ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_0TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_1RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_1BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_1OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_1EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_1StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_1EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_1StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_1ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_1TtuStateRegFile
    0x44218,                         // NOLINT: outfeedRunControl
    0x44220,                         // NOLINT: outfeedRunStatus
    0x44228,                         // NOLINT: outfeedBreakPoint
    0x44230,                         // NOLINT: outfeedOverwriteMode
    0x44238,                         // NOLINT: outfeedEnableTracing
    0x44240,                         // NOLINT: outfeedStartCycle
    0x44248,                         // NOLINT: outfeedEndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeedStallCycleCount
    0x44250,                         // NOLINT: outfeedProgramCounter
    0x44360,                         // NOLINT: outfeedTtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_0BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_0OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_0EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_0StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_0EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_0StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_0ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_0TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_1RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_1BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_1OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_1EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_1StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_1EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_1StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_1ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_1TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_0BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_0OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_0EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_0StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_0EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_0StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_0ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_0TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_1RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_1BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_1OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_1EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_1StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_1EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_1StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_1ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_1TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_0BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_0OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_0EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_0StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_0EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_0StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_0ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_0TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_1RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_1BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_1OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_1EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_1StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_1EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_1StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_1ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_1TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_0BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_0OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_0EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_0StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_0EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_0StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_0ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_0TtuStateRegFile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_1RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_1BreakPoint
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_1OverwriteMode
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_1EnableTracing
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_1StartCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_1EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_1StallCycleCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_1ProgramCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_1TtuStateRegFile
    0x44258,                         // NOLINT: scalarCoreRunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarCoreRunStatus_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarCoreRunStatus_1
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarCoreRunStatus_2
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarCoreRunStatus_3
};

const DebugTileCsrOffsets kBeagleDebugTileCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, TileClockControl
    0x42000,                         // NOLINT: tileid
    0x42008,                         // NOLINT: scratchpad
    0x42010,                         // NOLINT: memoryAccess
    0x42018,                         // NOLINT: memoryData
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowMemoryContext_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowMemoryContext_1
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowMemoryContext_2
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowMemoryContext_3
    0x42020,                         // NOLINT: deepSleep
    0x42028,                         // NOLINT: SyncCounter_AVDATA
    0x42030,                         // NOLINT: SyncCounter_PARAMETERS
    0x42038,                         // NOLINT: SyncCounter_PARTIAL_SUMS
    0x42040,                         // NOLINT: SyncCounter_MESH_NORTH_IN
    0x42048,                         // NOLINT: SyncCounter_MESH_EAST_IN
    0x42050,                         // NOLINT: SyncCounter_MESH_SOUTH_IN
    0x42058,                         // NOLINT: SyncCounter_MESH_WEST_IN
    0x42060,                         // NOLINT: SyncCounter_MESH_NORTH_OUT
    0x42068,                         // NOLINT: SyncCounter_MESH_EAST_OUT
    0x42070,                         // NOLINT: SyncCounter_MESH_SOUTH_OUT
    0x42078,                         // NOLINT: SyncCounter_MESH_WEST_OUT
    0x42080,                         // NOLINT: SyncCounter_WIDE_TO_NARROW
    0x42088,                         // NOLINT: SyncCounter_WIDE_TO_SCALING
    0x42090,                         // NOLINT: SyncCounter_NARROW_TO_WIDE
    0x42098,                         // NOLINT: SyncCounter_RING_READ_A
    0x420a0,                         // NOLINT: SyncCounter_RING_READ_B
    0x420a8,                         // NOLINT: SyncCounter_RING_WRITE
    0x420b0,                         // NOLINT: SyncCounter_RING_PRODUCER_A
    0x420b8,                         // NOLINT: SyncCounter_RING_PRODUCER_B
    0x420c0,                         // NOLINT: opRunControl
    0x420c8,                         // NOLINT: PowerSaveData
    0x420d0,                         // NOLINT: opBreakPoint
    0x420d8,                         // NOLINT: StallCounter
    0x420e0,                         // NOLINT: opRunStatus
    0x420e8,                         // NOLINT: OpOverwriteMode
    0x420f0,                         // NOLINT: OpEnableTracing
    0x420f8,                         // NOLINT: OpStartCycle
    0x42100,                         // NOLINT: OpEndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, OpStallCycleCount
    0x42108,                         // NOLINT: OpProgramCounter
    0x42110,                         // NOLINT: wideToNarrowRunControl
    0x42118,                         // NOLINT: wideToNarrowRunStatus
    0x42120,                         // NOLINT: wideToNarrowBreakPoint
    0x42128,                         // NOLINT: dmaWideToNarrowOverwriteMode
    0x42130,                         // NOLINT: dmaWideToNarrowEnableTracing
    0x42138,                         // NOLINT: dmaWideToNarrowStartCycle
    0x42140,                         // NOLINT: dmaWideToNarrowEndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaWideToNarrowStallCycleCount
    0x42148,                         // NOLINT: dmaWideToNarrowProgramCounter
    0x42150,                         // NOLINT: narrowToWideRunControl
    0x42158,                         // NOLINT: narrowToWideRunStatus
    0x42160,                         // NOLINT: narrowToWideBreakPoint
    0x42168,                         // NOLINT: dmaNarrowToWideOverwriteMode
    0x42170,                         // NOLINT: dmaNarrowToWideEnableTracing
    0x42178,                         // NOLINT: dmaNarrowToWideStartCycle
    0x42180,                         // NOLINT: dmaNarrowToWideEndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaNarrowToWideStallCycleCount
    0x42188,                         // NOLINT: dmaNarrowToWideProgramCounter
    0x42190,                         // NOLINT: ringBusConsumer0RunControl
    0x42198,                         // NOLINT: ringBusConsumer0RunStatus
    0x421a0,                         // NOLINT: ringBusConsumer0BreakPoint
    0x421a8,                         // NOLINT: dmaRingBusConsumer0OverwriteMode
    0x421b0,                         // NOLINT: dmaRingBusConsumer0EnableTracing
    0x421b8,                         // NOLINT: dmaRingBusConsumer0StartCycle
    0x421c0,                         // NOLINT: dmaRingBusConsumer0EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaRingBusConsumer0StallCycleCount
    0x421c8,  // NOLINT: dmaRingBusConsumer0ProgramCounter
    0x421d0,  // NOLINT: ringBusConsumer1RunControl
    0x421d8,  // NOLINT: ringBusConsumer1RunStatus
    0x421e0,  // NOLINT: ringBusConsumer1BreakPoint
    0x421e8,  // NOLINT: dmaRingBusConsumer1OverwriteMode
    0x421f0,  // NOLINT: dmaRingBusConsumer1EnableTracing
    0x421f8,  // NOLINT: dmaRingBusConsumer1StartCycle
    0x42200,  // NOLINT: dmaRingBusConsumer1EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaRingBusConsumer1StallCycleCount
    0x42208,  // NOLINT: dmaRingBusConsumer1ProgramCounter
    0x42210,  // NOLINT: ringBusProducerRunControl
    0x42218,  // NOLINT: ringBusProducerRunStatus
    0x42220,  // NOLINT: ringBusProducerBreakPoint
    0x42228,  // NOLINT: dmaRingBusProducerOverwriteMode
    0x42230,  // NOLINT: dmaRingBusProducerEnableTracing
    0x42238,  // NOLINT: dmaRingBusProducerStartCycle
    0x42240,  // NOLINT: dmaRingBusProducerEndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // dmaRingBusProducerStallCycleCount
    0x42248,                         // NOLINT: dmaRingBusProducerProgramCounter
    0x42250,                         // NOLINT: meshBus0RunControl
    0x42258,                         // NOLINT: meshBus0RunStatus
    0x42260,                         // NOLINT: meshBus0BreakPoint
    0x42270,                         // NOLINT: dmaMeshBus0OverwriteMode
    0x42278,                         // NOLINT: dmaMeshBus0EnableTracing
    0x42280,                         // NOLINT: dmaMeshBus0StartCycle
    0x42288,                         // NOLINT: dmaMeshBus0EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaMeshBus0StallCycleCount
    0x42290,                         // NOLINT: dmaMeshBus0ProgramCounter
    0x42298,                         // NOLINT: meshBus1RunControl
    0x422a0,                         // NOLINT: meshBus1RunStatus
    0x422a8,                         // NOLINT: meshBus1BreakPoint
    0x422b8,                         // NOLINT: dmaMeshBus1OverwriteMode
    0x422c0,                         // NOLINT: dmaMeshBus1EnableTracing
    0x422c8,                         // NOLINT: dmaMeshBus1StartCycle
    0x422d0,                         // NOLINT: dmaMeshBus1EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaMeshBus1StallCycleCount
    0x422d8,                         // NOLINT: dmaMeshBus1ProgramCounter
    0x422e0,                         // NOLINT: meshBus2RunControl
    0x422e8,                         // NOLINT: meshBus2RunStatus
    0x422f0,                         // NOLINT: meshBus2BreakPoint
    0x42300,                         // NOLINT: dmaMeshBus2OverwriteMode
    0x42308,                         // NOLINT: dmaMeshBus2EnableTracing
    0x42310,                         // NOLINT: dmaMeshBus2StartCycle
    0x42318,                         // NOLINT: dmaMeshBus2EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaMeshBus2StallCycleCount
    0x42320,                         // NOLINT: dmaMeshBus2ProgramCounter
    0x42328,                         // NOLINT: meshBus3RunControl
    0x42330,                         // NOLINT: meshBus3RunStatus
    0x42338,                         // NOLINT: meshBus3BreakPoint
    0x42348,                         // NOLINT: dmaMeshBus3OverwriteMode
    0x42350,                         // NOLINT: dmaMeshBus3EnableTracing
    0x42358,                         // NOLINT: dmaMeshBus3StartCycle
    0x42360,                         // NOLINT: dmaMeshBus3EndCycle
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dmaMeshBus3StallCycleCount
    0x42368,                         // NOLINT: dmaMeshBus3ProgramCounter
    0x42370,                         // NOLINT: Error_Tile
    0x42378,                         // NOLINT: Error_Mask_Tile
    0x42380,                         // NOLINT: Error_Force_Tile
    0x42388,                         // NOLINT: Error_Timestamp_Tile
    0x42390,                         // NOLINT: Error_Info_Tile
    0x42398,                         // NOLINT: Timeout
    0x423c0,                         // NOLINT: opTtuStateRegFile
    0x42400,                         // NOLINT: OpTrace
    0x42480,                         // NOLINT: wideToNarrowTtuStateRegFile
    0x42500,                         // NOLINT: dmaWideToNarrowTrace
    0x42580,                         // NOLINT: narrowToWideTtuStateRegFile
    0x42600,                         // NOLINT: dmaNarrowToWideTrace
    0x42620,                         // NOLINT: ringBusConsumer0TtuStateRegFile
    0x42640,                         // NOLINT: dmaRingBusConsumer0Trace
    0x42660,                         // NOLINT: ringBusConsumer1TtuStateRegFile
    0x42680,                         // NOLINT: dmaRingBusConsumer1Trace
    0x426a0,                         // NOLINT: ringBusProducerTtuStateRegFile
    0x426c0,                         // NOLINT: dmaRingBusProducerTrace
    0x42700,                         // NOLINT: meshBus0TtuStateRegFile
    0x42740,                         // NOLINT: dmaMeshBus0Trace
    0x42780,                         // NOLINT: meshBus1TtuStateRegFile
    0x427c0,                         // NOLINT: dmaMeshBus1Trace
    0x42800,                         // NOLINT: meshBus2TtuStateRegFile
    0x42840,                         // NOLINT: dmaMeshBus2Trace
    0x42880,                         // NOLINT: meshBus3TtuStateRegFile
    0x428c0,                         // NOLINT: dmaMeshBus3Trace
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowMemoryIsolation
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowMemoryRetention
};

const HibKernelCsrOffsets kBeagleHibKernelCsrOffsets = {
    0x46000,                         // NOLINT: page_table_size
    0x46008,                         // NOLINT: extended_table
    0x46050,                         // NOLINT: dma_pause
    0x46078,                         // NOLINT: page_table_init
    0x46080,                         // NOLINT: msix_table_init
    0x50000,                         // NOLINT: page_table
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, dma_burst_limiter
};

const HibUserCsrOffsets kBeagleHibUserCsrOffsets = {
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, axi_auser_scid
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, axi_auser_stream_id_output_actv
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, axi_auser_stream_id_instruction
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, axi_auser_stream_id_input_actv
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, axi_auser_stream_id_param
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, axi_aruser_overrides
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, axi_awuser_overrides
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, axi_overrides
    0x486b0,                         // NOLINT: top_level_int_control
    0x486b8,                         // NOLINT: top_level_int_status
    0x486d0,                         // NOLINT: sc_host_int_count
    0x486d8,                         // NOLINT: dma_pause
    0x486e0,                         // NOLINT: dma_paused
    0x486e8,                         // NOLINT: status_block_update
    0x486f0,                         // NOLINT: hib_error_status
    0x486f8,                         // NOLINT: hib_error_mask
    0x48700,                         // NOLINT: hib_first_error_status
    0x48708,                         // NOLINT: hib_first_error_timestamp
    0x48710,                         // NOLINT: hib_inject_error
    0x487a8,                         // NOLINT: dma_burst_limiter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, context_clock_gate_control
};

const QueueCsrOffsets kBeagleInstructionQueueCsrOffsets = {
    0x48568,  // NOLINT: instruction_queue_control
    0x48570,  // NOLINT: instruction_queue_status
    0x48578,  // NOLINT: instruction_queue_descriptor_size
    0x48590,  // NOLINT: instruction_queue_base
    0x48598,  // NOLINT: instruction_queue_status_block_base
    0x485a0,  // NOLINT: instruction_queue_size
    0x485a8,  // NOLINT: instruction_queue_tail
    0x485b0,  // NOLINT: instruction_queue_fetched_head
    0x485b8,  // NOLINT: instruction_queue_completed_head
    0x485c0,  // NOLINT: instruction_queue_int_control
    0x485c8,  // NOLINT: instruction_queue_int_status
    0x48580,  // NOLINT: instruction_queue_minimum_size
    0x48588,  // NOLINT: instruction_queue_maximum_size
    0x46018,  // NOLINT: instruction_queue_int_vector
};

const MemoryCsrOffsets kBeagleMemoryMemoryCsrOffsets = {
    0x42010,  // NOLINT: memoryAccess
    0x42018,  // NOLINT: memoryData
};

const ScalarCoreCsrOffsets kBeagleScalarCoreCsrOffsets = {
    0x44018,                         // NOLINT: scalarCoreRunControl
    0x44038,                         // NOLINT: executeControl
    0x44158,                         // NOLINT: avDataPopRunControl
    0x44198,                         // NOLINT: parameterPopRunControl
    0x44258,                         // NOLINT: scalarCoreRunStatus
    0x44168,                         // NOLINT: avDataPopRunStatus
    0x441a8,                         // NOLINT: parameterPopRunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarDatapath_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, executeControl_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarDatapath_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarDatapath_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, executeControl_1
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarDatapath_1RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_1RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_1RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarDatapath_2RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, executeControl_2
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_2RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_2RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarDatapath_2RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_2RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_2RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarDatapath_3RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, executeControl_3
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_3RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_3RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarDatapath_3RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, avDataPop_3RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, parameterPop_3RunStatus
    0x441d8,                         // NOLINT: infeedRunControl
    0x44218,                         // NOLINT: outfeedRunControl
    0x441e0,                         // NOLINT: infeedRunStatus
    0x44220,                         // NOLINT: outfeedRunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, contextControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, contextStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, contextSwitchCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, contextSwitchTimeoutCount
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, clusterPortControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, clusterPortStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_0_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_0_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_1_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_1_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_2_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_2_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_0RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_1RunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, infeed_3_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, outfeed_3_0RunStatus
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // ScalarCoreRingBusCreditSender_0Reset
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // ScalarCoreRingBusCreditReceiver_0Reset
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // ScalarCoreRingBusCreditSender_1Reset
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // ScalarCoreRingBusCreditReceiver_1Reset
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // ScalarCoreRingBusCreditSender_2Reset
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // ScalarCoreRingBusCreditReceiver_2Reset
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // ScalarCoreRingBusCreditSender_3Reset
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // ScalarCoreRingBusCreditReceiver_3Reset
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, TilePowerInterval
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, peakPowerSampleInterval
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, tdpPowerSampleInterval
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, didtPowerSampleInterval
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, peakSampleAccumulator
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, tdpSampleAccumulator
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, didtSampleAccumulator
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, peakThreshold0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, peakThreshold1
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, peakThreshold2
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, peakThreshold3
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, tdpThreshold0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, tdpThreshold1
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, tdpThreshold2
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, tdpThreshold3
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, didtThreshold0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, peakActionTable
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, tdpActionTable
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, didtActionTable
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, peakRunningSum
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, peakRunningSumInterval
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, tdpRunningSum
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, tdpRunningSumInterval
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, didtRunningSum
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, didtRunningSumInterval
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, didtDifference
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, packageTdpAction
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // ThrottleStallCounter_kMaskOneByFourAllOps
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // ThrottleStallCounter_kMaskTwoByFourAllOps
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // ThrottleStallCounter_kMaskThreeByFourAllOps
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, ThrottleStallCounter
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, scalarCoreClockControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, cycleCount
    0x44260,                         // NOLINT: Error_ScalarCore
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Error_ScalarCoreDatapath_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Error_Mask_ScalarCoreDatapath_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Error_Force_ScalarCoreDatapath_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // Error_Timestamp_ScalarCoreDatapath_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Error_Info_ScalarCoreDatapath_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Error_ScalarCoreDatapath_1
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Error_Mask_ScalarCoreDatapath_1
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Error_Force_ScalarCoreDatapath_1
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // Error_Timestamp_ScalarCoreDatapath_1
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Error_Info_ScalarCoreDatapath_1
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Error_ScalarCoreDatapath_2
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Error_Mask_ScalarCoreDatapath_2
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Error_Force_ScalarCoreDatapath_2
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // Error_Timestamp_ScalarCoreDatapath_2
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Error_Info_ScalarCoreDatapath_2
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Error_ScalarCoreDatapath_3
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Error_Mask_ScalarCoreDatapath_3
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Error_Force_ScalarCoreDatapath_3
    kCsrRegisterSpaceInvalidOffset,  // UNUSED,
                                     // Error_Timestamp_ScalarCoreDatapath_3
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Error_Info_ScalarCoreDatapath_3
};

const MemoryCsrOffsets kBeagleScmemoryMemoryCsrOffsets = {
    0x44040,  // NOLINT: scMemoryAccess
    0x44048,  // NOLINT: scMemoryData
};

const TileConfigCsrOffsets kBeagleTileConfigCsrOffsets = {
    0x48788,  // NOLINT: tileconfig0
    0x48790,  // NOLINT: tileconfig1
};

const TileCsrOffsets kBeagleTileCsrOffsets = {
    0x400c0,                         // NOLINT: opRunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowToNarrowRunControl
    0x40150,                         // NOLINT: narrowToWideRunControl
    0x40110,                         // NOLINT: wideToNarrowRunControl
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, opRunControl_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowToWideRunControl_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, wideToNarrowRunControl_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, opRunControl_1
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowToWideRunControl_1
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, wideToNarrowRunControl_1
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, opRunControl_2
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowToWideRunControl_2
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, wideToNarrowRunControl_2
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, opRunControl_3
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowToWideRunControl_3
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, wideToNarrowRunControl_3
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, opRunControl_4
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowToWideRunControl_4
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, wideToNarrowRunControl_4
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, opRunControl_5
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowToWideRunControl_5
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, wideToNarrowRunControl_5
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, opRunControl_6
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowToWideRunControl_6
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, wideToNarrowRunControl_6
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, opRunControl_7
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowToWideRunControl_7
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, wideToNarrowRunControl_7
    0x40190,                         // NOLINT: ringBusConsumer0RunControl
    0x401d0,                         // NOLINT: ringBusConsumer1RunControl
    0x40210,                         // NOLINT: ringBusProducerRunControl
    0x40250,                         // NOLINT: meshBus0RunControl
    0x40298,                         // NOLINT: meshBus1RunControl
    0x402e0,                         // NOLINT: meshBus2RunControl
    0x40328,                         // NOLINT: meshBus3RunControl
    0x40020,                         // NOLINT: deepSleep
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowMemoryIsolation
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowMemoryRetention
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, EnergyTable
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, didtSampleInterval
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, didtRunningSumInterval
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, opAccumulateRegister
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, didtRunningSumRegister
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, didtThreshold0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowMemoryContext_0
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowMemoryContext_1
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowMemoryContext_2
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowMemoryContext_3
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowMemoryContext_4
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, narrowMemoryContext_5
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, tileContext
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, TileRingBusCreditSenderReset
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, TileRingBusCreditReceiverReset
    0x40370,                         // NOLINT: Error_Tile
    0x40378,                         // NOLINT: Error_Mask_Tile
    0x40380,                         // NOLINT: Error_Force_Tile
    0x40388,                         // NOLINT: Error_Timestamp_Tile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, Error_ContextId_Tile
    0x40390,                         // NOLINT: Error_Info_Tile
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, TileMeshBusCreditSenderReset
    kCsrRegisterSpaceInvalidOffset,  // UNUSED, TileMeshBusCreditReceiverReset
};

const WireCsrOffsets kBeagleWireCsrOffsets = {
    0x48778,  // NOLINT: wire_int_pending_bit_array
    0x48780,  // NOLINT: wire_int_mask_array
};

const InterruptCsrOffsets kBeagleUsbFatalErrIntInterruptCsrOffsets = {
    0x4c060,  // NOLINT: fatal_err_int_control
    0x4c068,  // NOLINT: fatal_err_int_status
};

const InterruptCsrOffsets kBeagleUsbScHostInt0InterruptCsrOffsets = {
    0x4c0b0,  // NOLINT: sc_host_int_0_control
    0x4c0b8,  // NOLINT: sc_host_int_0_status
};

const InterruptCsrOffsets kBeagleUsbScHostInt1InterruptCsrOffsets = {
    0x4c0c8,  // NOLINT: sc_host_int_1_control
    0x4c0d0,  // NOLINT: sc_host_int_1_status
};

const InterruptCsrOffsets kBeagleUsbScHostInt2InterruptCsrOffsets = {
    0x4c0e0,  // NOLINT: sc_host_int_2_control
    0x4c0e8,  // NOLINT: sc_host_int_2_status
};

const InterruptCsrOffsets kBeagleUsbScHostInt3InterruptCsrOffsets = {
    0x4c0f8,  // NOLINT: sc_host_int_3_control
    0x4c100,  // NOLINT: sc_host_int_3_status
};

const InterruptCsrOffsets kBeagleUsbTopLevelInt0InterruptCsrOffsets = {
    0x4c070,  // NOLINT: top_level_int_0_control
    0x4c078,  // NOLINT: top_level_int_0_status
};

const InterruptCsrOffsets kBeagleUsbTopLevelInt1InterruptCsrOffsets = {
    0x4c080,  // NOLINT: top_level_int_1_control
    0x4c088,  // NOLINT: top_level_int_1_status
};

const InterruptCsrOffsets kBeagleUsbTopLevelInt2InterruptCsrOffsets = {
    0x4c090,  // NOLINT: top_level_int_2_control
    0x4c098,  // NOLINT: top_level_int_2_status
};

const InterruptCsrOffsets kBeagleUsbTopLevelInt3InterruptCsrOffsets = {
    0x4c0a0,  // NOLINT: top_level_int_3_control
    0x4c0a8,  // NOLINT: top_level_int_3_status
};

const ApexCsrOffsets kBeagleApexCsrOffsets = {
    0x1a000,  // NOLINT: omc0_00
    0x1a0d4,  // NOLINT: omc0_d4
    0x1a0d8,  // NOLINT: omc0_d8
    0x1a0dc,  // NOLINT: omc0_dc
    0x1a600,  // NOLINT: mst_abm_en
    0x1a500,  // NOLINT: slv_abm_en
    0x1a558,  // NOLINT: slv_err_resp_isr_mask
    0x1a658,  // NOLINT: mst_err_resp_isr_mask
    0x1a640,  // NOLINT: mst_wr_err_resp
    0x1a644,  // NOLINT: mst_rd_err_resp
    0x1a540,  // NOLINT: slv_wr_err_resp
    0x1a544,  // NOLINT: slv_rd_err_resp
    0x1a704,  // NOLINT: rambist_ctrl_1
    0x1a200,  // NOLINT: efuse_00
};

const CbBridgeCsrOffsets kBeagleCbBridgeCsrOffsets = {
    0x19018,  // NOLINT: bo0_fifo_status
    0x1901c,  // NOLINT: bo1_fifo_status
    0x19020,  // NOLINT: bo2_fifo_status
    0x19024,  // NOLINT: bo3_fifo_status
    0x1907c,  // NOLINT: gcbb_credit0
};

const MiscCsrOffsets kBeagleMiscCsrOffsets = {
    0x4a000,  // NOLINT: idleRegister
};

const MsixCsrOffsets kBeagleMsixCsrOffsets = {
    0x46018,  // NOLINT: instruction_queue_int_vector
    0x46020,  // NOLINT: input_actv_queue_int_vector
    0x46028,  // NOLINT: param_queue_int_vector
    0x46030,  // NOLINT: output_actv_queue_int_vector
    0x46040,  // NOLINT: top_level_int_vector
    0x46038,  // NOLINT: sc_host_int_vector
    0x46048,  // NOLINT: fatal_err_int_vector
    0x46068,  // NOLINT: msix_pending_bit_array0
    0x46800,  // NOLINT: msix_table
};

const ScuCsrOffsets kBeagleScuCsrOffsets = {
    0x1a30c,  // NOLINT: scu_ctrl_0
    0x1a310,  // NOLINT: scu_ctrl_1
    0x1a314,  // NOLINT: scu_ctrl_2
    0x1a318,  // NOLINT: scu_ctrl_3
    0x1a31c,  // NOLINT: scu_ctrl_4
    0x1a320,  // NOLINT: scu_ctrl_5
    0x1a32c,  // NOLINT: scu_ctr_6
    0x1a33c,  // NOLINT: scu_ctr_7
};

const UsbCsrOffsets kBeagleUsbCsrOffsets = {
    0x4c058,  // NOLINT: outfeed_chunk_length
    0x4c148,  // NOLINT: descr_ep
    0x4c150,  // NOLINT: ep_status_credit
    0x4c160,  // NOLINT: multi_bo_ep
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_CONFIG_BEAGLE_BEAGLE_CSR_OFFSETS_H_
