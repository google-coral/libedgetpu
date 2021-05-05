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

#ifndef DARWINN_DRIVER_MMIO_DRIVER_H_
#define DARWINN_DRIVER_MMIO_DRIVER_H_

#include <atomic>
#include <condition_variable>  // NOLINT
#include <memory>
#include <mutex>  // NOLINT
#include <queue>

#include "api/allocated_buffer.h"
#include "api/buffer.h"
#include "driver/allocator.h"
#include "driver/config/chip_config.h"
#include "driver/config/chip_structures.h"
#include "driver/config/hib_kernel_csr_offsets.h"
#include "driver/config/hib_user_csr_offsets.h"
#include "driver/device_buffer.h"
#include "driver/dma_info_extractor.h"
#include "driver/driver.h"
#include "driver/hardware_structures.h"
#include "driver/interrupt/interrupt_controller_interface.h"
#include "driver/interrupt/interrupt_handler.h"
#include "driver/interrupt/top_level_interrupt_manager.h"
#include "driver/memory/address_space.h"
#include "driver/memory/dma_direction.h"
#include "driver/memory/dram_allocator.h"
#include "driver/memory/mmu_mapper.h"
#include "driver/mmio/host_queue.h"
#include "driver/package_registry.h"
#include "driver/real_time_dma_scheduler.h"
#include "driver/registers/registers.h"
#include "driver/run_controller.h"
#include "driver/scalar_core_controller.h"
#include "driver/top_level_handler.h"
#include "driver/tpu_request.h"
#include "driver_shared/time_stamper/time_stamper.h"
#include "executable/executable_generated.h"
#include "port/integral_types.h"
#include "port/statusor.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// DarwiNN driver implementation that talks to the device through memory-mapped
// IO setup with a kernel device driver. Thread safe.
class MmioDriver : public Driver {
 public:
  MmioDriver(
      const api::DriverOptions& options,
      std::unique_ptr<config::ChipConfig> chip_config,
      std::unique_ptr<Registers> registers,
      std::unique_ptr<DramAllocator> dram_allocator,
      std::unique_ptr<MmuMapper> mmu_mapper,
      std::unique_ptr<AddressSpace> address_space,
      std::unique_ptr<Allocator> allocator,
      std::unique_ptr<HostQueue<HostQueueDescriptor, HostQueueStatusBlock>>
          instruction_queue,
      std::unique_ptr<InterruptHandler> interrupt_handler,
      std::unique_ptr<TopLevelInterruptManager> top_level_interrupt_manager,
      std::unique_ptr<InterruptControllerInterface>
          fatal_error_interrupt_controller,
      std::unique_ptr<ScalarCoreController> scalar_core_controller,
      std::unique_ptr<RunController> run_controller,
      std::unique_ptr<TopLevelHandler> top_level_handler,
      std::unique_ptr<PackageRegistry> executable_registry,
      std::unique_ptr<driver_shared::TimeStamper> time_stamper);

  // This class is neither copyable nor movable.
  MmioDriver(const MmioDriver&) = delete;
  MmioDriver& operator=(const MmioDriver&) = delete;

  ~MmioDriver() override;

  uint64_t allocation_alignment_bytes() const override {
    return chip_structure_.allocation_alignment_bytes;
  }

 protected:
  Status DoOpen(bool debug_mode) LOCKS_EXCLUDED(state_mutex_) override;
  Status DoClose(bool in_error, api::Driver::ClosingMode mode)
      LOCKS_EXCLUDED(state_mutex_) override;
  Status DoCancelAndWaitRequests(bool in_error)
      LOCKS_EXCLUDED(state_mutex_) override;

  Buffer DoMakeBuffer(size_t size_bytes) const override;

  StatusOr<MappedDeviceBuffer> DoMapBuffer(const Buffer& buffer,
                                           DmaDirection direction) override;

  StatusOr<std::shared_ptr<TpuRequest>> DoCreateRequest(
      const std::shared_ptr<Request> parent_request,
      const ExecutableReference* executable, TpuRequest::RequestType type)
      LOCKS_EXCLUDED(state_mutex_) override;

  // We do support real-time mode in this driver.
  bool HasImplementedRealtimeMode() const final { return true; }

  Status DoSetExecutableTiming(const ExecutableReference* executable,
                               const api::Timing& timing) final {
    return dma_scheduler_.SetExecutableTiming(executable, timing);
  }

  Status DoRemoveExecutableTiming(const ExecutableReference* executable) {
    return dma_scheduler_.RemoveExecutableTiming(executable);
  }

  Status DoSetRealtimeMode(bool on) final;

  Status DoSubmit(std::shared_ptr<driver::TpuRequest> request)
      LOCKS_EXCLUDED(state_mutex_) override;

  int64 MaxRemainingCycles() const override {
    return dma_scheduler_.MaxRemainingCycles();
  }

  // Returns a pointer to the registers in this driver. The pointer is valid as
  // long as the driver instance is.
  Registers* registers() { return registers_.get(); }

  // Returns a reference to the chip config. It is valid as long as the
  // MmioDriver instance is.
  const config::ChipConfig& chip_config() const {
    return *chip_config_;
  }

  StatusOr<std::shared_ptr<TpuRequest>> GetOldestActiveRequest()
      const override {
    return dma_scheduler_.GetOldestActiveRequest();
  }

 private:
  // TODO: Eliminate state management here. Since this is now done
  // in the base class.
  // Driver state. Transitions :
  //  kClosed -> kOpen -> kClosing -> kClosed.
  enum State {
    kOpen,     // Driver is Open.
    kClosing,  // Driver is Closing.
    kClosed,   // Driver is Closed. (Initial state.)
  };

  // Attempts a state transition to the given state.
  Status SetState(State next_state) EXCLUSIVE_LOCKS_REQUIRED(state_mutex_);

  // Validates that we are in the expected state.
  Status ValidateState(State expected_state) const
      SHARED_LOCKS_REQUIRED(state_mutex_);

  // Attempts to issue as many DMAs as possible.
  Status TryIssueDmas() LOCKS_EXCLUDED(dma_issue_mutex_);

  // Handles request execution completions.
  void HandleExecutionCompletion();

  // Handles instruction queue pop notifications.
  void HandleHostQueueCompletion(uint32 error_code);

  // Checks for HIB Errors.
  Status CheckHibError();

  // Catch all fatal error handling during runtime.
  void CheckFatalError(const Status& status);

  // Registers and enables all interrupts.
  Status RegisterAndEnableAllInterrupts();

  // Pauses all the DMAs and returns once that is verified.
  Status PauseAllDmas() EXCLUSIVE_LOCKS_REQUIRED(state_mutex_);

  // Programs errata CSRs to disable hardware features with known issues.
  Status FixErrata();

  // CSR offsets.
  const config::HibUserCsrOffsets& hib_user_csr_offsets_;
  const config::HibKernelCsrOffsets& hib_kernel_csr_offsets_;

  // Chip structure.
  const config::ChipStructures& chip_structure_;

  // Register interface.
  std::unique_ptr<Registers> registers_;

  // The object responsible for allocating on-chip DRAM buffers (if supported).
  std::unique_ptr<DramAllocator> dram_allocator_;

  // MMU Mapper.
  std::unique_ptr<MmuMapper> mmu_mapper_;

  // Address space management.
  std::unique_ptr<AddressSpace> address_space_;

  // Host buffer allocator.
  std::unique_ptr<Allocator> allocator_;

  // Instruction queue.
  std::unique_ptr<HostQueue<HostQueueDescriptor, HostQueueStatusBlock>>
      instruction_queue_;

  // Interrupt handler.
  std::unique_ptr<InterruptHandler> interrupt_handler_;

  // Top level interrupt manager.
  std::unique_ptr<TopLevelInterruptManager> top_level_interrupt_manager_;

  // Fatal error interrupt controller.
  std::unique_ptr<InterruptControllerInterface>
      fatal_error_interrupt_controller_;

  // Scalar core controller.
  std::unique_ptr<ScalarCoreController> scalar_core_controller_;

  // Run controller.
  std::unique_ptr<RunController> run_controller_;

  // Reset handler.
  std::unique_ptr<TopLevelHandler> top_level_handler_;

  // Maintains integrity of the driver state.
  std::mutex state_mutex_;

  // Ensures that DMAs produced by the dma scheduler is submitted
  // in order to the instruction queue.
  std::mutex dma_issue_mutex_;

  // Driver state.
  State state_ GUARDED_BY(state_mutex_){kClosed};

  // When in state |kClosing|, a notification to wait for all active
  // requests to complete.
  std::condition_variable wait_active_requests_complete_;

  // ID for tracking requests.
  std::atomic<int> next_id_{0};

  // DMA info extractor.
  DmaInfoExtractor dma_info_extractor_;

  // DMA scheduler.
  RealTimeDmaScheduler dma_scheduler_;

  // Chip configuration.
  std::unique_ptr<config::ChipConfig> chip_config_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_MMIO_DRIVER_H_
