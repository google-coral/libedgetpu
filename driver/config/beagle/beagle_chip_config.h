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

#ifndef DARWINN_DRIVER_CONFIG_BEAGLE_BEAGLE_CHIP_CONFIG_H_
#define DARWINN_DRIVER_CONFIG_BEAGLE_BEAGLE_CHIP_CONFIG_H_

#include "driver/config/beagle/beagle_chip_structures.h"
#include "driver/config/beagle/beagle_csr_offsets.h"
#include "driver/config/chip_config.h"
#include "port/logging.h"
#include "port/unreachable.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// Beagle-specific configuration.
class BeagleChipConfig : public ChipConfig {
 public:
  ~BeagleChipConfig() override = default;

  api::Chip GetChip() const override { return api::Chip::kBeagle; }

  // Extracts CSR offsets for various modules in DarwiNN.
  const HibKernelCsrOffsets& GetHibKernelCsrOffsets() const override {
    return kBeagleHibKernelCsrOffsets;
  }
  const HibUserCsrOffsets& GetHibUserCsrOffsets() const override {
    return kBeagleHibUserCsrOffsets;
  }
  const QueueCsrOffsets& GetInstructionQueueCsrOffsets() const override {
    return kBeagleInstructionQueueCsrOffsets;
  }
  const HibUserCsrOffsets& GetContextSpecificHibUserCsrOffsets(
      int context_id) const override {
    CHECK_EQ(context_id, 0);
    return kBeagleHibUserCsrOffsets;
  }
  const QueueCsrOffsets& GetContextSpecificInstructionQueueCsrOffsets(
      int context_id) const override {
    CHECK_EQ(context_id, 0);
    return kBeagleInstructionQueueCsrOffsets;
  }
  const InterruptCsrOffsets& GetContextSpecificScalarCoreInterruptCsrOffsets(
      int context_id) const override {
    CHECK_EQ(context_id, 0);
    return kBeagleScHostIntInterruptCsrOffsets;
  }
  const InterruptCsrOffsets& GetContextSpecificTopLevelInterruptCsrOffsets(
      int context_id) const override {
    CHECK_EQ(context_id, 0);
    return kBeagleTopLevelIntInterruptCsrOffsets;
  }
  const InterruptCsrOffsets& GetContextSpecificFatalErrorInterruptCsrOffsets(
      int context_id) const override {
    CHECK_EQ(context_id, 0);
    return kBeagleFatalErrIntInterruptCsrOffsets;
  }
  const WireCsrOffsets& GetContextSpecificWireCsrOffsets(
      int context_id) const override {
    CHECK_EQ(context_id, 0);
    return kBeagleWireCsrOffsets;
  }
  const DebugHibUserCsrOffsets& GetContextSpecificDebugHibUserCsrOffsets(
      int context_id) const override {
    CHECK_EQ(context_id, 0);
    return kBeagleDebugHibUserCsrOffsets;
  }
  const ScalarCoreCsrOffsets& GetScalarCoreCsrOffsets() const override {
    return kBeagleScalarCoreCsrOffsets;
  }
  const TileConfigCsrOffsets& GetTileConfigCsrOffsets() const override {
    return kBeagleTileConfigCsrOffsets;
  }
  const TileCsrOffsets& GetTileCsrOffsets() const override {
    return kBeagleTileCsrOffsets;
  }
  const InterruptCsrOffsets& GetScalarCoreInterruptCsrOffsets() const override {
    return kBeagleScHostIntInterruptCsrOffsets;
  }
  const InterruptCsrOffsets& GetTopLevelInterruptCsrOffsets() const override {
    return kBeagleTopLevelIntInterruptCsrOffsets;
  }
  const InterruptCsrOffsets& GetFatalErrorInterruptCsrOffsets() const override {
    return kBeagleFatalErrIntInterruptCsrOffsets;
  }

  // Extracts CSR offsets that supports specific functionality in DarwiNN.
  const MsixCsrOffsets& GetMsixCsrOffsets() const override {
    return kBeagleMsixCsrOffsets;
  }
  const WireCsrOffsets& GetWireCsrOffsets() const override {
    LOG(FATAL) << "Wire interrupt not supported.";
    unreachable();
  }
  const MiscCsrOffsets& GetMiscCsrOffsets() const override {
    return kBeagleMiscCsrOffsets;
  }

  // Extracts chip-specific constants in DarwiNN.
  const ChipStructures& GetChipStructures() const override {
    return kBeagleChipStructures;
  }

  // Extracts CSR offsets used by scalar core debugger in DarwiNN.
  const BreakpointCsrOffsets& GetScalarCoreBreakpointCsrOffsets()
      const override {
    return kBeagleScalarcoreBreakpointCsrOffsets;
  }
  const BreakpointCsrOffsets& GetScalarCoreActivationTtuBreakpointCsrOffsets()
      const override {
    return kBeagleAvdatapopBreakpointCsrOffsets;
  }
  const BreakpointCsrOffsets& GetScalarCoreInfeedTtuBreakpointCsrOffsets()
      const override {
    return kBeagleInfeedBreakpointCsrOffsets;
  }
  const BreakpointCsrOffsets& GetScalarCoreOutfeedTtuBreakpointCsrOffsets()
      const override {
    return kBeagleOutfeedBreakpointCsrOffsets;
  }
  const BreakpointCsrOffsets& GetScalarCoreParameterTtuBreakpointCsrOffsets()
      const override {
    return kBeagleParameterpopBreakpointCsrOffsets;
  }

  const RegisterFileCsrOffsets& GetScalarRegisterFileCsrOffsets()
      const override {
    return kBeagleScalarRegisterFileCsrOffsets;
  }
  const RegisterFileCsrOffsets& GetPredicateRegisterFileCsrOffsets()
      const override {
    return kBeaglePredicateRegisterFileCsrOffsets;
  }

  const MemoryCsrOffsets& GetScalarCoreMemoryCsrOffsets() const override {
    return kBeagleScmemoryMemoryCsrOffsets;
  }

  // Extracts CSR offsets used by tile debugger in DarwiNN.
  const BreakpointCsrOffsets& GetTileOpTtuBreakpointCsrOffsets()
      const override {
    return kBeagleOpBreakpointCsrOffsets;
  }
  const BreakpointCsrOffsets& GetTileWideToNarrowTtuBreakpointCsrOffsets()
      const override {
    return kBeagleWidetonarrowBreakpointCsrOffsets;
  }
  const BreakpointCsrOffsets& GetTileNarrowToWideTtuBreakpointCsrOffsets()
      const override {
    return kBeagleNarrowtowideBreakpointCsrOffsets;
  }
  const BreakpointCsrOffsets& GetTileRingBusConsumer0TtuBreakpointCsrOffsets()
      const override {
    return kBeagleRingbusconsumer0BreakpointCsrOffsets;
  }
  const BreakpointCsrOffsets& GetTileRingBusConsumer1TtuBreakpointCsrOffsets()
      const override {
    return kBeagleRingbusconsumer1BreakpointCsrOffsets;
  }
  const BreakpointCsrOffsets& GetTileRingBusProducerTtuBreakpointCsrOffsets()
      const override {
    return kBeagleRingbusproducerBreakpointCsrOffsets;
  }
  const BreakpointCsrOffsets& GetTileMeshBus0TtuBreakpointCsrOffsets()
      const override {
    return kBeagleMeshbus0BreakpointCsrOffsets;
  }
  const BreakpointCsrOffsets& GetTileMeshBus1TtuBreakpointCsrOffsets()
      const override {
    return kBeagleMeshbus1BreakpointCsrOffsets;
  }
  const BreakpointCsrOffsets& GetTileMeshBus2TtuBreakpointCsrOffsets()
      const override {
    return kBeagleMeshbus2BreakpointCsrOffsets;
  }
  const BreakpointCsrOffsets& GetTileMeshBus3TtuBreakpointCsrOffsets()
      const override {
    return kBeagleMeshbus3BreakpointCsrOffsets;
  }

  const MemoryCsrOffsets& GetTileMemoryCsrOffsets() const override {
    return kBeagleMemoryMemoryCsrOffsets;
  }

  // Extracts CSR offsets used by scalar core performance tracing.
  const TraceCsrOffsets& GetScalarCoreActivationTtuTraceCsrOffsets()
      const override {
    return kBeagleAvdatapopTraceCsrOffsets;
  }
  const TraceCsrOffsets& GetScalarCoreInfeedTtuTraceCsrOffsets()
      const override {
    return kBeagleInfeedTraceCsrOffsets;
  }
  const TraceCsrOffsets& GetScalarCoreOutfeedTtuTraceCsrOffsets()
      const override {
    return kBeagleOutfeedTraceCsrOffsets;
  }
  const TraceCsrOffsets& GetScalarCoreParameterTtuTraceCsrOffsets()
      const override {
    return kBeagleParameterpopTraceCsrOffsets;
  }

  // Extracts CSR offsets used by tile performance tracing.
  const TraceCsrOffsets& GetTileOpTtuTraceCsrOffsets() const override {
    return kBeagleOpTraceCsrOffsets;
  }
  const TraceCsrOffsets& GetTileWideToNarrowTtuTraceCsrOffsets()
      const override {
    return kBeagleDmawidetonarrowTraceCsrOffsets;
  }
  const TraceCsrOffsets& GetTileNarrowToWideTtuTraceCsrOffsets()
      const override {
    return kBeagleDmanarrowtowideTraceCsrOffsets;
  }
  const TraceCsrOffsets& GetTileRingBusConsumer0TtuTraceCsrOffsets()
      const override {
    return kBeagleDmaringbusconsumer0TraceCsrOffsets;
  }
  const TraceCsrOffsets& GetTileRingBusConsumer1TtuTraceCsrOffsets()
      const override {
    return kBeagleDmaringbusconsumer1TraceCsrOffsets;
  }
  const TraceCsrOffsets& GetTileRingBusProducerTtuTraceCsrOffsets()
      const override {
    return kBeagleDmaringbusproducerTraceCsrOffsets;
  }
  const TraceCsrOffsets& GetTileMeshBus0TtuTraceCsrOffsets() const override {
    return kBeagleDmameshbus0TraceCsrOffsets;
  }
  const TraceCsrOffsets& GetTileMeshBus1TtuTraceCsrOffsets() const override {
    return kBeagleDmameshbus1TraceCsrOffsets;
  }
  const TraceCsrOffsets& GetTileMeshBus2TtuTraceCsrOffsets() const override {
    return kBeagleDmameshbus2TraceCsrOffsets;
  }
  const TraceCsrOffsets& GetTileMeshBus3TtuTraceCsrOffsets() const override {
    return kBeagleDmameshbus3TraceCsrOffsets;
  }

  // Extracts CSR offsets used to access sync flags in scalar core.
  const SyncFlagCsrOffsets& GetScalarCoreAvdataPopSyncFlagCsrOffsets()
      const override {
    return kBeagleAvdataPopSyncFlagCsrOffsets;
  }
  const SyncFlagCsrOffsets& GetScalarCoreParameterPopSyncFlagCsrOffsets()
      const override {
    return kBeagleParameterPopSyncFlagCsrOffsets;
  }
  const SyncFlagCsrOffsets& GetScalarCoreAvdataInfeedSyncFlagCsrOffsets()
      const override {
    return kBeagleAvdataInfeedSyncFlagCsrOffsets;
  }
  const SyncFlagCsrOffsets& GetScalarCoreParameterInfeedSyncFlagCsrOffsets()
      const override {
    return kBeagleParameterInfeedSyncFlagCsrOffsets;
  }
  const SyncFlagCsrOffsets& GetScalarCoreScalarInfeedSyncFlagCsrOffsets()
      const override {
    return kBeagleScalarInfeedSyncFlagCsrOffsets;
  }
  const SyncFlagCsrOffsets& GetScalarCoreProducerASyncFlagCsrOffsets()
      const override {
    return kBeagleProducerASyncFlagCsrOffsets;
  }
  const SyncFlagCsrOffsets& GetScalarCoreProducerBSyncFlagCsrOffsets()
      const override {
    return kBeagleProducerBSyncFlagCsrOffsets;
  }
  const SyncFlagCsrOffsets& GetScalarCoreRingOutfeedSyncFlagCsrOffsets()
      const override {
    return kBeagleRingOutfeedSyncFlagCsrOffsets;
  }
  const SyncFlagCsrOffsets& GetScalarCoreScalarPipelineSyncFlagCsrOffsets()
      const override {
    return kBeagleScalarPipelineSyncFlagCsrOffsets;
  }

  // Extracts CSR offsets used by bug report generator in DarwiNN.
  const DebugHibUserCsrOffsets& GetDebugHibUserCsrOffsets() const override {
    return kBeagleDebugHibUserCsrOffsets;
  }
  const DebugScalarCoreCsrOffsets& GetDebugScalarCoreCsrOffsets()
      const override {
    return kBeagleDebugScalarCoreCsrOffsets;
  }
  const DebugTileCsrOffsets& GetDebugTileCsrOffsets() const override {
    return kBeagleDebugTileCsrOffsets;
  }

  // Beagle-specific.
  const ApexCsrOffsets& GetApexCsrOffsets() const override {
    return kBeagleApexCsrOffsets;
  }
  const ScuCsrOffsets& GetScuCsrOffsets() const override {
    return kBeagleScuCsrOffsets;
  }
  const CbBridgeCsrOffsets& GetCbBridgeCsrOffsets() const override {
    return kBeagleCbBridgeCsrOffsets;
  }
  const UsbCsrOffsets& GetUsbCsrOffsets() const override {
    return kBeagleUsbCsrOffsets;
  }
  const InterruptCsrOffsets& GetUsbFatalErrorInterruptCsrOffsets()
      const override {
    return kBeagleUsbFatalErrIntInterruptCsrOffsets;
  }
  const InterruptCsrOffsets& GetUsbTopLevel0InterruptCsrOffsets()
      const override {
    return kBeagleUsbTopLevelInt0InterruptCsrOffsets;
  }
  const InterruptCsrOffsets& GetUsbTopLevel1InterruptCsrOffsets()
      const override {
    return kBeagleUsbTopLevelInt1InterruptCsrOffsets;
  }
  const InterruptCsrOffsets& GetUsbTopLevel2InterruptCsrOffsets()
      const override {
    return kBeagleUsbTopLevelInt2InterruptCsrOffsets;
  }
  const InterruptCsrOffsets& GetUsbTopLevel3InterruptCsrOffsets()
      const override {
    return kBeagleUsbTopLevelInt3InterruptCsrOffsets;
  }
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_CONFIG_BEAGLE_BEAGLE_CHIP_CONFIG_H_
