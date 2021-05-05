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

#ifndef DARWINN_DRIVER_USB_USB_DRIVER_H_
#define DARWINN_DRIVER_USB_USB_DRIVER_H_

#include <condition_variable>  // NOLINT
#include <cstdint>
#include <memory>
#include <mutex>  // NOLINT
#include <queue>
#include <thread>  // NOLINT

#include "api/buffer.h"
#include "driver/aligned_allocator.h"
#include "driver/config/apex_csr_offsets.h"
#include "driver/config/cb_bridge_csr_offsets.h"
#include "driver/config/chip_config.h"
#include "driver/config/scu_csr_offsets.h"
#include "driver/config/usb_csr_offsets.h"
#include "driver/device_buffer.h"
#include "driver/device_buffer_mapper.h"
#include "driver/dma_chunker.h"
#include "driver/dma_info.h"
#include "driver/dma_info_extractor.h"
#include "driver/driver.h"
#include "driver/interrupt/interrupt_controller_interface.h"
#include "driver/interrupt/top_level_interrupt_manager.h"
#include "driver/memory/dma_direction.h"
#include "driver/memory/dram_allocator.h"
#include "driver/memory/nop_address_space.h"
#include "driver/package_registry.h"
#include "driver/registers/registers.h"
#include "driver/request.h"
#include "driver/run_controller.h"
#include "driver/single_queue_dma_scheduler.h"
#include "driver/top_level_handler.h"
#include "driver/tpu_request.h"
#include "driver/usb/usb_device_interface.h"
#include "driver/usb/usb_dfu_commands.h"
#include "driver/usb/usb_io_request.h"
#include "driver/usb/usb_ml_commands.h"
#include "driver/usb/usb_registers.h"
#include "port/integral_types.h"
#include "port/statusor.h"
#include "port/stringprintf.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// DarwiNN USB driver. Thread safe.
class UsbDriver : public Driver {
 public:
  // Denotes how endpoints are operated.
  enum class OperatingMode {
    // Use independent endpoints to transfer instructions, input activations,
    // and parameters with hardware flow control.
    kMultipleEndpointsHardwareControl = 0,

    // Use independent endpoints to transfer instructions, input activations,
    // and parameters with software flow control.
    kMultipleEndpointsSoftwareQuery = 1,

    // Use single endpoint to transfer instructions, input activations, and
    // parameters with simple hardware flow control.
    kSingleEndpoint = 2,
  };

  // USB driver options.
  struct UsbDriverOptions {
    // USB EP operting mode.
    OperatingMode mode{OperatingMode::kSingleEndpoint};

    // If true, bulk-in data is transmitted in largest chunks possible.
    // By default, driver uses 1KB chunk size for USB3 and 256B for USB2.
    // This is part of workaround for b/73181174
    bool usb_force_largest_bulk_in_chunk_size{false};

    /*
     * There are only 3 modes of operation regarding
     * usb_enable_bulk_out_descriptors_from_device and
     * usb_enable_processing_of_hints:
     *
     * 1) both true, we follow the hints, and
     * use descriptors sent from device as validation. This mode doesn't work
     * if the device sends a lot of bulk-out or bulk-in descriptors out which
     * could clog the descriptor/bulk-in pipeline.
     *
     * 2) disable descriptors but enable hints. We blindly follow the hints
     * and send data to device as fast as we can. The mode is similar to the
     * previous one, but could be slightly faster.
     *
     * 3) enable descriptors but disable the hints. we use descriptors from
     * device and pretend there is no hint from code gen, except for first one
     * (for instructions). This mode doesn't work with multiple instruction
     * chunks, as device is not capable of generating descriptors for
     * instructions.
     *
     */

    // If true, all bulk in/out descriptors are enabled to be sent from
    // device.
    bool usb_enable_bulk_descriptors_from_device{false};

    // If true, all hints from code generator are processed and followed.
    bool usb_enable_processing_of_hints{true};

    // Max number of concurrent async bulk transfers.
    int usb_max_num_async_transfers{kDefaultMaxNumAsyncTransfers};

    // Maximum amount of data to be sent to device in a single bulk out
    // transfer.
    uint32 max_bulk_out_transfer_size_in_bytes{
        kDefaultMaxBulkOutTransferSizeInBytes};

    // Lower limit of credits in software flow control mode.
    uint32 software_credits_lower_limit_in_bytes{
        kDefaultSoftwareCreditsLowerLimitInBytes};

    // If true, the next queued request would be sent to device right after the
    // current request gets into final extraction state. This feature is
    // temporarily fixed at true and cannot be turned off.
    bool usb_enable_overlapping_requests{true};

    // If true, the fence between bulk-out and bulk-in would be lifted, allowing
    // bulk-in to be issued before all bulk-out are finished. This feature could
    // improve performance significantly on Android platform.
    bool usb_enable_overlapping_bulk_in_and_out{true};

    // If true, multiple bulk-in requests would be issued instead of just one at
    // any moment. usb_enable_overlapping_bulk_in_and_out must also be true for
    // this feature to be enabled.
    bool usb_enable_queued_bulk_in_requests{true};

    // If true, driver would fail to open if the current connection is low,
    // full, or high speed. If the connection speed is not observable from
    // underlying provider, this option is ignored.
    bool usb_fail_if_slower_than_superspeed{false};

    // General timeout for USB operations in milliseconds.
    int usb_timeout_millis{6000};

    // If non-empty, the firmware image to use for automatic DFU.
    // This feature is only available when a device factory has been supplied.
    std::vector<uint8> usb_firmware_image;

    // If true, driver would always perform DFU at open.
    // This feature is only available when a device factory has been supplied.
    bool usb_always_dfu{true};

    // Must be packet-size-aligned to avoid buffer overflow during bulk-in,
    // which is 512-byte for USB2 HighSpeed and 1024-byte for USB3 SuperSpeed.
    // TODO: Due to b/77531949, we can only set it to exactly 1024 for
    // USB3 and 256 for USB2 for now.
    size_t usb_bulk_in_max_chunk_size_in_bytes{1024};

    // Max number of buffers to queue.
    int usb_bulk_in_queue_capacity{32};
  };

  // Constructs a device from the factory provided, and performs DFU according
  // to options and discovered device state. Note that DFU requires closing and
  // creating new device instances, and hence can only be achieved by supplying
  // a factory.
  UsbDriver(
      const api::DriverOptions& driver_options,
      std::unique_ptr<config::ChipConfig> chip_config,
      std::function<StatusOr<std::unique_ptr<UsbDeviceInterface>>()>
          device_factory,
      std::unique_ptr<UsbRegisters> registers,
      std::unique_ptr<TopLevelInterruptManager> top_level_interrupt_manager,
      std::unique_ptr<InterruptControllerInterface>
          fatal_error_interrupt_controller,
      std::unique_ptr<TopLevelHandler> top_level_handler,
      std::unique_ptr<DramAllocator> dram_allocator,
      std::unique_ptr<PackageRegistry> executable_registry,
      const UsbDriverOptions& options,
      std::unique_ptr<driver_shared::TimeStamper> time_stamper);

  // Constructs a driver instance around a supplied device object.
  // Note that since no factory is provided, this driver cannot be re-opened
  // after close.
  UsbDriver(
      const api::DriverOptions& driver_options,
      std::unique_ptr<config::ChipConfig> chip_config,
      std::unique_ptr<UsbMlCommands> usb_device,
      std::unique_ptr<UsbRegisters> registers,
      std::unique_ptr<TopLevelInterruptManager> top_level_interrupt_manager,
      std::unique_ptr<InterruptControllerInterface>
          fatal_error_interrupt_controller,
      std::unique_ptr<TopLevelHandler> top_level_handler,
      std::unique_ptr<DramAllocator> dram_allocator,
      std::unique_ptr<PackageRegistry> executable_registry,
      const UsbDriverOptions& options,
      std::unique_ptr<driver_shared::TimeStamper> time_stamper);

  // This class is neither copyable nor movable.
  UsbDriver(const UsbDriver&) = delete;
  UsbDriver& operator=(const UsbDriver&) = delete;

  ~UsbDriver() override;

  uint64_t allocation_alignment_bytes() const override {
    return chip_config_->GetChipStructures().allocation_alignment_bytes;
  }

 protected:
  Status DoOpen(bool debug_mode) LOCKS_EXCLUDED(mutex_) final;
  Status DoClose(bool in_error, api::Driver::ClosingMode mode)
      LOCKS_EXCLUDED(mutex_) final;
  Status DoCancelAndWaitRequests(bool in_error) LOCKS_EXCLUDED(mutex_) final;

  Buffer DoMakeBuffer(size_t size_bytes) const final;
  StatusOr<MappedDeviceBuffer> DoMapBuffer(const Buffer& buffer,
                                           DmaDirection direction) final;
  StatusOr<std::shared_ptr<TpuRequest>> DoCreateRequest(
      const std::shared_ptr<Request> parent_request,
      const ExecutableReference* executable_ref, TpuRequest::RequestType type)
      LOCKS_EXCLUDED(mutex_) final;

  Status DoSetExecutableTiming(const ExecutableReference* executable,
                               const api::Timing& timing) final;
  Status DoSetRealtimeMode(bool on) final;

  Status DoSubmit(std::shared_ptr<TpuRequest> request_in)
      LOCKS_EXCLUDED(mutex_) final;

  int64 MaxRemainingCycles() const override {
    return dma_scheduler_.MaxRemainingCycles();
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
    kPaused,   // Device has been paused.
    kClosing,  // Driver is Closing.
    kClosed,   // Driver is Closed. (Initial state.)
  };

  // Record information for a filled bulk-in buffer.
  struct FilledBulkInInfo {
    int buffer_index;
    size_t begin_offset;
    size_t end_offset;
  };

  // Default value for #usb_max_num_async_transfers, if not set by the client.
  static constexpr int kDefaultMaxNumAsyncTransfers = 3;

  // Default value for #max_bulk_out_transfer_size_in_bytes, if not set by the
  // client.
  static constexpr uint32 kDefaultMaxBulkOutTransferSizeInBytes = 1024 * 1024;

  // Default value for #software_credits_lower_limit_in_bytes, if not set by the
  // client.
  static constexpr uint32 kDefaultSoftwareCreditsLowerLimitInBytes = 8 * 1024;

  // Constructor to be used as delegate target.
  UsbDriver(
      const api::DriverOptions& driver_options,
      std::unique_ptr<config::ChipConfig> chip_config,
      std::unique_ptr<UsbRegisters> registers,
      std::unique_ptr<TopLevelInterruptManager> top_level_interrupt_manager,
      std::unique_ptr<InterruptControllerInterface>
          fatal_error_interrupt_controller,
      std::unique_ptr<TopLevelHandler> top_level_handler,
      std::unique_ptr<DramAllocator> dram_allocator,
      std::unique_ptr<PackageRegistry> executable_registry,
      const UsbDriverOptions& options,
      std::unique_ptr<driver_shared::TimeStamper> time_stamper);

  // Prepares USB device with resets and DFU according to options_.
  Status PrepareUsbDevice();

  // Creates a UsbMlCommands and assigns it to usb_device_, with timed retry.
  Status OpenMlUsbDevice();

  // Creates a raw USB device from device_factory_, with timed retry.
  StatusOr<std::unique_ptr<UsbDeviceInterface>> CreateRawUsbDeviceWithRetry();

  // Attempts a state transition to the given state.
  Status SetState(State next_state) EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Validates that we are in the expected state.
  Status ValidateState(State expected_state) const
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Validates that we are in any of the expected states.
  Status ValidateStates(const std::vector<State>& expected_states) const
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Catches all fatal error handling during runtime.
  void CheckFatalError(const Status& status);

  // Initializes the chip through CSR access.
  Status InitializeChip() EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Registers and enables all interrupts that come through interrupt
  // endpoint.
  Status RegisterAndEnableAllInterrupts();

  // Disables all interrupts that come through interrupt endpoint.
  Status DisableAllInterrupts();

  // Runs the worker thread.
  void WorkerThreadFunc() LOCKS_EXCLUDED(mutex_);

  // Handles bulk-in completion event from device.
  void HandleQueuedBulkIn(const Status& status, int buffer_index,
                          size_t num_bytes_transferred);

  // Handles data in/out and software interrupt events sent from the device,
  // in the worker thread. Thread safety analysis is confused by wrapping
  // this function into a functor, and hence has to be disabled.
  void HandleEvent(const Status& status,
                   const UsbMlCommands::EventDescriptor& event_info)
      NO_THREAD_SAFETY_ANALYSIS;

  // Handles hardware interrupt events sent from the device, in the
  // worker thread. Thread safety analysis is confused by wrapping this
  // function into a functor, and hence has to be disabled.
  void HandleInterrupt(const Status& status,
                       const UsbMlCommands::InterruptInfo& interrupt_info)
      NO_THREAD_SAFETY_ANALYSIS;

  // Retrieves credits for the endpoint specified with tag.
  uint32_t GetCredits(UsbMlCommands::DescriptorTag tag)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Processes data in/out requests associated with specified task.
  StatusOr<bool> ProcessIo() EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Records DMA descriptors and in-band interrupts sent from device.
  Status HandleDmaDescriptor(UsbMlCommands::DescriptorTag tag,
                             uint64_t device_virtual_address,
                             uint32_t size_bytes, bool bulk_events_enabled);

  Status CheckHibError();

  std::function<StatusOr<std::unique_ptr<UsbDeviceInterface>>()>
      device_factory_;

  // The current active USB device supporting ML commands.
  std::unique_ptr<UsbMlCommands> usb_device_;

  // CSR offsets.
  std::unique_ptr<config::ChipConfig> chip_config_;

  // Implements CSR access.
  std::unique_ptr<UsbRegisters> registers_;

  // Buffer management.
  // TODO: Allocate zero copy USB buffers.
  std::unique_ptr<Allocator> allocator_;

  // Protects access to callback queue, resource shared by the worker thread
  // and callbacks from usb device.
  mutable std::mutex callback_mutex_;

  // Stores functors submitted by callbacks, to be executed in work thread.
  std::queue<std::function<void()>> callback_queue_ GUARDED_BY(callback_mutex_);

  // Maintains integrity of the driver state.
  mutable std::mutex mutex_;

  // Driver state.
  State state_ GUARDED_BY(mutex_){kClosed};

  // Conditional variable for worker thread to wait on events from both
  // application layer and callbacks from usb devices.
  std::condition_variable_any driver_state_changed_;

  // Worker thread object.
  std::thread worker_thread_;

  // ID for tracking requests.
  int next_id_ GUARDED_BY(mutex_){0};

  // Top level interrupt controller.
  std::unique_ptr<TopLevelInterruptManager> top_level_interrupt_manager_;

  // Fatal error interrupt controller.
  std::unique_ptr<InterruptControllerInterface>
      fatal_error_interrupt_controller_;

  // Implements device run control.
  std::unique_ptr<RunController> run_controller_;

  // Implements reset handler.
  std::unique_ptr<TopLevelHandler> top_level_handler_;

  // Enables driver to allocate buffer in On-Chip DRAM (if any).
  std::unique_ptr<DramAllocator> dram_allocator_;

  // Address space management.
  NopAddressSpace address_space_;

  // Driver options.
  UsbDriverOptions options_;

  // DMA info extractor.
  DmaInfoExtractor dma_info_extractor_;

  // DMA scheduler.
  SingleQueueDmaScheduler dma_scheduler_;

  // list is used instead of vector, for iterators/pointers must be kept stable
  // through out async callbacks.
  std::list<UsbIoRequest> io_requests_;

  // If true, limit every bulk-in request to be at most 256-byte long.
  // This is part of workaround for b/73181174
  bool cap_bulk_in_size_at_256_bytes_{false};

  // Container for all bulk-in buffers.
  std::vector<Buffer> bulk_in_buffers_;

  // Container for indices for bulk-in buffers that are not queued for data in.
  // Note the reason for using queue here is for easier log interpretation.
  std::queue<int> available_bulk_in_buffers_;

  // Container for indices for bulk-in buffers that contain data from device.
  std::queue<FilledBulkInInfo> filled_bulk_in_buffers_;

  // CSR offsets.
  const config::ApexCsrOffsets& apex_csr_offsets_;
  const config::CbBridgeCsrOffsets& cb_bridge_csr_offsets_;
  const config::HibKernelCsrOffsets& hib_kernel_csr_offsets_;
  const config::ScuCsrOffsets& scu_csr_offsets_;
  const config::UsbCsrOffsets& usb_csr_offsets_;
  const config::HibUserCsrOffsets& hib_user_csr_offsets_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_USB_USB_DRIVER_H_
