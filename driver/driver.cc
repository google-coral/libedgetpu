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

#include "driver/driver.h"

#include <atomic>
#include <memory>

#include "absl/strings/str_format.h"
#include "api/execution_context_interface.h"
#include "api/package_reference.h"
#include "api/request.h"
#include "driver/package_registry.h"
#include "driver/request.h"
#include "driver/tpu_request.h"
#include "executable/executable_generated.h"
#include "port/blocking_counter.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/logging.h"
#include "port/math_util.h"
#include "port/ptr_util.h"
#include "port/shared_mutex.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/statusor.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"
#include "port/tracing.h"

namespace platforms {
namespace darwinn {
namespace driver {

namespace {

using api::ExecutionContextInterface;

}  // namespace

Driver::Driver(api::Chip chip,
               std::unique_ptr<PackageRegistry> executable_registry,
               const api::DriverOptions& driver_options,
               std::unique_ptr<TimeStamper> time_stamper)
    : chip_(chip),
      executable_registry_(std::move(executable_registry)),
      time_stamper_(std::move(time_stamper)),
      current_parameter_caching_token_(0),
      debug_mode_(false),
      max_scheduled_work_ns_(driver_options.max_scheduled_work_ns()) {
  // Use the default_telemeter by default.
  telemeter_interface_ = &default_telemeter_;

  operational_settings_.tpu_frequency_hz = driver_options.tpu_frequency_hz();
  operational_settings_.host_to_tpu_bps = driver_options.host_to_tpu_bps();

  scheduler_thread_ = std::thread([this]() { SchedulerWorker(); });
}

Driver::~Driver() {
  {
    StdMutexLock scheduler_lock(&scheduler_mutex_);
    destructing_ = true;
    scheduler_wakeup_.notify_one();
  }

  if (scheduler_thread_.joinable()) {
    scheduler_thread_.join();
  }
}

std::string Driver::BadStateMessage(State expected_state) const {
  return StringPrintf("Bad driver state. expected=%d, actual=%d.",
                         expected_state, state_);
}

util::Status Driver::SetState(State next_state) {
  switch (state_) {
    case kOpen:
      if (next_state == kClosing) {
        state_ = next_state;
        return util::Status();  // OK
      }
      break;

    case kClosing:
      if (next_state == kClosed) {
        state_ = next_state;
        return util::Status();  // OK
      }
      break;

    case kClosed:
      if (next_state == kOpen) {
        state_ = next_state;
        return util::Status();  // OK
      }
      break;
  }

  // Illegal state transition.
  return util::FailedPreconditionError(StringPrintf(
      "Invalid state transition. current=%d, next=%d.", state_, next_state));
}

bool Driver::IsOpen() const {
  ReaderMutexLock state_reader_lock(&state_mutex_);
  return state_ == kOpen;
}

bool Driver::IsError() const { return in_error_; }

util::Status Driver::Open(bool debug_mode, bool context_lost) {
  WriterMutexLock state_writer_lock(&state_mutex_);
  if (num_clients_ > 0) {
    if (context_lost) {
      return util::InvalidArgumentError(
          "context_lost was set at open() yet there were others holding the "
          "driver open.");
    }

    num_clients_++;
    return util::Status();  // OK
  }

  if (state_ != kClosed) {
    return util::FailedPreconditionError(BadStateMessage(kClosed));
  }

  if (context_lost) {
    executable_registry_->ResetParametersLoaded();
  }

  debug_mode_ = debug_mode;
  RETURN_IF_ERROR(DoOpen(debug_mode));
  num_clients_++;

  // All good. Move state to open.
  RETURN_IF_ERROR(SetState(kOpen));

  return util::Status();  // OK.
}

namespace {

// Computes maximum execution time in ms by taking the ceiling of
// cycle / frequency in KHz.
int64 ComputeMETinMs(int64 cycles, int64 frequency) {
  constexpr int64 kKilo = 1000LL;
  if (cycles > 0 && frequency > 0) {
    return 1 + (cycles - 1) / (frequency / kKilo);
  } else {
    return 0;
  }
}

}  // namespace

util::Status Driver::UpdateInitialTiming(
    const api::PackageReference* api_package_reference) {
  StdMutexLock lock(&submit_mutex_);

  const PackageReference* driver_package_reference =
      static_cast<const PackageReference*>(api_package_reference);
  const auto* executable_reference =
      driver_package_reference->MainExecutableReference();

  // Don't bother calling the driver's SetExecutableTiming if it doesn't even
  // support real-time mode, or if the driver's operating frequency is not set.
  if (!HasImplementedRealtimeMode() ||
      operational_settings_.tpu_frequency_hz <= 0) {
    return util::OkStatus();
  }

  // Producing initial guess for estimated execution time.
  // More precise execution time can be obtained from real execution timing
  // statistics. At this point, understating operating frequency is ok as we
  // need a conservative estimation.
  if (executable_reference->EstimatedCycles() > 0) {
    api::Timing timing;
    timing.max_execution_time_ms = static_cast<int>(
        ComputeMETinMs(executable_reference->EstimatedCycles(),
                       operational_settings_.tpu_frequency_hz));
    // This initial timing setting is set regardless whether the underlying
    // driver supports real-time mode or not, thus is best effort.
    return SetExecutableTiming(api_package_reference, timing);
  } else {
    // The executable doesn't carry estimated cycles information.
    return util::OkStatus();
  }
}

util::StatusOr<const api::PackageReference*> Driver::RegisterExecutableFile(
    const std::string& executable_filename) {
  TRACE_SCOPE("Driver::RegisterExecutableFile");
  ASSIGN_OR_RETURN(auto* registered_package,
                   executable_registry_->RegisterFile(executable_filename));
  RETURN_IF_ERROR(UpdateInitialTiming(registered_package));
  return registered_package;
}

util::StatusOr<const api::PackageReference*>
Driver::RegisterExecutableSerialized(const std::string& executable_content) {
  TRACE_SCOPE("Driver::RegisterExecutableSerialized");
  ASSIGN_OR_RETURN(
      auto* registered_package,
      executable_registry_->RegisterSerialized(executable_content));
  RETURN_IF_ERROR(UpdateInitialTiming(registered_package));
  return registered_package;
}

util::StatusOr<const api::PackageReference*>
Driver::RegisterExecutableSerialized(const char* executable_content,
                                     size_t length) {
  TRACE_SCOPE("Driver::RegisterExecutableSerialized");
  ASSIGN_OR_RETURN(
      auto* registered_package,
      executable_registry_->RegisterSerialized(executable_content, length));
  RETURN_IF_ERROR(UpdateInitialTiming(registered_package));
  return registered_package;
}

// TODO  Keeping parameters mapped for the entire time driver is
// open can lead to OOM even if we have enough memory for one request.
util::Status Driver::MapParameters(PackageReference& package_ref) {
  TRACE_SCOPE("Driver::MapParameters");

  // If this is the first time we are mapping parameters and the parameters are
  // supposed to reside in the on-chip DRAM, we should transfer them first.

  for (auto* driver_executable_ref : package_ref.AllExecutableReferences()) {
    RETURN_IF_ERROR(driver_executable_ref->PrepareParameters());
    const Buffer& buffer = driver_executable_ref->parameters();

    // TODO Investigate if we need to optimize cache flushing here.
    ASSIGN_OR_RETURN(MappedDeviceBuffer mapped_device_buffer,
                     DoMapBuffer(buffer, DmaDirection::kToDevice));

    const DeviceBuffer& device_buffer = mapped_device_buffer.device_buffer();
    VLOG(3) << absl::StrFormat("Mapped params : %s -> 0x%016llx, %zu bytes.",
                               buffer.ToString().c_str(),
                               device_buffer.device_address(),
                               device_buffer.size_bytes());
    RETURN_IF_ERROR(driver_executable_ref->SetMappedParameters(
        std::move(mapped_device_buffer)));
  }

  return util::OkStatus();
}

Buffer Driver::MakeBuffer(size_t size_bytes) const {
  return DoMakeBuffer(size_bytes);
}

util::Status Driver::UnregisterExecutable(
    const api::PackageReference* executable_ref) {
  ReaderMutexLock state_reader_lock(&state_mutex_);

  // Remove per-executable timing information from real-time scheduler.
  if (HasImplementedRealtimeMode()) {
    RETURN_IF_ERROR(RemoveExecutableTiming(executable_ref));
  }

  // TODO : should defer unregistering if there are pending
  // requests.
  return executable_registry_->Unregister(executable_ref);
}

util::StatusOr<std::shared_ptr<api::Request>> Driver::CreateRequest(
    const api::PackageReference* api_package_ref) {
  if (api_package_ref == nullptr) {
    return util::InvalidArgumentError("Package reference is null.");
  }

  const auto* package_ref =
      static_cast<const PackageReference*>(api_package_ref);
  return {std::make_shared<Request>(
      next_id_.fetch_add(1, std::memory_order_relaxed), *package_ref,
      *time_stamper_)};
}

util::Status Driver::Submit(std::shared_ptr<api::Request> api_request,
                            api::Request::Done done_callback) {
  TRACE_SCOPE("Driver::Submit");
  ReaderMutexLock state_reader_lock(&state_mutex_);
  StdMutexLock submit_lock(&submit_mutex_);

  if (state_ != kOpen) {
    return util::UnavailableError(BadStateMessage(kOpen));
  }

  auto request = std::static_pointer_cast<Request>(api_request);
  RETURN_IF_ERROR(request->SetDone(std::move(done_callback)));
  RETURN_IF_ERROR(request->Prepare());
  RETURN_IF_ERROR(CheckLatencyTolerance(request));

  if (request->GetPriority() == 0) {
    VLOG(4) << StringPrintf("Request [%d]: Submitting P0 request immediately.",
                            request->id());
    ASSIGN_OR_RETURN(auto remaining_tpu_requests,
                     request->RemainingTpuRequestCount());
    for (int i = 0; i < remaining_tpu_requests; ++i) {
      RETURN_IF_ERROR(SubmitInferenceRequest(request));
    }
  } else {
    VLOG(4) << StringPrintf(
        "Request [%d]: Pushing P%d request to its priority queue.",
        request->id(), request->GetPriority());
    pending_requests_[request->GetPriority()].push(std::move(request));
    RETURN_IF_ERROR(TrySchedulePendingRequests());
  }

  return util::OkStatus();
}

util::Status Driver::CheckLatencyTolerance(
    const std::shared_ptr<Request>& request) {
  TRACE_SCOPE("Driver::CheckLatencyTolerance");
  const auto& package_ref = request->GetPackageReference();
  if (package_ref.LatencyToleranceMs() <= 0) {
    // No latency requirement set.
    return util::OkStatus();
  }

  if (request->GetPriority() > 0) {
    return util::InvalidArgumentError(
        "Latency tolerance can only be set for P0 requests.");
  }

  ASSIGN_OR_RETURN(auto tpu_request_count, request->RemainingTpuRequestCount());
  int64 estimated_cycles =
      tpu_request_count *
      package_ref.MainExecutableReference()->EstimatedCycles();

  ASSIGN_OR_RETURN(bool needs_parameter_caching,
                   NeedsParameterCaching(request));
  if (needs_parameter_caching) {
    estimated_cycles +=
        package_ref.ParameterCachingExecutableReference()->EstimatedCycles();
  }

  estimated_cycles += MaxRemainingCycles();

  int64 estimated_time_ms =
      ComputeMETinMs(estimated_cycles, operational_settings_.tpu_frequency_hz);
  if (estimated_time_ms > package_ref.LatencyToleranceMs()) {
    return util::DeadlineExceededError(absl::StrFormat(
        "Estimated execution time (%lld ms) exceeds max tolerance (%lld ms).",
        estimated_time_ms, package_ref.LatencyToleranceMs()));
  }

  return util::OkStatus();
}

util::Status Driver::SubmitInferenceRequest(std::shared_ptr<Request> request) {
  TRACE_SCOPE("Driver::SubmitInferenceRequest");
  const auto& package_ref = request->GetPackageReference();
  ASSIGN_OR_RETURN(auto parameters_mapped, package_ref.ParametersMapped());
  if (!parameters_mapped) {
    // TODO Remove the const casts.
    VLOG(5) << StringPrintf("Request [%d]: Need to map parameters.",
                            request->id());
    RETURN_IF_ERROR(MapParameters(const_cast<PackageReference&>(package_ref)));
  }

  const auto& main_ref = request->MainExecutableReference();
  if (main_ref.ParameterCachingToken() == 0 ||
      main_ref.ParameterCachingToken() != current_parameter_caching_token_) {
    ResetCachedParameters();
  }

  ASSIGN_OR_RETURN(bool needs_parameter_caching,
                   NeedsParameterCaching(request));
  if (needs_parameter_caching) {
    VLOG(5) << StringPrintf("Request [%d]: Need to do parameter-caching.",
                            request->id());
    RETURN_IF_ERROR(SubmitParameterCachingRequest(request));
  }

  ASSIGN_OR_RETURN(auto tpu_request,
                   DoCreateRequest(request, &request->MainExecutableReference(),
                                   TpuRequest::RequestType::INFERENCE));
  RETURN_IF_ERROR(request->PrepareTpuRequest(tpu_request));

  // Record the submission time before actually submitting the workload. This
  // avoids race conditions where the completion is notified before submission.
  request->NotifySubmission(TpuRequest::RequestType::INFERENCE);
  RETURN_IF_ERROR(DoSubmit(std::move(tpu_request)));

  return util::OkStatus();
}

util::StatusOr<bool> Driver::NeedsParameterCaching(
    const std::shared_ptr<Request>& request) const {
  const auto& package_ref = request->GetPackageReference();
  if (!package_ref.ParameterCachingEnabled()) {
    return false;
  }

  const auto& parameter_caching_ref =
      package_ref.ParameterCachingExecutableReference();
  if (parameter_caching_ref->ParameterCachingToken() == 0) {
    return util::InternalError("Parameter caching tag is not set.");
  }

  return currently_cached_refs_.find(parameter_caching_ref) ==
         currently_cached_refs_.end();
}

util::Status Driver::SubmitParameterCachingRequest(
    const std::shared_ptr<Request>& request) {
  TRACE_SCOPE("Driver::SubmitParameterCachingRequest");
  auto parameter_caching_ref =
      request->GetPackageReference().ParameterCachingExecutableReference();

  current_parameter_caching_token_ =
      parameter_caching_ref->ParameterCachingToken();
  currently_cached_refs_.insert(parameter_caching_ref);

  ASSIGN_OR_RETURN(auto tpu_request,
                   DoCreateRequest(request, parameter_caching_ref,
                                   TpuRequest::RequestType::PARAMETER_CACHING));
  RETURN_IF_ERROR(tpu_request->SetDone([](int, const util::Status&) {}));

  // Record the submission time before actually submitting the workload. This
  // avoids race conditions where the completion is notified before submission.
  request->NotifySubmission(TpuRequest::RequestType::PARAMETER_CACHING);
  RETURN_IF_ERROR(DoSubmit(std::move(tpu_request)));

  return util::OkStatus();
}

void Driver::ResetCachedParameters() {
  current_parameter_caching_token_ = 0;
  currently_cached_refs_.clear();
}

void Driver::SchedulerWorker() {
  while (true) {
    {
      StdCondMutexLock lock(&scheduler_mutex_);
      while (!schedule_more_requests_ && !destructing_) {
        scheduler_wakeup_.wait(lock);
      }

      if (destructing_) {
        return;
      }

      schedule_more_requests_ = false;
    }

    ReaderMutexLock state_reader_lock(&state_mutex_);
    StdMutexLock submit_lock(&submit_mutex_);
    // TODO Improve handling of this error.
    CHECK_OK(TrySchedulePendingRequests());
  }
}

void Driver::HandleTpuRequestCompletion() {
  StdMutexLock lock(&scheduler_mutex_);
  schedule_more_requests_ = true;
  scheduler_wakeup_.notify_one();
}

util::Status Driver::TrySchedulePendingRequests() {
  for (auto& priority_and_queue : pending_requests_) {
    auto& request_queue = priority_and_queue.second;

    while (!request_queue.empty()) {
      ASSIGN_OR_RETURN(bool can_schedule,
                       CanScheduleTpuRequest(request_queue.front()));
      if (!can_schedule) {
        VLOG(5) << absl::StrFormat(
            "Already have %lld cycles in scheduler, no need to schedule more "
            "work.",
            MaxRemainingCycles());
        return util::OkStatus();
      }

      auto request = request_queue.front();
      VLOG(5) << absl::StrFormat(
          "Request [%d]: Scheduling one more TPU request that takes %lld "
          "cycles.",
          request->id(), request->EstimatedCyclesPerInference());

      RETURN_IF_ERROR(SubmitInferenceRequest(request));

      ASSIGN_OR_RETURN(auto remaining_tpu_requests,
                       request->RemainingTpuRequestCount());
      if (remaining_tpu_requests == 0) {
        VLOG(5) << StringPrintf(
            "Request [%d]: All TPU requests are now submitted.", request->id());
        request_queue.pop();
      }
    }
  }

  return util::OkStatus();
}

util::StatusOr<bool> Driver::CanScheduleTpuRequest(
    const std::shared_ptr<Request>& request) {
  if (request->GetPriority() == 0) {
    return util::InvalidArgumentError(
        "P0 requests should be immediately scheduled.");
  }

  if (max_scheduled_work_ns_ < 0) {
    VLOG(7) << StringPrintf(
        "max_scheduled_work_ns=%0.f, all requests are scheduled immediately.",
        max_scheduled_work_ns_);
    return true;
  }

  int64 remaining_cycles = MaxRemainingCycles();
  if (remaining_cycles == 0) {
    VLOG(7) << "Nothing is in the scheduler, submit one TPU request no matter "
               "what.";
    return true;
  }

  int64 max_cycles_to_schedule =
      static_cast<int64>(
          (max_scheduled_work_ns_ *
           static_cast<double>(operational_settings_.tpu_frequency_hz)) /
          1e9) -
      remaining_cycles;

  int64 total_cycles = request->EstimatedCyclesPerInference();
  ASSIGN_OR_RETURN(auto needs_parameter_caching,
                   NeedsParameterCaching(request));
  if (needs_parameter_caching) {
    total_cycles += request->GetPackageReference()
                        .ParameterCachingExecutableReference()
                        ->EstimatedCycles();
  }

  VLOG(7) << absl::StrFormat(
      "Request [%d]: Total cycles needed for scheduling a new inference: %lld, "
      "%lld available.",
      request->id(), total_cycles, max_cycles_to_schedule);
  return (max_cycles_to_schedule >= total_cycles);
}

util::Status Driver::CancelAllPendingRequests() {
  StdMutexLock submit_lock(&submit_mutex_);

  for (auto& priority_and_queue : pending_requests_) {
    auto& request_queue = priority_and_queue.second;

    while (!request_queue.empty()) {
      auto request = request_queue.front();
      ASSIGN_OR_RETURN(auto remaining_tpu_requests,
                       request->RemainingTpuRequestCount());
      VLOG(4) << StringPrintf(
          "Request [%d]: Cancelling %d remaining TPU requests.", request->id(),
          remaining_tpu_requests);

      RETURN_IF_ERROR(request->HandleTpuRequestsDone(
          util::CancelledError("Request cancelled."), remaining_tpu_requests));
      request_queue.pop();
    }
  }

  return util::OkStatus();
}

util::Status Driver::Execute(std::shared_ptr<api::Request> request) {
  BlockingCounter counter(1);
  util::Status final_status;

  auto done_callback = [&counter, &final_status](int id, util::Status status) {
    final_status = std::move(status);
    counter.DecrementCount();
  };

  // Submit asynchronously and wait.
  RETURN_IF_ERROR(Submit(std::move(request), std::move(done_callback)));

  counter.Wait();

  return final_status;
}

util::Status Driver::Execute(
    const std::vector<std::shared_ptr<api::Request>>& requests) {
  BlockingCounter counter(requests.size());
  std::mutex status_mutex;
  util::Status final_status;

  auto done_callback = [&counter, &final_status, &status_mutex](
                           int id, util::Status status) {
    StdMutexLock status_lock(&status_mutex);
    final_status.Update(status);
    counter.DecrementCount();
  };

  // Submit asynchronously and wait.
  for (auto request : requests) {
    RETURN_IF_ERROR(Submit(std::move(request), done_callback));
  }

  counter.Wait();
  return final_status;
}

util::Status Driver::Cancel(std::shared_ptr<api::Request> request) {
  return util::UnimplementedError("Unimplemented.");
}

util::Status Driver::CancelAllRequests() {
  return util::UnimplementedError("Unimplemented.");
}

util::Status Driver::Close(api::Driver::ClosingMode mode) {
  WriterMutexLock state_writer_lock(&state_mutex_);

  if (num_clients_ > 1) {
    num_clients_--;
    return util::Status();  // OK
  }

  if (state_ != kOpen) {
    return util::FailedPreconditionError(BadStateMessage(kOpen));
  }

  // Note our intention to close.
  RETURN_IF_ERROR(SetState(kClosing));

  // Before starting shutdown process in the lower layers of the stack, we
  // need to cancel all pending requests in the priority queue.
  RETURN_IF_ERROR(CancelAllPendingRequests());

  // If we are not in a rush, just clear the pending requests and let the ones
  // that have already started DMAing finish. If ASAP is enabled, we can skip
  // this step and a full cleanup of queues happens in DoClose.
  if (mode == api::Driver::ClosingMode::kGraceful) {
    RETURN_IF_ERROR(DoCancelAndWaitRequests(in_error_));
  }

  // Since chip is getting reset, anything cachedon SRAM will be wiped.
  {
    StdMutexLock submit_lock(&submit_mutex_);
    ResetCachedParameters();
  }

  // Actually close.
  RETURN_IF_ERROR(DoClose(in_error_, mode));

  num_clients_--;
  return SetState(kClosed);
}

void Driver::SetFatalErrorCallback(FatalErrorCallback callback) {
  fatal_error_callback_ = std::move(callback);
}

void Driver::SetThermalWarningCallback(ThermalWarningCallback callback) {
  thermal_warning_callback_ = std::move(callback);
}

void Driver::NotifyFatalError(const util::Status& status) {
  // Set error state.
  bool was_in_error = std::atomic_exchange(&in_error_, true);
  if (!was_in_error) {
    // Notify Error only the first time the fatal error is triggered.
    // TODO: Issue this is in a new detached thread to decouple
    // itself from other driver contexts.
    if (fatal_error_callback_) {
      fatal_error_callback_(status);
    }
  }
}

void Driver::HandleWatchdogTimeout() {
  LOG(ERROR) << "Watchdog timed out. Collecting runtime metrics.";
  auto status_or_request = GetOldestActiveRequest();
  if (!status_or_request.ok()) {
    // TODO: Log metric even if TpuRequest is not found.
    LOG(ERROR)
        << "No active request during watchdog timeout. Unable to log metrics.";
  } else {
    ExecutionContextInterface* context = status_or_request.ValueOrDie()
                                             ->executable_reference()
                                             .GetPackageReference()
                                             .GetExecutionContextInterface();
    GetTelemeterInterface()->LogWatchdogTimeout(*context);
  }

  LOG(ERROR) << "Watchdog activated, resetting TPU.";
  CHECK_OK(Close(api::Driver::ClosingMode::kAsap));
  CHECK_OK(Open(debug_mode_));
}

util::Status Driver::SetExecutableTiming(
    const api::PackageReference* executable, const api::Timing& timing) {
  return DoSetExecutableTiming(
      static_cast<const driver::PackageReference*>(executable)
          ->MainExecutableReference(),
      timing);
}

void Driver::UpdateOperationalSettings(const OperationalSettings& settings) {
  StdMutexLock lock(&submit_mutex_);
  operational_settings_ = settings;
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
