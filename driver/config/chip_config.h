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

#ifndef DARWINN_DRIVER_CONFIG_CHIP_CONFIG_H_
#define DARWINN_DRIVER_CONFIG_CHIP_CONFIG_H_

#include "api/chip.h"
#include "driver/config/apex_csr_offsets.h"
#include "driver/config/breakpoint_csr_offsets.h"
#include "driver/config/cb_bridge_csr_offsets.h"
#include "driver/config/chip_structures.h"
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
#include "driver/config/register_file_csr_offsets.h"
#include "driver/config/scalar_core_csr_offsets.h"
#include "driver/config/scu_csr_offsets.h"
#include "driver/config/sync_flag_csr_offsets.h"
#include "driver/config/tile_config_csr_offsets.h"
#include "driver/config/tile_csr_offsets.h"
#include "driver/config/tile_thread_csr_offsets.h"
#include "driver/config/trace_csr_offsets.h"
#include "driver/config/usb_csr_offsets.h"
#include "driver/config/wire_csr_offsets.h"
#include "port/logging.h"
#include "port/unreachable.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// Project-independent interface for CSR offsets and system constants.
class ChipConfig {
 public:
  virtual ~ChipConfig() = default;

  virtual api::Chip GetChip() const = 0;

  // Extracts CSR offsets for various modules in DarwiNN.
  virtual const HibKernelCsrOffsets& GetHibKernelCsrOffsets() const = 0;
  virtual const HibUserCsrOffsets& GetHibUserCsrOffsets() const = 0;
  virtual const QueueCsrOffsets& GetInstructionQueueCsrOffsets() const = 0;
  virtual const HibUserCsrOffsets& GetContextSpecificHibUserCsrOffsets(
      int context_id) const = 0;
  virtual const QueueCsrOffsets& GetContextSpecificInstructionQueueCsrOffsets(
      int context_id) const = 0;
  virtual const ScalarCoreCsrOffsets& GetScalarCoreCsrOffsets() const = 0;
  virtual const TileConfigCsrOffsets& GetTileConfigCsrOffsets() const = 0;
  virtual const TileCsrOffsets& GetTileCsrOffsets() const = 0;
  virtual bool HasThreadCsrOffsets() const { return false; }
  virtual const TileThreadCsrOffsets& GetTileThread0CsrOffsets() const {
    LOG(FATAL) << "Tile thread 0 not supported.";
    unreachable();
  }
  virtual const TileThreadCsrOffsets& GetTileThread1CsrOffsets() const {
    LOG(FATAL) << "Tile thread 1 not supported.";
    unreachable();
  }
  virtual const TileThreadCsrOffsets& GetTileThread2CsrOffsets() const {
    LOG(FATAL) << "Tile thread 2 not supported.";
    unreachable();
  }
  virtual const TileThreadCsrOffsets& GetTileThread3CsrOffsets() const {
    LOG(FATAL) << "Tile thread 3 not supported.";
    unreachable();
  }
  virtual const TileThreadCsrOffsets& GetTileThread4CsrOffsets() const {
    LOG(FATAL) << "Tile thread 4 not supported.";
    unreachable();
  }
  virtual const TileThreadCsrOffsets& GetTileThread5CsrOffsets() const {
    LOG(FATAL) << "Tile thread 5 not supported.";
    unreachable();
  }
  virtual const TileThreadCsrOffsets& GetTileThread6CsrOffsets() const {
    LOG(FATAL) << "Tile thread 6 not supported.";
    unreachable();
  }
  virtual const TileThreadCsrOffsets& GetTileThread7CsrOffsets() const {
    LOG(FATAL) << "Tile thread 7 not supported.";
    unreachable();
  }
  virtual const InterruptCsrOffsets& GetScalarCoreInterruptCsrOffsets()
      const = 0;
  virtual const InterruptCsrOffsets& GetTopLevelInterruptCsrOffsets() const = 0;
  virtual const InterruptCsrOffsets& GetFatalErrorInterruptCsrOffsets()
      const = 0;
  virtual const InterruptCsrOffsets&
  GetContextSpecificScalarCoreInterruptCsrOffsets(int context_id) const = 0;
  virtual const InterruptCsrOffsets&
  GetContextSpecificTopLevelInterruptCsrOffsets(int context_id) const = 0;
  virtual const InterruptCsrOffsets&
  GetContextSpecificFatalErrorInterruptCsrOffsets(int context_id) const = 0;
  // Extracts CSR offsets that supports specific functionality in DarwiNN.
  virtual const MsixCsrOffsets& GetMsixCsrOffsets() const {
    LOG(FATAL) << "MSIX interrupt not supported.";
    unreachable();
  }
  virtual const WireCsrOffsets& GetWireCsrOffsets() const = 0;
  virtual const WireCsrOffsets& GetContextSpecificWireCsrOffsets(
      int context_id) const = 0;
  virtual const MiscCsrOffsets& GetMiscCsrOffsets() const {
    LOG(FATAL) << "Misc not supported.";
    unreachable();
  }

  // Extracts chip-specific constants in DarwiNN.
  virtual const ChipStructures& GetChipStructures() const = 0;

  // Extracts CSR offsets used by scalar core debugger in DarwiNN.
  virtual const BreakpointCsrOffsets& GetScalarCoreBreakpointCsrOffsets()
      const = 0;
  virtual const BreakpointCsrOffsets&
  GetScalarCoreActivationTtuBreakpointCsrOffsets() const = 0;
  virtual const BreakpointCsrOffsets&
  GetScalarCoreInfeedTtuBreakpointCsrOffsets() const = 0;
  virtual const BreakpointCsrOffsets&
  GetScalarCoreOutfeedTtuBreakpointCsrOffsets() const = 0;
  virtual const BreakpointCsrOffsets&
  GetScalarCoreParameterTtuBreakpointCsrOffsets() const = 0;

  virtual const RegisterFileCsrOffsets& GetScalarRegisterFileCsrOffsets()
      const = 0;
  virtual const RegisterFileCsrOffsets& GetPredicateRegisterFileCsrOffsets()
      const = 0;

  virtual const MemoryCsrOffsets& GetScalarCoreMemoryCsrOffsets() const = 0;

  // Extracts CSR offsets used by tile debugger in DarwiNN.
  virtual const BreakpointCsrOffsets& GetTileOpTtuBreakpointCsrOffsets()
      const = 0;
  virtual const BreakpointCsrOffsets&
  GetTileWideToNarrowTtuBreakpointCsrOffsets() const = 0;
  virtual const BreakpointCsrOffsets&
  GetTileNarrowToWideTtuBreakpointCsrOffsets() const = 0;
  virtual const BreakpointCsrOffsets&
  GetTileRingBusConsumer0TtuBreakpointCsrOffsets() const = 0;
  virtual const BreakpointCsrOffsets&
  GetTileRingBusConsumer1TtuBreakpointCsrOffsets() const = 0;
  virtual const BreakpointCsrOffsets&
  GetTileRingBusProducerTtuBreakpointCsrOffsets() const = 0;
  virtual const BreakpointCsrOffsets& GetTileMeshBus0TtuBreakpointCsrOffsets()
      const = 0;
  virtual const BreakpointCsrOffsets& GetTileMeshBus1TtuBreakpointCsrOffsets()
      const = 0;
  virtual const BreakpointCsrOffsets& GetTileMeshBus2TtuBreakpointCsrOffsets()
      const = 0;
  virtual const BreakpointCsrOffsets& GetTileMeshBus3TtuBreakpointCsrOffsets()
      const = 0;

  virtual const MemoryCsrOffsets& GetTileMemoryCsrOffsets() const = 0;

  // Extracts CSR offsets used by scalar core performance tracing.
  virtual const TraceCsrOffsets& GetScalarCoreActivationTtuTraceCsrOffsets()
      const = 0;
  virtual const TraceCsrOffsets& GetScalarCoreInfeedTtuTraceCsrOffsets()
      const = 0;
  virtual const TraceCsrOffsets& GetScalarCoreOutfeedTtuTraceCsrOffsets()
      const = 0;
  virtual const TraceCsrOffsets& GetScalarCoreParameterTtuTraceCsrOffsets()
      const = 0;

  // Extracts CSR offsets used by tile performance tracing.
  virtual const TraceCsrOffsets& GetTileOpTtuTraceCsrOffsets() const = 0;
  virtual const TraceCsrOffsets& GetTileWideToNarrowTtuTraceCsrOffsets()
      const = 0;
  virtual const TraceCsrOffsets& GetTileNarrowToWideTtuTraceCsrOffsets()
      const = 0;
  virtual const TraceCsrOffsets& GetTileRingBusConsumer0TtuTraceCsrOffsets()
      const = 0;
  virtual const TraceCsrOffsets& GetTileRingBusConsumer1TtuTraceCsrOffsets()
      const = 0;
  virtual const TraceCsrOffsets& GetTileRingBusProducerTtuTraceCsrOffsets()
      const = 0;
  virtual const TraceCsrOffsets& GetTileMeshBus0TtuTraceCsrOffsets() const = 0;
  virtual const TraceCsrOffsets& GetTileMeshBus1TtuTraceCsrOffsets() const = 0;
  virtual const TraceCsrOffsets& GetTileMeshBus2TtuTraceCsrOffsets() const = 0;
  virtual const TraceCsrOffsets& GetTileMeshBus3TtuTraceCsrOffsets() const = 0;

  virtual const TraceCsrOffsets&
  GetScalarCoreIrqCompletionBufferTraceCsrOffsets() const {
    LOG(FATAL) << "Irq completion buffer trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileDmaNarrowToWide0TraceCsrOffsets()
      const {
    LOG(FATAL) << "DMA narrow to wide 0 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileDmaNarrowToWide1TraceCsrOffsets()
      const {
    LOG(FATAL) << "DMA narrow to wide 1 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileDmaNarrowToWide2TraceCsrOffsets()
      const {
    LOG(FATAL) << "DMA narrow to wide 2 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileDmaNarrowToWide3TraceCsrOffsets()
      const {
    LOG(FATAL) << "DMA narrow to wide 3 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileDmaNarrowToWide4TraceCsrOffsets()
      const {
    LOG(FATAL) << "DMA narrow to wide 4 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileDmaNarrowToWide5TraceCsrOffsets()
      const {
    LOG(FATAL) << "DMA narrow to wide 5 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileDmaNarrowToWide6TraceCsrOffsets()
      const {
    LOG(FATAL) << "DMA narrow to wide 6 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileDmaNarrowToWide7TraceCsrOffsets()
      const {
    LOG(FATAL) << "DMA narrow to wide 7 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileDmaWideToNarrow0TraceCsrOffsets()
      const {
    LOG(FATAL) << "DMA wide to narrow 0 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileDmaWideToNarrow1TraceCsrOffsets()
      const {
    LOG(FATAL) << "DMA wide to narrow 1 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileDmaWideToNarrow2TraceCsrOffsets()
      const {
    LOG(FATAL) << "DMA wide to narrow 2 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileDmaWideToNarrow3TraceCsrOffsets()
      const {
    LOG(FATAL) << "DMA wide to narrow 3 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileDmaWideToNarrow4TraceCsrOffsets()
      const {
    LOG(FATAL) << "DMA wide to narrow 4 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileDmaWideToNarrow5TraceCsrOffsets()
      const {
    LOG(FATAL) << "DMA wide to narrow 5 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileDmaWideToNarrow6TraceCsrOffsets()
      const {
    LOG(FATAL) << "DMA wide to narrow 6 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileDmaWideToNarrow7TraceCsrOffsets()
      const {
    LOG(FATAL) << "DMA wide to narrow 7 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileDmaNarrowToNarrowTraceCsrOffsets()
      const {
    LOG(FATAL) << "DMA narrow to narrow trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileOp0TraceCsrOffsets() const {
    LOG(FATAL) << "Op0 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileOp1TraceCsrOffsets() const {
    LOG(FATAL) << "Op1 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileOp2TraceCsrOffsets() const {
    LOG(FATAL) << "Op2 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileOp3TraceCsrOffsets() const {
    LOG(FATAL) << "Op3 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileOp4TraceCsrOffsets() const {
    LOG(FATAL) << "Op4 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileOp5TraceCsrOffsets() const {
    LOG(FATAL) << "Op5 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileOp6TraceCsrOffsets() const {
    LOG(FATAL) << "Op6 trace not supported.";
    unreachable();
  }
  virtual const TraceCsrOffsets& GetTileOp7TraceCsrOffsets() const {
    LOG(FATAL) << "Op7 trace not supported.";
    unreachable();
  }

  // Extracts CSR offsets used to access sync flags in scalar core.
  virtual const SyncFlagCsrOffsets& GetScalarCoreAvdataPopSyncFlagCsrOffsets()
      const = 0;
  virtual const SyncFlagCsrOffsets&
  GetScalarCoreParameterPopSyncFlagCsrOffsets() const = 0;
  virtual const SyncFlagCsrOffsets&
  GetScalarCoreAvdataInfeedSyncFlagCsrOffsets() const = 0;
  virtual const SyncFlagCsrOffsets&
  GetScalarCoreParameterInfeedSyncFlagCsrOffsets() const = 0;
  virtual const SyncFlagCsrOffsets&
  GetScalarCoreScalarInfeedSyncFlagCsrOffsets() const = 0;
  virtual const SyncFlagCsrOffsets& GetScalarCoreProducerASyncFlagCsrOffsets()
      const = 0;
  virtual const SyncFlagCsrOffsets& GetScalarCoreProducerBSyncFlagCsrOffsets()
      const = 0;
  virtual const SyncFlagCsrOffsets& GetScalarCoreRingOutfeedSyncFlagCsrOffsets()
      const = 0;
  virtual const SyncFlagCsrOffsets&
  GetScalarCoreScalarPipelineSyncFlagCsrOffsets() const = 0;

  // Extracts CSR offsets used by bug report generator in DarwiNN.
  virtual const DebugHibUserCsrOffsets& GetDebugHibUserCsrOffsets() const = 0;
  virtual const DebugHibUserCsrOffsets&
  GetContextSpecificDebugHibUserCsrOffsets(int context_id) const = 0;
  virtual const DebugScalarCoreCsrOffsets& GetDebugScalarCoreCsrOffsets()
      const = 0;
  virtual const DebugTileCsrOffsets& GetDebugTileCsrOffsets() const = 0;

  // Beagle-specific.
  virtual const ApexCsrOffsets& GetApexCsrOffsets() const {
    LOG(FATAL) << "Apex not supported.";
    unreachable();
  }
  virtual const ScuCsrOffsets& GetScuCsrOffsets() const {
    LOG(FATAL) << "SCU not supported.";
    unreachable();
  }
  virtual const CbBridgeCsrOffsets& GetCbBridgeCsrOffsets() const {
    LOG(FATAL) << "CB bridge not supported.";
    unreachable();
  }
  virtual const UsbCsrOffsets& GetUsbCsrOffsets() const {
    LOG(FATAL) << "USB not supported.";
    unreachable();
  }
  virtual const InterruptCsrOffsets& GetUsbFatalErrorInterruptCsrOffsets()
      const {
    LOG(FATAL) << "USB not supported.";
    unreachable();
  }
  virtual const InterruptCsrOffsets& GetUsbTopLevel0InterruptCsrOffsets()
      const {
    LOG(FATAL) << "USB not supported.";
    unreachable();
  }
  virtual const InterruptCsrOffsets& GetUsbTopLevel1InterruptCsrOffsets()
      const {
    LOG(FATAL) << "USB not supported.";
    unreachable();
  }
  virtual const InterruptCsrOffsets& GetUsbTopLevel2InterruptCsrOffsets()
      const {
    LOG(FATAL) << "USB not supported.";
    unreachable();
  }
  virtual const InterruptCsrOffsets& GetUsbTopLevel3InterruptCsrOffsets()
      const {
    LOG(FATAL) << "USB not supported.";
    unreachable();
  }
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_CONFIG_CHIP_CONFIG_H_
