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

#include "driver/usb/usb_driver.h"

#include <bitset>
#include <functional>
#include <queue>
#include <utility>
#include <vector>

#include "api/buffer.h"
#include "api/watchdog.h"
#include "driver/device_buffer_mapper.h"
#include "driver/dma_info_extractor.h"
#include "driver/hardware_structures.h"
#include "driver/interrupt/interrupt_controller_interface.h"
#include "driver/interrupt/top_level_interrupt_manager.h"
#include "driver/memory/address_utilities.h"
#include "driver/memory/dram_allocator.h"
#include "driver/package_registry.h"
#include "driver/single_tpu_request.h"
#include "driver/top_level_handler.h"
#include "driver/tpu_request.h"
#include "driver/usb/usb_dfu_util.h"
#include "driver/usb/usb_latest_firmware.h"
#include "driver/usb/usb_ml_commands.h"
#include "driver_shared/time_stamper/time_stamper.h"
#include "port/cleanup.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/logging.h"
#include "port/ptr_util.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/statusor.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"
#include "port/time.h"
#include "port/tracing.h"
#include "port/unreachable.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace {

// Sleep time before we try or retry to open a device.
// TODO: revisit this setting after we finalize PHY tuning.
constexpr int kSleepTimeMicroSecondsBeforeRetry = 1000000;

// TODO: revisit this setting after we finalize PHY tuning.
constexpr int kMaxNumOfRetryAfterReset = 25;

constexpr uint16_t kTargetAppVendorId = 0x18D1;
constexpr uint16_t kTargetAppProductId = 0x9302;

constexpr uint16_t kTargetDfuVendorId = 0x1A6E;
constexpr uint16_t kTargetDfuProductId = 0x089A;

// This class implements BasicLockable concept, to be used with
// std::conditional_variable_any.
// The implementation is specialized as no re-locking is needed.
// Since the conditional variable is used in the end of a do-while loop,
// re-locking is just waste of time.
class Lock2 {
 public:
  Lock2(StdCondMutexLock& m1, StdCondMutexLock& m2) : m1_(m1), m2_(m2) {}

  ~Lock2() = default;

  // Does nothing. This function is part of BasicLockable concept.
  void lock() {
    // do nothing.
    VLOG(10) << "lock (does nothing)";
  }

  // Unlocks both Lockables. This function is part of BasicLockable concept.
  void unlock() {
    VLOG(10) << "Unlocks both mutex";
    m1_.unlock();
    m2_.unlock();
  }

 private:
  StdCondMutexLock& m1_;
  StdCondMutexLock& m2_;
};

// Returns the number of entries in a queue concept, protected by the given
// mutex.
template <typename Queue>
static typename Queue::size_type QueueSize(const Queue* queue,
                                           std::mutex* mutex) {
  StdCondMutexLock state_lock(mutex);
  return queue->size();
}

// Returns if a queue concept is empty, protected by the given mutex.
template <typename Queue>
static bool IsQueueEmpty(const Queue* queue, std::mutex* mutex) {
  StdCondMutexLock state_lock(mutex);
  return queue->empty();
}

// Returns the first entry in a queue concept, protected by the given mutex.
template <typename Queue>
static typename Queue::value_type QueuePop(Queue* queue, std::mutex* mutex) {
  StdCondMutexLock state_lock(mutex);
  typename Queue::value_type item = queue->front();
  queue->pop();
  return item;
}

}  // namespace

UsbDriver::UsbDriver(
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
    std::unique_ptr<driver_shared::TimeStamper> time_stamper)
    : Driver(
          [](config::ChipConfig* chip_config) {
            CHECK(chip_config != nullptr);
            return chip_config->GetChip();
          }(chip_config.get()),
          std::move(executable_registry), driver_options,
          std::move(time_stamper)),
      chip_config_(std::move(chip_config)),
      registers_(std::move(registers)),
      allocator_(gtl::MakeUnique<AlignedAllocator>(
          chip_config_->GetChipStructures().allocation_alignment_bytes)),
      top_level_interrupt_manager_(std::move(top_level_interrupt_manager)),
      fatal_error_interrupt_controller_(
          std::move(fatal_error_interrupt_controller)),
      top_level_handler_(std::move(top_level_handler)),
      dram_allocator_(std::move(dram_allocator)),
      options_(options),
      dma_info_extractor_(
          options.usb_enable_processing_of_hints
              ? DmaInfoExtractor::ExtractorType::kDmaHints
              : DmaInfoExtractor::ExtractorType::kFirstInstruction,
          options.usb_enable_overlapping_requests),
      dma_scheduler_(api::Watchdog::MakeWatchdog(
          driver_options.watchdog_timeout_ns(),
          [this](int64) { HandleWatchdogTimeout(); })),
      apex_csr_offsets_(chip_config_->GetApexCsrOffsets()),
      cb_bridge_csr_offsets_(chip_config_->GetCbBridgeCsrOffsets()),
      hib_kernel_csr_offsets_(chip_config_->GetHibKernelCsrOffsets()),
      scu_csr_offsets_(chip_config_->GetScuCsrOffsets()),
      usb_csr_offsets_(chip_config_->GetUsbCsrOffsets()),
      hib_user_csr_offsets_(chip_config_->GetHibUserCsrOffsets()) {
  run_controller_ =
      gtl::MakeUnique<RunController>(*chip_config_, registers_.get());

  if (options_.mode == OperatingMode::kMultipleEndpointsSoftwareQuery) {
    options_.usb_max_num_async_transfers = 1;
    VLOG(5) << StringPrintf(
        "force setting usb_max_num_async_transfers to 1 for software "
        "query mode");
  }
}

UsbDriver::UsbDriver(
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
    std::unique_ptr<driver_shared::TimeStamper> time_stamper)
    : UsbDriver(driver_options, std::move(chip_config), std::move(registers),
                std::move(top_level_interrupt_manager),
                std::move(fatal_error_interrupt_controller),
                std::move(top_level_handler), std::move(dram_allocator),
                std::move(executable_registry), options,
                std::move(time_stamper)) {
  usb_device_ = std::move(usb_device);
}

UsbDriver::UsbDriver(
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
    std::unique_ptr<driver_shared::TimeStamper> time_stamper)
    : UsbDriver(driver_options, std::move(chip_config), std::move(registers),
                std::move(top_level_interrupt_manager),
                std::move(fatal_error_interrupt_controller),
                std::move(top_level_handler), std::move(dram_allocator),
                std::move(executable_registry), options,
                std::move(time_stamper)) {
  device_factory_ = std::move(device_factory);
}

UsbDriver::~UsbDriver() {
  CHECK_OK(UnregisterAll());
  if (Close(api::Driver::ClosingMode::kGraceful).ok()) {
    LOG(WARNING) << "Driver destroyed when open. Forced Close().";
  }
}

Status UsbDriver::ValidateState(State expected_state) const {
  return ValidateStates({expected_state});
}

Status UsbDriver::ValidateStates(
    const std::vector<State>& expected_states) const {
  for (auto& state : expected_states) {
    if (state_ == state) {
      return Status();  // OK
    }
  }

  return FailedPreconditionError(StringPrintf("Unexpected state %d.", state_));
}

Status UsbDriver::SetState(State next_state) {
  driver_state_changed_.notify_all();

  if ((next_state == kClosing) || (next_state == kPaused)) {
    // Cancel all transfers when we enter closing or paused state.
    //
    // Cancellation generates new callbacks with canceled status which need to
    // be handled for each transfer that is still active. Pointers to task
    // records and hence bulk in/out requests could already been invalidated.
    usb_device_->TryCancelAllTransfers();
  }

  switch (state_) {
    case kOpen:
      if ((next_state == kOpen) || (next_state == kClosing)) {
        // There is nothing special to do.
        state_ = next_state;
        return Status();  // OK
      } else if (next_state == kPaused) {
        VLOG(7) << StringPrintf("%s try enable clock gating", __func__);
        RETURN_IF_ERROR(top_level_handler_->EnableSoftwareClockGate());

        state_ = next_state;
        return Status();  // OK
      }
      break;

    case kPaused:
      if (next_state == kPaused) {
        // We're already paused. Do nothing.
        return Status();  // OK
      } else if ((next_state == kOpen) || (next_state == kClosing)) {
        // Disable clock gating so we can access the chip.
        VLOG(7) << StringPrintf("%s try disable clock gating", __func__);
        RETURN_IF_ERROR(top_level_handler_->DisableSoftwareClockGate());

        state_ = next_state;
        return Status();  // OK
      }
      break;

    case kClosing:
      if (next_state == kClosed) {
        state_ = next_state;
        return Status();  // OK
      }
      break;

    case kClosed:
      if (next_state == kOpen) {
        state_ = next_state;
        return Status();  // OK
      }
      break;
  }

  // Illegal state transition.
  return FailedPreconditionError(StringPrintf(
      "Invalid state transition. current=%d, next=%d.", state_, next_state));
}

// TODO: review the sequence with hardware team and convert them to
// use named constants.
Status UsbDriver::InitializeChip() {
  TRACE_SCOPE("UsbDriver::InitializeChip");

  ASSIGN_OR_RETURN(auto omc_reg, registers_->Read32(apex_csr_offsets_.omc0_00));
  constexpr int EFUSE_PROGRAMMING_REVISION_SHIFT = 24;
  constexpr int EFUSE_PROGRAMMING_REVISION_MASK = 0xFF;
  const uint8_t efuse_programming_revision =
      (omc_reg >> EFUSE_PROGRAMMING_REVISION_SHIFT) &
      EFUSE_PROGRAMMING_REVISION_MASK;
  VLOG(1) << StringPrintf("e-fuse programming revision: %d",
                          efuse_programming_revision);

  if (options_.usb_enable_bulk_descriptors_from_device) {
    VLOG(7) << StringPrintf("%s Enabling all descriptors", __func__);
    RETURN_IF_ERROR(registers_->Write(usb_csr_offsets_.descr_ep, 0xFF));
  } else {
    VLOG(7) << StringPrintf("%s Enabling only sc host interrupt descriptors",
                            __func__);
    RETURN_IF_ERROR(registers_->Write(usb_csr_offsets_.descr_ep, 0xF0));
  }

  switch (options_.mode) {
    case OperatingMode::kMultipleEndpointsHardwareControl:
    case OperatingMode::kMultipleEndpointsSoftwareQuery:
      VLOG(7) << StringPrintf("%s Enabling multiple EP mode", __func__);
      RETURN_IF_ERROR(registers_->Write(usb_csr_offsets_.multi_bo_ep, 1));
      break;

    case OperatingMode::kSingleEndpoint:
      VLOG(7) << StringPrintf("%s Enabling single EP mode", __func__);
      RETURN_IF_ERROR(registers_->Write(usb_csr_offsets_.multi_bo_ep, 0));
      break;

    default:
      return FailedPreconditionError("Unrecognized USB operating mode");
  }

  if ((!options_.usb_force_largest_bulk_in_chunk_size) &&
      (usb_device_->GetDeviceSpeed() ==
       UsbStandardCommands::DeviceSpeed::kHigh)) {
    // If we know it's USB2 Highspeed (max bulk packet size 512B), and there is
    // no option to force max chunk size, use 256B chunk size to limit packet
    // length to 256B. This is a workaround for b/73181174
    VLOG(7) << StringPrintf("%s Setting 256B chunk for USB 2 High Speed",
                            __func__);

    // This is an optimiaztion for some host controllers, so everyone knows the
    // response would be only 256 bytes long. Without this, host would need to
    // identify the response as a "short" packet and hence ends transfer. Not
    // all host controllers need this change.
    cap_bulk_in_size_at_256_bytes_ = true;

    RETURN_IF_ERROR(
        registers_->Write(usb_csr_offsets_.outfeed_chunk_length, 0x20));
  } else {
    // Otherwise, use the largest chunk size (1KB) for max bulk packet size
    // determined by the hardware.
    VLOG(7) << StringPrintf("%s Setting 1KB chunk for bulk-ins", __func__);

    cap_bulk_in_size_at_256_bytes_ = false;
    RETURN_IF_ERROR(
        registers_->Write(usb_csr_offsets_.outfeed_chunk_length, 0x80));
  }

  return Status();  // OK.
}

Status UsbDriver::RegisterAndEnableAllInterrupts() {
  // TODO: Register interrupts to interrupt EP.
  RETURN_IF_ERROR(fatal_error_interrupt_controller_->EnableInterrupts());
  RETURN_IF_ERROR(top_level_interrupt_manager_->EnableInterrupts());

  return Status();  // OK
}

Status UsbDriver::DisableAllInterrupts() {
  RETURN_IF_ERROR(top_level_interrupt_manager_->DisableInterrupts());
  RETURN_IF_ERROR(fatal_error_interrupt_controller_->DisableInterrupts());

  return Status();  // OK
}

void UsbDriver::HandleEvent(const Status& status,
                            const UsbMlCommands::EventDescriptor& event_info) {
  if (status.ok()) {
    // TODO: analyze if there is any failure case we can recover from.
    CHECK_OK(HandleDmaDescriptor(
        event_info.tag, event_info.offset, event_info.length,
        options_.usb_enable_bulk_descriptors_from_device));
  } else if (IsDeadlineExceeded(status)) {
    VLOG(10) << StringPrintf("%s timed out, ignore.", __func__);
  } else if (IsCancelled(status)) {
    VLOG(10) << StringPrintf("%s cancelled, ignore.", __func__);
  } else {
    LOG(FATAL) << StringPrintf("%s failed. %s", __func__,
                               status.error_message().c_str());
  }
}

Status UsbDriver::CheckHibError() {
  // Indicates no HIB Fatal Error.
  constexpr uint64 kHibErrorStatusNone = 0;

  ASSIGN_OR_RETURN(uint64 hib_error_status,
                   registers_->Read(hib_user_csr_offsets_.hib_error_status));
  if (hib_error_status == kHibErrorStatusNone) {
    return Status();  // OK
  }

  ASSIGN_OR_RETURN(
      uint64 hib_first_error_status,
      registers_->Read(hib_user_csr_offsets_.hib_first_error_status));

  auto error_string = StringPrintf(
      "HIB Error. hib_error_status = %016llx, hib_first_error_status = %016llx",
      static_cast<unsigned long long>(hib_error_status),  // NOLINT(runtime/int)
      static_cast<unsigned long long>(                    // NOLINT(runtime/int)
          hib_first_error_status));
  LOG(ERROR) << error_string;
  return InternalError(error_string);
}

void UsbDriver::HandleInterrupt(
    const Status& status, const UsbMlCommands::InterruptInfo& interrupt_info) {
  if (status.ok()) {
    VLOG(10) << StringPrintf("%s interrupt received.", __func__);

    constexpr uint32_t kFatalErrorInterruptMask = 1;
    constexpr int kTopLevelInterruptBitShift = 1;
    const uint32_t kTopLevelInterruptMask =
        ((1 << top_level_interrupt_manager_->NumInterrupts()) - 1)
        << kTopLevelInterruptBitShift;
    if (interrupt_info.raw_data & kFatalErrorInterruptMask) {
      VLOG(1) << StringPrintf("%s Fatal error interrupt received.", __func__);
      CHECK_OK(CheckHibError());
      CHECK_OK(fatal_error_interrupt_controller_->ClearInterruptStatus(0));
    }
    if ((interrupt_info.raw_data & kTopLevelInterruptMask) != 0) {
      uint32_t top_level_interrupts = static_cast<uint32_t>(
          (interrupt_info.raw_data & kTopLevelInterruptMask) >>
          kTopLevelInterruptBitShift);

      for (int id = 0; id < top_level_interrupt_manager_->NumInterrupts();
           ++id) {
        const uint32_t mask = 1 << id;
        if ((top_level_interrupts & mask) == mask) {
          VLOG(1) << StringPrintf("%s Top level interrupt %d received.",
                                  __func__, id);
          CHECK_OK(top_level_interrupt_manager_->HandleInterrupt(id));
        }
      }
    }

  } else if (IsCancelled(status)) {
    VLOG(10) << StringPrintf("%s cancelled, ignore.", __func__);
  } else {
    VLOG(1) << status.message();
  }
}

uint32_t UsbDriver::GetCredits(UsbMlCommands::DescriptorTag tag) {
  if (!registers_->Write32(apex_csr_offsets_.omc0_00, 0xffffffff).ok()) {
    VLOG(1) << StringPrintf("%s write failed. silently assume 0 credit",
                            __func__);
    return 0;
  }

  auto query_result = registers_->Read(usb_csr_offsets_.ep_status_credit);
  if (!query_result.status().ok()) {
    VLOG(1) << StringPrintf("%s read failed. silently assume 0 credit",
                            __func__);
    return 0;
  }
  const uint64_t gcb_credits = query_result.ValueOrDie();

  constexpr uint32_t kCounterInBytes = 8;
  constexpr uint32_t kCreditShift = 21;
  constexpr uint32_t kCreditMask = (1ULL << kCreditShift) - 1;

  const uint32_t instructions =
      static_cast<uint32_t>((gcb_credits & kCreditMask) * kCounterInBytes);
  const uint32_t input_activations = static_cast<uint32_t>(
      ((gcb_credits >> kCreditShift) & kCreditMask) * kCounterInBytes);
  const uint32_t parameters = static_cast<uint32_t>(
      ((gcb_credits >> (kCreditShift * 2)) & kCreditMask) * kCounterInBytes);

  VLOG(10) << StringPrintf("%s credits: instructions %u, input %u, params %u",
                           __func__, instructions, input_activations,
                           parameters);

  switch (tag) {
    case UsbMlCommands::DescriptorTag::kInstructions:
      return instructions;
    case UsbMlCommands::DescriptorTag::kInputActivations:
      return input_activations;
    case UsbMlCommands::DescriptorTag::kParameters:
      return parameters;
    default:
      LOG(FATAL) << StringPrintf("%s unrecognized tag", __func__);
      unreachable();  // NOLINT
  }
}

// TODO: breaks up this function according to functionality.
StatusOr<bool> UsbDriver::ProcessIo() {
  TRACE_SCOPE("UsbDriver::ProcessIO");
  static constexpr int kNumBulkOutTags = 3;
  static constexpr uint8_t tag_to_bulk_out_endpoint_id[kNumBulkOutTags] = {
      UsbMlCommands::kInstructionsEndpoint,
      UsbMlCommands::kInputActivationsEndpoint,
      UsbMlCommands::kParametersEndpoint};
  int num_active_transfers = 0;
  std::bitset<kNumBulkOutTags> tag_to_bulk_out_with_unsent_chunk;

  // Remove UsbIoRequest that are completed.
  while (!io_requests_.empty()) {
    const auto& io_request = io_requests_.front();
    if (!io_request.IsCompleted()) {
      break;
    }

    // If DMA descriptors are coming in, and hint is not yet matched. Consider
    // it not completed.
    if (options_.usb_enable_bulk_descriptors_from_device &&
        io_request.GetSourceAndMatchStatus() ==
            UsbIoRequest::SourceAndMatchStatus::kHintNotYetMatched) {
      break;
    }

    if (io_request.FromDmaHint()) {
      CHECK_OK(dma_scheduler_.NotifyDmaCompletion(io_request.dma_info()));
    }

    if (io_request.GetTag() == UsbMlCommands::DescriptorTag::kInterrupt0) {
      TRACE_WITHIN_SCOPE("UsbDriver::ProcessIO::RequestCompletion");
      CHECK_OK(dma_scheduler_.NotifyRequestCompletion());
      HandleTpuRequestCompletion();
    }
    VLOG(9) << "IO completed";
    io_requests_.pop_front();
  }

  // TODO: Remove this loop.
  // As an intermediate step, IO requests are completely pulled out from a
  // Request. Eventually, We should GetNextDma() only when we can perform DMA.
  ASSIGN_OR_RETURN(auto* dma_info, dma_scheduler_.GetNextDma());
  while (dma_info) {
    io_requests_.push_back(UsbIoRequest(dma_info));
    ASSIGN_OR_RETURN(dma_info, dma_scheduler_.GetNextDma());
  }

  // True if some libusb command has been issued and we should skip waiting on
  // the completion queue.
  bool is_task_state_changed = false;

  // All previous bulk out requests must be completed before a bulk in, and
  // interrupt 0 request can be processed.
  bool is_any_bulk_out_still_uncompleted = false;
  bool is_any_bulk_in_still_uncompleted = false;

  for (auto& io_request : io_requests_) {
    if (io_request.IsCompleted()) {
      continue;
    }

    if (io_request.GetTag() == UsbMlCommands::DescriptorTag::kInterrupt0) {
      // Nothing to do for interrupts.
      continue;
    }

    auto io_type = io_request.GetType();
    const int tag = static_cast<int>(io_request.GetTag());
    if (io_type == UsbIoRequest::Type::kBulkOut) {
      // block further processing of any bulk in requests.
      is_any_bulk_out_still_uncompleted = true;

      if (io_request.IsActive()) {
        // simply increase the counter and proceed to see if we can fire another
        // request for the next chunk.
        num_active_transfers += io_request.GetActiveCounts(
            options_.max_bulk_out_transfer_size_in_bytes);
      } else {
        if (options_.mode == OperatingMode::kMultipleEndpointsHardwareControl) {
          // In multiple-ep hardware control mode, let's continue
          // searching for a different tag to be sent out. It's never okay to
          // interleve/mix chunks from different requests of the same tag.
          if (tag_to_bulk_out_with_unsent_chunk[static_cast<int>(
                  UsbMlCommands::DescriptorTag::kInstructions)]) {
            // If there is any uncompleted instructions, break from the search.
            break;
          } else if (tag_to_bulk_out_with_unsent_chunk.count() ==
                     (kNumBulkOutTags - 1)) {
            // If all endpoints(tags) supported, other than instructions, are
            // busy, break from the search. If instructions endpoint is busy, we
            // already break from previous clause.
            break;
          } else if (tag_to_bulk_out_with_unsent_chunk[tag]) {
            // If something sharing with my endpoint is busy, keep looking for
            // something different.
            continue;
          }
        } else {
          if (tag_to_bulk_out_with_unsent_chunk.any()) {
            // In other modes, especially for single-ep mode, continue searching
            // is not necessary if any previous request has unsent chunk data
            // waiting. This is because we only send out header once for each
            // request. Note that it's okay to start sending the next chunk if
            // the request is already active, as they share the same header.
            // Also, it's okay to start processing a new request after the
            // previous request has all its data pushed into the pipeline.
            break;
          }
        }
      }

      if (is_any_bulk_in_still_uncompleted) {
        // Prevent any queuing of bulk-out after bulk-in, in single ep mode.
        // It's not very safe to allow bulk-out after bulk-in in single ep mode,
        // as the bulk-out could hog the internal data path and prevent bulk-in
        // from completion. In multiple ep mode, the internal data path cannot
        // be occupied for long time, and hence it's safe to queue any request.
        if (options_.mode == OperatingMode::kSingleEndpoint) {
          // Due to hardware limitation, bulk-out could delay completion of
          // bulk-in till deadlock occurs.
          VLOG(10) << StringPrintf(
              "[%d-%d] all bulk in requests must be completed before "
              "processing of bulk out can start, wait",
              io_request.id(), tag);
          break;
        }
      } else if (num_active_transfers >= options_.usb_max_num_async_transfers) {
        VLOG(10) << StringPrintf(
            "[%d-%d] number of concurrent transfers too high, wait "
            "(%d >= %d)",
            io_request.id(), tag, num_active_transfers,
            options_.usb_max_num_async_transfers);
        break;
      }

      if (!io_request.HasNextChunk()) {
        // There is nothing we can do for this request. All data has been put
        // in transit.
        continue;
      }

      if (options_.mode == OperatingMode::kMultipleEndpointsSoftwareQuery) {
        // TODO: add some mechansim to slowly poll for available
        // credits.
        // Setting this to true would cause unpleasant busy looping.
        is_task_state_changed = true;

        // query for credits available.
        uint32_t credits = GetCredits(io_request.GetTag());
        // proceed only if the credits are above some threashold.
        if (credits <= options_.software_credits_lower_limit_in_bytes) {
          VLOG(10) << StringPrintf(
              "[%d-%d] available credits too low, wait (%u <= %u)",
              io_request.id(), tag, credits,
              options_.software_credits_lower_limit_in_bytes);

          // Stop further processing if credit for any endpoint is lower than
          // the limit.
          // TODO: allow a different endpoint to proceed.
          break;
        }

        // Clamp the transfer size with available credits.
        uint32_t transfer_size =
            std::min(options_.max_bulk_out_transfer_size_in_bytes, credits);

        const auto device_buffer = io_request.GetNextChunk(transfer_size);
        auto host_buffer = address_space_.Translate(device_buffer).ValueOrDie();
        UsbMlCommands::ConstBuffer transfer_buffer(host_buffer.ptr(),
                                                   host_buffer.size_bytes());

        ++num_active_transfers;
        if (io_request.HasNextChunk()) {
          // This request still has some data not sent over to the pipeline.
          // Setting this to true prevents rquests of the same tag to start, as
          // chunks from different requests of the same tag must not interleve.
          tag_to_bulk_out_with_unsent_chunk[tag] = true;
        }

        // To make sure the query result for credits is accurate, we have to
        // use sync transfer. Because we only send data according to credits
        // available, there is no way we could get a timeout error.
        Status status = usb_device_->BulkOutTransfer(
            tag_to_bulk_out_endpoint_id[tag], transfer_buffer, __func__);
        if (status.ok()) {
          io_request.NotifyTransferComplete(transfer_size);
          VLOG(10) << StringPrintf("[%d-%d] bulk out for %u bytes done",
                                   io_request.id(), tag, transfer_size);
        } else {
          // TODO: terminate the task early, as there is no
          // chance we can continue. The more reasonable next step would
          // be resetting the device.
          LOG(FATAL) << StringPrintf(
              "[%d-%d] bulk out for %u bytes failed. Abort. %s",
              io_request.id(), tag, transfer_size, status.ToString().c_str());
        }
      } else if (options_.mode ==
                 OperatingMode::kMultipleEndpointsHardwareControl) {
        is_task_state_changed = true;

        const auto device_buffer = io_request.GetNextChunk(
            options_.max_bulk_out_transfer_size_in_bytes);
        auto host_buffer = address_space_.Translate(device_buffer).ValueOrDie();
        UsbMlCommands::ConstBuffer transfer_buffer(host_buffer.ptr(),
                                                   host_buffer.size_bytes());
        uint32_t transfer_size = static_cast<uint32_t>(transfer_buffer.size());

        ++num_active_transfers;
        if (io_request.HasNextChunk()) {
          // This request still has some data not sent over to the pipeline.
          // Setting this to true prevents rquests of the same tag to start, as
          // chunks from different requests of the same tag must not interleve.
          tag_to_bulk_out_with_unsent_chunk[tag] = true;
        }

        Status async_request_status = usb_device_->AsyncBulkOutTransfer(
            tag_to_bulk_out_endpoint_id[tag], transfer_buffer,
            [this, &io_request, tag, transfer_size](Status status) {
              // Inject a functor into a completion queue driven by the worker
              // thread. Note that the reference to io_request could have been
              // invalidated when the async transfer is cancelled.
              StdMutexLock queue_lock(&callback_mutex_);
              callback_queue_.push([&io_request, tag, status, transfer_size] {
                // Starting from here is an functor which would be executed
                // within the worker thread context, after the async transfer
                // has been completed.
                if (status.ok()) {
                  io_request.NotifyTransferComplete(transfer_size);
                  VLOG(10) << StringPrintf("[%d-%d] bulk out for %u bytes done",
                                           io_request.id(), tag, transfer_size);
                } else {
                  // TODO: terminate the task early, as there is no
                  // chance we can continue. The more reasonable next step
                  // would be resetting the device.
                  LOG(FATAL) << StringPrintf(
                      "[%d-%d] bulk out failed. Abort. %s", io_request.id(),
                      tag, status.ToString().c_str());
                }
              });
              driver_state_changed_.notify_all();
            },
            __func__);

        if (!async_request_status.ok()) {
          // TODO: terminate the task early, as there is no
          // chance we can continue. The more reasonable next step would
          // be resetting the device.
          LOG(FATAL) << StringPrintf(
              "[%d-%d] async transfer out for %u bytes failed. Abort. %s",
              io_request.id(), tag, transfer_size,
              async_request_status.ToString().c_str());
        }
      } else if (options_.mode == OperatingMode::kSingleEndpoint) {
        is_task_state_changed = true;

        if (!io_request.IsActive() && !io_request.IsCompleted() &&
            !io_request.IsHeaderSent()) {
          // Prepare the header with full data size.
          // add one extra count for the header transfer.
          ++num_active_transfers;

          VLOG(10) << StringPrintf("%s [%d-%d] bulk out header", __func__,
                                   io_request.id(), tag);

          io_request.SetHeader(usb_device_->PrepareHeader(
              io_request.GetTag(), io_request.GetBuffer().size_bytes()));

          Status async_request_status = usb_device_->AsyncBulkOutTransfer(
              UsbMlCommands::kSingleBulkOutEndpoint,
              UsbMlCommands::ConstBuffer(io_request.header()),
              [this, &io_request, tag](Status status) {
                // Inject a functor into a completion queue driven by the worker
                // thread. Note that the reference to io_request could have been
                // invalidated when the async transfer is cancelled.
                StdMutexLock queue_lock(&callback_mutex_);
                callback_queue_.push([&io_request, tag, status] {
                  // Starting from here is an functor which would be executed
                  // within the worker thread context, after the async transfer
                  // has been completed.
                  if (status.ok()) {
                    VLOG(10) << StringPrintf("[%d-%d] bulk out for header done",
                                             io_request.id(), tag);
                  } else {
                    // TODO: terminate the task early, as there is no
                    // chance we can continue. The more reasonable next step
                    // would be resetting the device.
                    LOG(FATAL) << StringPrintf(
                        "[%d-%d] bulk out for header failed. Abort. %s",
                        io_request.id(), tag, status.ToString().c_str());
                  }
                });
                driver_state_changed_.notify_all();
              },
              __func__);

          if (!async_request_status.ok()) {
            // TODO: terminate the task early, as there is no
            // chance we can continue. The more reasonable next step would
            // be resetting the device.
            LOG(FATAL) << StringPrintf(
                "[%d-%d] bulk out for header failed. Abort. %s",
                io_request.id(), tag, async_request_status.ToString().c_str());
          }
        }

        // Send the actual data in chunks.
        const auto device_buffer = io_request.GetNextChunk(
            options_.max_bulk_out_transfer_size_in_bytes);
        auto host_buffer = address_space_.Translate(device_buffer).ValueOrDie();
        UsbMlCommands::ConstBuffer transfer_buffer(host_buffer.ptr(),
                                                   host_buffer.size_bytes());
        uint32_t transfer_size = static_cast<uint32_t>(transfer_buffer.size());

        ++num_active_transfers;
        if (io_request.HasNextChunk()) {
          // This request still has some data not sent over to the pipeline.
          // Setting this to true prevents rquests of the same tag to start, as
          // chunks from different requests of the same tag must not interleve.
          tag_to_bulk_out_with_unsent_chunk[tag] = true;
        }

        Status async_request_status = usb_device_->AsyncBulkOutTransfer(
            UsbMlCommands::kSingleBulkOutEndpoint, transfer_buffer,
            [this, &io_request, tag, transfer_size](Status status) {
              // Inject a functor into a completion queue driven by the worker
              // thread. Note that the reference to io_request could have been
              // invalidated when the async transfer is cancelled.
              StdMutexLock queue_lock(&callback_mutex_);
              callback_queue_.push([&io_request, tag, status, transfer_size] {
                // Starting from here is an functor which would be executed
                // within the worker thread context, after the async transfer
                // has been completed.
                if (status.ok()) {
                  io_request.NotifyTransferComplete(transfer_size);
                  VLOG(10) << StringPrintf(
                      "%s [%d-%d] bulk out for %u bytes done", __func__,
                      io_request.id(), tag, transfer_size);
                } else {
                  // TODO: terminate the task early, as there is no
                  // chance we can continue. The more reasonable next step
                  // would be resetting the device.
                  LOG(FATAL)
                      << StringPrintf("transfer on tag %d failed. Abort. %s",
                                      tag, status.ToString().c_str());
                }
              });
              driver_state_changed_.notify_all();
            },
            __func__);

        if (!async_request_status.ok()) {
          // TODO: terminate the task early, as there is no
          // chance we can continue. The more reasonable next step would
          // be resetting the device.
          LOG(FATAL) << StringPrintf(
              "%s [%d-%d] async transfer out failed. Abort. %s", __func__,
              io_request.id(), tag, async_request_status.ToString().c_str());
        }
      }
    } else if (io_type == UsbIoRequest::Type::kBulkIn) {
      // If queuing is enabled, bulk-in requests are handled similar to
      // interrupt and dma descriptors.
      if (options_.usb_enable_queued_bulk_in_requests) {
        // Skip if any previous bulk-in request is still incomplete. This is
        // because all bulk-in requests have to be serialized.
        if (is_any_bulk_in_still_uncompleted) {
          continue;
        }

        // Walk through filled buffer queue.
        while (!filled_bulk_in_buffers_.empty()) {
          // We're about to change the state of io requests.
          // This flag indicates we need to call ProcessIo() again.
          is_task_state_changed = true;

          // We're getting a reference, as we're directly modifying
          // begin_offset.
          FilledBulkInInfo& filled_info = filled_bulk_in_buffers_.front();

          const Buffer& buffer = bulk_in_buffers_[filled_info.buffer_index];

          const size_t available_data_size_bytes =
              filled_info.end_offset - filled_info.begin_offset;

          auto device_buffer = io_request.GetNextChunk();

          auto host_buffer =
              address_space_.Translate(device_buffer).ValueOrDie();

          const size_t requested_size_bytes = host_buffer.size_bytes();

          const size_t transferred_bytes =
              std::min(available_data_size_bytes, requested_size_bytes);

          memcpy(host_buffer.ptr(), buffer.ptr() + filled_info.begin_offset,
                 transferred_bytes);

          io_request.NotifyTransferComplete(transferred_bytes);

          if (available_data_size_bytes <= requested_size_bytes) {
            VLOG(10) << StringPrintf(
                "[%d-%d] bulk in for %zu bytes has yielded %zu bytes from "
                "index [%d]",
                io_request.id(), tag, requested_size_bytes,
                available_data_size_bytes, filled_info.buffer_index);

            // We've depleted the buffer. Return it to available queue.
            available_bulk_in_buffers_.push(filled_info.buffer_index);
            filled_bulk_in_buffers_.pop();

            if (io_request.IsCompleted()) {
              // There is no need to check the next buffer, as we've just
              // completed this io_request.
              break;
            }
          } else {
            VLOG(10) << StringPrintf(
                "[%d-%d] bulk in for %zu bytes has yielded %zu bytes "
                "(OVERFLOW) from index [%d]",
                io_request.id(), tag, requested_size_bytes,
                available_data_size_bytes, filled_info.buffer_index);

            filled_info.begin_offset += requested_size_bytes;

            // We just completed this io_request, stop iterating through
            // buffers.
            break;
          }
        }

        if (!io_request.IsCompleted()) {
          // This flag would prevent further bulk-in request in all modes, and
          // further bulk-out in single-ep mode.
          is_any_bulk_in_still_uncompleted = true;
        }
        // Continue to the next io_request.
        continue;
      }

      if (!options_.usb_enable_overlapping_bulk_in_and_out &&
          is_any_bulk_out_still_uncompleted) {
        VLOG(10) << StringPrintf(
            "[%d-%d] configured to start only after all "
            "bulk-out requests complete, wait",
            io_request.id(), tag);
        break;
      } else if (num_active_transfers >= options_.usb_max_num_async_transfers) {
        VLOG(10) << StringPrintf(
            "[%d-%d] number of concurrent transfers too high, wait "
            "(%d >= %d)",
            io_request.id(), tag, num_active_transfers,
            options_.usb_max_num_async_transfers);
        break;
      } else if (io_request.IsActive()) {
        ++num_active_transfers;
        // Still transferring data in. Break from the loop.
        VLOG(10) << StringPrintf(
            "[%d-%d] this bulk in request is still active, wait",
            io_request.id(), tag);
        break;
      } else {
        is_task_state_changed = true;
        is_any_bulk_in_still_uncompleted = true;

        auto device_buffer = (cap_bulk_in_size_at_256_bytes_)
                                 ? io_request.GetNextChunk(256)
                                 : io_request.GetNextChunk();

        auto host_buffer = address_space_.Translate(device_buffer).ValueOrDie();
        UsbMlCommands::MutableBuffer transfer_buffer(host_buffer.ptr(),
                                                     host_buffer.size_bytes());
        uint32_t transfer_size = static_cast<uint32_t>(transfer_buffer.size());

        VLOG(10) << StringPrintf("[%d-%d] bulk in for %zu bytes",
                                 io_request.id(), tag, transfer_buffer.size());

        ++num_active_transfers;

        Status async_request_status = usb_device_->AsyncBulkInTransfer(
            UsbMlCommands::kBulkInEndpoint, transfer_buffer,
            [this, &io_request, tag, transfer_size](
                Status status, size_t num_bytes_transferred) {
              // Inject a functor into a completion queue driven by the worker
              // thread. Note that the reference to io_request could have been
              // invalidated when the async transfer is cancelled.
              StdMutexLock queue_lock(&callback_mutex_);
              callback_queue_.push([&io_request, status, num_bytes_transferred,
                                    tag, transfer_size] {
                // Starting from here is an functor which would be executed
                // within the worker thread context, after the async
                // transfer has been completed.
                if (status.ok()) {
                  io_request.NotifyTransferComplete(num_bytes_transferred);
                  VLOG(10) << StringPrintf(
                      "[%d-%d] bulk in for %u bytes has yielded %zu bytes",
                      io_request.id(), tag, transfer_size,
                      num_bytes_transferred);
                } else {
                  // Note that the reference to io_request could have been
                  // invalidated when the async transfer is cancelled.
                  // TODO: fail the task and allow reset of the
                  // chip.
                  LOG(FATAL)
                      << StringPrintf("%s transfer in failed. Abort. %s",
                                      __func__, status.ToString().c_str());
                }
              });
              driver_state_changed_.notify_all();
            },
            __func__);

        if (!async_request_status.ok()) {
          LOG(FATAL) << StringPrintf("[%d-%d] transfer in failed. Abort",
                                     io_request.id(), tag);
        }

        // Break from further processing if there is any bulk-in request which
        // has not been completed.
        break;
      }
    } else {
      LOG(FATAL) << StringPrintf("%s [%d-%d] unexpected request type", __func__,
                                 io_request.id(), tag);
    }
  }

  return is_task_state_changed;
}

Status UsbDriver::HandleDmaDescriptor(UsbMlCommands::DescriptorTag tag,
                                      uint64_t device_virtual_address,
                                      uint32_t size_bytes,
                                      bool bulk_events_enabled) {
  DeviceBuffer buffer(device_virtual_address, size_bytes);
  VLOG(10) << StringPrintf(
      "Digesting descriptor from device tag[%d], data[0x%llx], size[%zu]",
      static_cast<int>(tag),
      static_cast<unsigned long long>(  // NOLINT(runtime/int)
          buffer.device_address()),
      buffer.size_bytes());

  // First check whether if there is any matching hint.
  for (auto& io_request : io_requests_) {
    const auto hint_tag = io_request.GetTag();
    const auto hint_type = io_request.GetType();
    const auto hint_buffer = io_request.GetBuffer();
    const auto hint_status = io_request.GetSourceAndMatchStatus();

    if (hint_status == UsbIoRequest::SourceAndMatchStatus::kSubmittedByDevice ||
        hint_status ==
            UsbIoRequest::SourceAndMatchStatus::kHintAlreadyMatched) {
      continue;
    }

    if (hint_tag == UsbMlCommands::DescriptorTag::kInstructions) {
      // Device never sends DMA descriptor for instructions, consider them as
      // always matched.
      io_request.SetMatched();
      continue;
    }

    if (!bulk_events_enabled &&
        hint_type != UsbIoRequest::Type::kScHostInterrupt) {
      // Only in-band scalar core interrupts can be matched.
      continue;
    }

    if (tag != hint_tag) {
      // If DMA descriptor from device does not match hint, then it is a new
      // DMA.
      break;
    }

    if (hint_tag != UsbMlCommands::DescriptorTag::kInterrupt0 &&
        hint_buffer != buffer) {
      continue;
    }

    io_request.SetMatched();
    return Status();  // OK.
  }

  // If there is no matching hint, then USB driver should process the
  // descriptor.
  switch (tag) {
    case UsbMlCommands::DescriptorTag::kInputActivations:
    case UsbMlCommands::DescriptorTag::kParameters:
      VLOG(9) << "Received new bulk out command";
      io_requests_.push_back(UsbIoRequest(
          io_requests_.back().id(), UsbIoRequest::Type::kBulkOut, tag, buffer));
      break;

    case UsbMlCommands::DescriptorTag::kOutputActivations:
      VLOG(9) << "Received new bulk in command";
      io_requests_.push_back(UsbIoRequest(
          io_requests_.back().id(), UsbIoRequest::Type::kBulkIn, tag, buffer));
      break;

    case UsbMlCommands::DescriptorTag::kInterrupt0:
    case UsbMlCommands::DescriptorTag::kInterrupt1:
    case UsbMlCommands::DescriptorTag::kInterrupt2:
    case UsbMlCommands::DescriptorTag::kInterrupt3:
      VLOG(9) << "Received new interrupt";
      io_requests_.push_back(UsbIoRequest(io_requests_.back().id(), tag));
      break;

    // Instruction descriptor is never sent from device.
    case UsbMlCommands::DescriptorTag::kInstructions:
    case UsbMlCommands::DescriptorTag::kUnknown:
      LOG(FATAL) << StringPrintf("Unknown descriptor from device");
  }

  return Status();  // OK.
}

void UsbDriver::HandleQueuedBulkIn(const Status& status, int buffer_index,
                                   size_t num_bytes_transferred) {
  if (status.ok()) {
    // Enqueue the filled buffer with actual data size.
    filled_bulk_in_buffers_.push(
        FilledBulkInInfo{buffer_index, 0, num_bytes_transferred});

    VLOG(1) << StringPrintf("bulk in %zu bytes from buffer index [%d]",
                            num_bytes_transferred, buffer_index);
  } else {
    // num_bytes_transferred is not valid. Just return the buffer to available
    // queue.

    available_bulk_in_buffers_.push(buffer_index);

    if (!IsCancelled(status) && !IsDeadlineExceeded(status)) {
      // TODO: convert to driver error.
      LOG(FATAL) << StringPrintf("%s transfer in failed. %s", __func__,
                                 status.ToString().c_str());
    }
  }
}

void UsbDriver::WorkerThreadFunc() {
  VLOG(7) << StringPrintf("%s starting worker thread", __func__);
  TRACE_START_THREAD("UsbDriverWorkerThread");

  // Types of background operations that need to be triggered in parallel to IO
  // request handling.
  enum BackgroudOperations {
    kReadOutputActivations = 0,
    kReadEvent,
    kReadInterrupt,
    kNumBackgroundOperations,
  };

  // Current background operations.
  std::bitset<kNumBackgroundOperations> background_ops;

  do {
    // Lock the driver and check for state.
    StdCondMutexLock state_lock(&mutex_);

    VLOG(10) << StringPrintf(
        "%s dispatching %d callback events in worker thread", __func__,
        static_cast<int>(QueueSize(&callback_queue_, &callback_mutex_)));

    while (!IsQueueEmpty(&callback_queue_, &callback_mutex_)) {
      // Note the queue is not locked when the callback executes. This is
      // intentional, as it simplifies design of interrupt handlers, allowing
      // sync CSR access from the handlers.
      QueuePop(&callback_queue_, &callback_mutex_)();
    }

    bool reevaluation_needed = false;

    if (state_ == kClosing) {
      // If all buffers are available, flag that we're not reading output
      // activations at this moment. (So it's okay to close the driver.)
      if (available_bulk_in_buffers_.size() ==
          options_.usb_bulk_in_queue_capacity) {
        background_ops[kReadOutputActivations] = false;

        VLOG(10) << "All bulk-in buffers are available";
      }

      if (background_ops.any() || !dma_scheduler_.IsEmpty()) {
        VLOG(7) << "Driver is closing. Wait for async operations to complete.";
      } else {
        // Terminate the worker thread.
        VLOG(7)
            << "Driver is closing, and all async operations have completed.";
        break;
      }
    } else if (state_ == kPaused) {
      VLOG(7) << "Driver is paused. Do not initiate further device operations.";
    } else {
      // Check if any of the async operations needs to be re-installed.
      if (!background_ops[kReadEvent]) {
        VLOG(7) << StringPrintf("%s Re-installing event reader", __func__);
        reevaluation_needed = true;
        background_ops[kReadEvent] = true;
        Status status = usb_device_->AsyncReadEvent(
            [this, &background_ops](
                Status status,
                const UsbMlCommands::EventDescriptor& event_info) {
              StdMutexLock queue_lock(&callback_mutex_);
              callback_queue_.push([this, &background_ops, status, event_info] {
                // Note this wrapping confuses thread safety analyzer
                HandleEvent(status, event_info);
                background_ops[kReadEvent] = false;
              });
              driver_state_changed_.notify_all();
            });
        if (!status.ok()) {
          VLOG(1) << StringPrintf("%s AsyncReadEvent failed:", __func__)
                  << status;
          break;
        }
      }
      if (!background_ops[kReadInterrupt]) {
        VLOG(7) << StringPrintf("%s Re-installing interrupt reader", __func__);
        background_ops[kReadInterrupt] = true;
        reevaluation_needed = true;
        Status status = usb_device_->AsyncReadInterrupt(
            [this, &background_ops](
                Status status,
                const UsbMlCommands::InterruptInfo& interrupt_info) {
              StdMutexLock queue_lock(&callback_mutex_);
              callback_queue_.push(
                  [this, &background_ops, status, interrupt_info] {
                    // Note this wrapping confuses thread safety analyzer
                    HandleInterrupt(status, interrupt_info);
                    background_ops[kReadInterrupt] = false;
                  });
              driver_state_changed_.notify_all();
            });
        if (!status.ok()) {
          VLOG(1) << StringPrintf("%s AsyncReadInterrupt failed:", __func__)
                  << status;
          break;
        }
      }

      if (options_.usb_enable_queued_bulk_in_requests) {
        while (!available_bulk_in_buffers_.empty()) {
          const int buffer_index = available_bulk_in_buffers_.front();
          available_bulk_in_buffers_.pop();

          VLOG(7) << StringPrintf(
              "%s Installing bulk-in reader. buffer index [%d]", __func__,
              buffer_index);

          background_ops[kReadOutputActivations] = true;
          reevaluation_needed = true;

          UsbMlCommands::MutableBuffer transfer_buffer(
              bulk_in_buffers_[buffer_index].ptr(),
              bulk_in_buffers_[buffer_index].size_bytes());

          // Clear data to prevent data leakage from request to request.
          memset(transfer_buffer.data(), 0, transfer_buffer.size());

          Status async_request_status = usb_device_->AsyncBulkInTransfer(
              UsbMlCommands::kBulkInEndpoint, transfer_buffer,
              [this, buffer_index](Status status,
                                   size_t num_bytes_transferred) {
                // This functor is executed directly from underlying completion
                // callback thread. We need to transfer it to be processed in
                // WorkerThreadFunc by pushing a new functor to the callback
                // queue.
                StdMutexLock queue_lock(&callback_mutex_);
                callback_queue_.push(
                    [this, status, buffer_index, num_bytes_transferred] {
                      // This function is executed from WorkerThreadFunc.
                      // Note this wrapping confuses thread safety analyzer.
                      HandleQueuedBulkIn(status, buffer_index,
                                         num_bytes_transferred);
                    });
                // Notify the worker thread to work on the callback queue.
                driver_state_changed_.notify_all();
              },
              __func__);

          if (!async_request_status.ok()) {
            // TODO: convert to some driver error.
            LOG(FATAL) << "Bulk-in failed. Abort";
          }
        }
      }

      reevaluation_needed = ProcessIo().ValueOrDie();

      // TODO: Enter kPaused state when dma_scheduler_.IsEmpty(). Any
      // new task should kick the driver back to kOpen state. Note this is in
      // contradiction to the plan to remove state in USB driver.
    }

    if (reevaluation_needed) {
      VLOG(10) << StringPrintf("%s re-evaluation is needed", __func__);
    } else {
      StdCondMutexLock queue_lock(&callback_mutex_);

      Lock2 unlock_both(state_lock, queue_lock);

      if (callback_queue_.empty()) {
        VLOG(10) << StringPrintf("%s waiting on state change", __func__);

        // Release the lock and wait for further state change.
        driver_state_changed_.wait(unlock_both);

        VLOG(10) << StringPrintf("%s driver state change detected", __func__);
      } else {
        VLOG(10) << StringPrintf("%s callback event available. skip waiting",
                                 __func__);
      }
    }
  } while (true);

  VLOG(7) << StringPrintf("%s leaving worker thread", __func__);
}

StatusOr<std::unique_ptr<UsbDeviceInterface>>
UsbDriver::CreateRawUsbDeviceWithRetry() {
  TRACE_SCOPE("UsbDriver::CreateRawUsbDeviceWithRetry");
  Status result;
  for (int i = 0; i < kMaxNumOfRetryAfterReset; ++i) {
    TRACE_SCOPE("UsbDriver::CreateRawUsbDeviceWithRetry:try");

    // Wait even for the first time, before opening the raw device.
    // We found it seems to reduce chances of transfer errors in long-running
    // back-to-back tests.
    // TODO: revisit after the connection issue has been resolved.
    {
      TRACE_SCOPE("UsbDriver::CreateRawUsbDeviceWithRetry:Microsleep");
      Microsleep(kSleepTimeMicroSecondsBeforeRetry);
    }

    // Try to open the raw device.
    auto raw_device_or_error = device_factory_();

    // Return early if we get an OK.
    result = raw_device_or_error.status();
    if (result.ok()) {
      return raw_device_or_error;
    }
  }
  return result;
}

Status UsbDriver::OpenMlUsbDevice() {
  TRACE_SCOPE("UsbDriver::OpenMlUsbDevice");

  VLOG(7) << "Opening device expecting application mode";

  ASSIGN_OR_RETURN(auto raw_usb_device, CreateRawUsbDeviceWithRetry());

  usb_device_ = gtl::MakeUnique<UsbMlCommands>(std::move(raw_usb_device),
                                               options_.usb_timeout_millis);

  return usb_device_ ? Status() : UnknownError("Failed to create ML device");
}

Status UsbDriver::PrepareUsbDevice() {
  TRACE_SCOPE("UsbDriver::PrepareUsbDevice");

  // 1) Send DFU Detach command if already in application mode
  // 2) USB Reset
  // 3) Perform DFU
  // 4) USB Reset
  std::unique_ptr<UsbDeviceInterface> raw_usb_device;

  VLOG(7) << "Open device and check if DFU is needed";

  ASSIGN_OR_RETURN(raw_usb_device, CreateRawUsbDeviceWithRetry());

  auto dfu_device = gtl::MakeUnique<UsbDfuCommands>(
      std::move(raw_usb_device), options_.usb_timeout_millis);

  ASSIGN_OR_RETURN(UsbDfuCommands::DeviceDescriptor device_desc,
                   dfu_device->GetDeviceDescriptor());

  // Timeout before DFU Detach expires.
  constexpr int kShortTimeoutMillis = 100;
  bool expect_app_mode_after_reset = false;

  if ((device_desc.vendor_id == kTargetAppVendorId) &&
      (device_desc.product_id == kTargetAppProductId)) {
    if (options_.usb_always_dfu) {
      // Device is in app mode, send DFU Detach command.
      VLOG(7) << "Device is in application mode, sending DFU Detach";

      constexpr int kDfuInterface = 0;
      RETURN_IF_ERROR(dfu_device->ClaimInterface(kDfuInterface));
      RETURN_IF_ERROR(dfu_device->DfuDetach(kShortTimeoutMillis));

      expect_app_mode_after_reset = false;
    } else {
      // Device is in app mode, we're done.
      VLOG(7) << "Device is already in application mode, skipping DFU";
      expect_app_mode_after_reset = true;
    }
  } else if ((device_desc.vendor_id == kTargetDfuVendorId) &&
             (device_desc.product_id == kTargetDfuProductId)) {
    // Do nothing.
    expect_app_mode_after_reset = false;
    VLOG(7) << "Device is in DFU mode";
  } else {
    return FailedPreconditionError("Unrecognized USB Vendor/Product ID");
  }

  VLOG(7) << "Resetting device";

  // Close with USB Reset no matter which mode we're in.
  RETURN_IF_ERROR(
      dfu_device->Close(UsbDfuCommands::CloseAction::kGracefulPortReset));

  if (expect_app_mode_after_reset) {
    return OpenMlUsbDevice();
  }

  VLOG(7) << "Opening device expecting DFU mode";

  // Try to open again.
  ASSIGN_OR_RETURN(raw_usb_device, CreateRawUsbDeviceWithRetry());

  dfu_device = gtl::MakeUnique<UsbDfuCommands>(std::move(raw_usb_device),
                                               options_.usb_timeout_millis);

  // Download firmware, and then upload for verification.
  if (!options_.usb_firmware_image.empty()) {
    VLOG(7) << "DFU with supplied firmware image";

    // Use firmware image supplied.
    RETURN_IF_ERROR(UsbUpdateDfuDevice(dfu_device.get(),
                                       options_.usb_firmware_image,
                                       /*skip_verify*/ false));
  } else {
    // Use firmware image built-in.
    VLOG(7) << "DFU with built-in firmware image";

    const uint8* dfu_firmware = nullptr;
    size_t dfu_firmware_size = 0;
    switch (options_.mode) {
      case OperatingMode::kMultipleEndpointsHardwareControl:
      case OperatingMode::kMultipleEndpointsSoftwareQuery:
        dfu_firmware = apex_latest_multi_ep;
        dfu_firmware_size = apex_latest_multi_ep_len;
        break;

      case OperatingMode::kSingleEndpoint:
        dfu_firmware = apex_latest_single_ep;
        dfu_firmware_size = apex_latest_single_ep_len;
        break;

      default:
        return FailedPreconditionError("Unrecognized operating mode");
    }

    RETURN_IF_ERROR(UsbUpdateDfuDevice(
        dfu_device.get(),
        UsbDeviceInterface::ConstBuffer(
            reinterpret_cast<const uint8*>(dfu_firmware), dfu_firmware_size),
        /*skip_verify＝*/ false));
  }

  VLOG(7) << "Resetting device";
  // Reset to trigger switching to application mode.
  RETURN_IF_ERROR(
      dfu_device->Close(UsbDfuCommands::CloseAction::kGracefulPortReset));

  return OpenMlUsbDevice();
}

Status UsbDriver::DoOpen(bool debug_mode) {
  TRACE_SCOPE("UsbDriver::DoOpen");

  StdMutexLock state_lock(&mutex_);
  RETURN_IF_ERROR(ValidateState(/*expected_state=*/kClosed));

  if (options_.usb_enable_queued_bulk_in_requests) {
    if (!options_.usb_enable_overlapping_bulk_in_and_out) {
      return FailedPreconditionError(
          "Overlapping bulk-in/out must be enabled for queued bulk-in "
          "feature");
    }

    constexpr unsigned int k1kBMask = 1024 - 1;
    if (options_.usb_bulk_in_max_chunk_size_in_bytes & k1kBMask) {
      return OutOfRangeError(
          "Bulk-in buffer max chunk size must be 1024-byte aligned");
    }

    if (options_.usb_bulk_in_queue_capacity <= 0) {
      return OutOfRangeError("Bulk-in queue capacity must be positive");
    }
  } else {
    options_.usb_bulk_in_queue_capacity = 0;
  }

  if (device_factory_) {
    RETURN_IF_ERROR(PrepareUsbDevice());
  } else {
    // No device factory is provided. An instance must already be supplied.
    if (usb_device_ == nullptr) {
      return FailedPreconditionError(
          "Either device factory or device instance must be supplied");
    }
  }

  switch (usb_device_->GetDeviceSpeed()) {
    case UsbStandardCommands::DeviceSpeed::kLow:
      return FailedPreconditionError("USB Low speed is not supported");

    case UsbStandardCommands::DeviceSpeed::kFull:
    case UsbStandardCommands::DeviceSpeed::kHigh:
      if (options_.usb_fail_if_slower_than_superspeed) {
        return FailedPreconditionError("Connection speed is too slow, fail.");
      } else if (options_.mode != OperatingMode::kSingleEndpoint) {
        return FailedPreconditionError(
            "Connection speed is incompatible with operating mode, fail");
      }
      break;

    case UsbStandardCommands::DeviceSpeed::kSuper:
      break;

    case UsbStandardCommands::DeviceSpeed::kUnknown:
    default:
      VLOG(7) << "Connection speed is unknown, ignore speed constraint";
      break;
  }

  constexpr int kMlInterface = 0;
  RETURN_IF_ERROR(usb_device_->ClaimInterface(kMlInterface));

  RETURN_IF_ERROR(registers_->Open(usb_device_.get()));

  RETURN_IF_ERROR(top_level_handler_->Open());
  auto top_level_handler_closer =
      MakeCleanup([this] { CHECK_OK(top_level_handler_->Close()); });

  // Disable clock gate and reset GCB for clean state.
  RETURN_IF_ERROR(top_level_handler_->DisableSoftwareClockGate());
  RETURN_IF_ERROR(top_level_handler_->DisableHardwareClockGate());
  RETURN_IF_ERROR(top_level_handler_->EnableReset());

  // Quit from reset mode before accessing the chip.
  RETURN_IF_ERROR(top_level_handler_->QuitReset());
  RETURN_IF_ERROR(top_level_handler_->EnableHardwareClockGate());

  RETURN_IF_ERROR(InitializeChip());
  if (!debug_mode) {
    // Move all subsystems to Run state.
    RETURN_IF_ERROR(run_controller_->DoRunControl(RunControl::kMoveToRun));
  }

  RETURN_IF_ERROR(RegisterAndEnableAllInterrupts());

  if (cap_bulk_in_size_at_256_bytes_) {
    constexpr size_t k256Bytes = 256;
    if (options_.usb_bulk_in_max_chunk_size_in_bytes > k256Bytes) {
      options_.usb_bulk_in_max_chunk_size_in_bytes = k256Bytes;

      VLOG(7) << "Reducing bulk-in request size to 256 bytes for USB2";
    }
  }

  for (int i = 0; i < options_.usb_bulk_in_queue_capacity; ++i) {
    auto chunk = DoMakeBuffer(options_.usb_bulk_in_max_chunk_size_in_bytes);
    if (!chunk.IsValid()) {
      return ResourceExhaustedError("Bulk-in buffer chunk allocation failure");
    }

    // Save the Buffer object into a container, so it will be destroyed when
    // driver destructs.
    bulk_in_buffers_.push_back(chunk);

    // Save the index of available Buffer into the queue.
    available_bulk_in_buffers_.push(i);
  }

  // DMA scheduler.
  RETURN_IF_ERROR(dma_scheduler_.Open());
  auto dma_scheduler_closer = MakeCleanup([this] {
    CHECK_OK(dma_scheduler_.Close(api::Driver::ClosingMode::kGraceful));
  });

  worker_thread_ = std::thread([this] { WorkerThreadFunc(); });

  // On-Chip DRAM allocator.
  RETURN_IF_ERROR(dram_allocator_->Open());

  // All good. Move state to open.
  RETURN_IF_ERROR(SetState(kOpen));

  // Release cleanup functions.
  dma_scheduler_closer.release();
  top_level_handler_closer.release();

  return Status();  // OK
}

Status UsbDriver::DoClose(bool in_error, api::Driver::ClosingMode mode) {
  TRACE_SCOPE("UsbDriver::DoClose");

  if (mode != api::Driver::ClosingMode::kGraceful) {
    LOG(WARNING) << "Only graceful closing mode is currently supported in USB "
                    "driver; forcing to graceful";
    mode = api::Driver::ClosingMode::kGraceful;
  }

  {
    StdMutexLock state_lock(&mutex_);
    RETURN_IF_ERROR(ValidateStates({kOpen, kPaused}));

    // Note our intention to close. Clocking gating is disabled here.
    RETURN_IF_ERROR(SetState(kClosing));
  }

  worker_thread_.join();

  // All good. Shut down stuff. This is best effort. So if things starts
  // failing, keep going and try cleaning up as much as we can.

  RETURN_IF_ERROR(dma_scheduler_.Close(mode));
  RETURN_IF_ERROR(DisableAllInterrupts());
  RETURN_IF_ERROR(UnmapAllParameters());
  RETURN_IF_ERROR(run_controller_->DoRunControl(RunControl::kMoveToHalt));
  RETURN_IF_ERROR(top_level_handler_->EnableReset());
  RETURN_IF_ERROR(registers_->Close());
  RETURN_IF_ERROR(dram_allocator_->Close());

  // Deallocate all bulk-in buffers. This is not absolutely necessary, but it's
  // better to have a clean slate for the next Open.
  bulk_in_buffers_.clear();

  // Flush available buffers queue, marking we have no any buffer available.
  while (!available_bulk_in_buffers_.empty()) {
    available_bulk_in_buffers_.pop();
  }

  // All buffers should have been released, as all lsusb request should have
  // been canceled.
  CHECK(filled_bulk_in_buffers_.empty());

  // Release ownership to the USB device instance.
  usb_device_.reset();

  // Finalize.
  {
    StdMutexLock state_lock(&mutex_);
    RETURN_IF_ERROR(SetState(kClosed));
  }

  return Status();  // OK
}

Status UsbDriver::DoCancelAndWaitRequests(bool in_error) {
  RETURN_IF_ERROR(dma_scheduler_.CancelPendingRequests());
  if (!in_error) {
    RETURN_IF_ERROR(dma_scheduler_.WaitActiveRequests());
  }
  return Status();  // OK
}

Buffer UsbDriver::DoMakeBuffer(size_t size_bytes) const {
  Buffer buffer = allocator_->MakeBuffer(size_bytes);

  if (buffer.IsValid()) {
    // Clear data to prevent data leakage from request to request.
    memset(buffer.ptr(), 0, buffer.size_bytes());
  }
  return buffer;
}

StatusOr<MappedDeviceBuffer> UsbDriver::DoMapBuffer(const Buffer& buffer,
                                                    DmaDirection direction) {
  if (buffer.IsValid()) {
    ASSIGN_OR_RETURN(auto device_buffer, address_space_.MapMemory(buffer));
    // TODO : this is dangerous: the std::bind captures a raw pointer to
    // the underlying object of the unique_ptr'd address space.
    // This will break if executable registry outlives address space in the
    // driver.
    return MappedDeviceBuffer(
        device_buffer, std::bind(&NopAddressSpace::UnmapMemory, &address_space_,
                                 std::placeholders::_1));
  }

  return MappedDeviceBuffer();
}

StatusOr<std::shared_ptr<TpuRequest>> UsbDriver::DoCreateRequest(
    const std::shared_ptr<Request> parent_request,
    const ExecutableReference* executable_ref, TpuRequest::RequestType type) {
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateStates({kOpen}));

  // TODO: find a way to mix models, switching on and off descriptors
  // on the fly.
  if (!options_.usb_enable_bulk_descriptors_from_device) {
    // If we disable bulk in/out descriptors from device, the hint must be
    // complete.
    if (!executable_ref->executable().dma_hints()->fully_deterministic()) {
      return FailedPreconditionError(
          StringPrintf("Executable '%s' must have fully deterministic DMA "
                       "hints when DMA descriptors from device are disabled.",
                       executable_ref->executable().name()->c_str()));
    }
  }

  return {std::make_shared<SingleTpuRequest>(
      next_id_++, parent_request, executable_ref, allocator_.get(),
      dram_allocator_.get(),
      gtl::MakeUnique<DeviceBufferMapper>(&address_space_),
      &dma_info_extractor_,
      chip_config_->GetChipStructures().minimum_alignment_bytes, type)};
}

Status UsbDriver::DoSubmit(std::shared_ptr<TpuRequest> request) {
  TRACE_SCOPE("UsbDriver::DoSubmit");
  StdMutexLock state_lock(&mutex_);
  RETURN_IF_ERROR(ValidateStates({kOpen}));

  // Validate and prepare request.
  RETURN_IF_ERROR(request->Validate());
  RETURN_IF_ERROR(request->Prepare());

  RETURN_IF_ERROR(dma_scheduler_.Submit(std::move(request)));

  // Set the driver state to open and kick off processing.
  RETURN_IF_ERROR(SetState(kOpen));

  TRACE_WITHIN_SCOPE("UsbDriver::DoSubmit::Finished");
  return Status();  // OK
}

Status UsbDriver::DoSetRealtimeMode(bool on) {
  // TODO: Implementing real-time scheduler support for USB as
  // well.
  return FailedPreconditionError(
      "This driver does not support real-time mode.");
}

Status UsbDriver::DoSetExecutableTiming(const ExecutableReference* executable,
                                        const api::Timing& timing) {
  // TODO: Implementing real-time scheduler support for USB as
  // well.
  return FailedPreconditionError(
      "This driver does not support real-time mode.");
}

void UsbDriver::CheckFatalError(const Status& status) {
  // TODO: Forward to the client application for handling.
  CHECK_OK(status) << "Driver fatal error";
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
