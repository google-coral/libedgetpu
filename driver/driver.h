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

#ifndef DARWINN_DRIVER_DRIVER_H_
#define DARWINN_DRIVER_DRIVER_H_

#include <atomic>
#include <map>
#include <memory>
#include <mutex>  // NOLINT
#include <queue>
#include <thread>  // NOLINT
#include <unordered_set>

#include "api/buffer.h"
#include "api/chip.h"
#include "api/driver.h"
#include "api/package_reference.h"
#include "api/request.h"
#include "api/telemeter_interface.h"
#include "driver/default_telemeter.h"
#include "driver/device_buffer_mapper.h"
#include "driver/memory/dma_direction.h"
#include "driver/package_registry.h"
#include "driver/request.h"
#include "driver/tpu_request.h"
#include "driver_shared/time_stamper/time_stamper.h"
#include "executable/executable_generated.h"
#include "port/integral_types.h"
#include "port/shared_mutex.h"
#include "port/statusor.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Base driver implementation.
class Driver : public api::Driver {
 public:
  ~Driver() override;

  bool IsOpen() const override;

  bool IsError() const override;

  util::Status Open(bool debug_mode = false, bool context_lost = false)
      LOCKS_EXCLUDED(state_mutex_) override;

  util::StatusOr<const api::PackageReference*> RegisterExecutableFile(
      const std::string& executable_filename) override;

  util::StatusOr<const api::PackageReference*> RegisterExecutableSerialized(
      const std::string& executable_content) override;

  util::StatusOr<const api::PackageReference*> RegisterExecutableSerialized(
      const char* executable_content, size_t length) override;

  util::Status UnregisterExecutable(const api::PackageReference* executable_ref)
      LOCKS_EXCLUDED(state_mutex_) override;

  util::StatusOr<std::shared_ptr<api::Request>> CreateRequest(
      const api::PackageReference*) override;

  // TODO If we end up spliting driver::Driver to 2 layers, this
  // method can go up a layer.
  util::Status Submit(std::shared_ptr<api::Request> request,
                      api::Request::Done done_callback)
      LOCKS_EXCLUDED(state_mutex_, submit_mutex_) override;

  util::Status Execute(std::shared_ptr<api::Request> request)
      LOCKS_EXCLUDED(state_mutex_, submit_mutex_) override;

  util::Status Execute(
      const std::vector<std::shared_ptr<api::Request>>& requests)
      LOCKS_EXCLUDED(state_mutex_, submit_mutex_) override;

  util::Status Cancel(std::shared_ptr<api::Request> request)
      LOCKS_EXCLUDED(state_mutex_) override;

  util::Status CancelAllRequests() LOCKS_EXCLUDED(state_mutex_) override;

  util::Status Close(api::Driver::ClosingMode mode)
      LOCKS_EXCLUDED(state_mutex_) override;

  void SetFatalErrorCallback(FatalErrorCallback callback) override;

  void SetThermalWarningCallback(ThermalWarningCallback callback) override;

  Buffer MakeBuffer(size_t size_bytes) const override;

  util::Status SetRealtimeMode(bool on) override {
    return DoSetRealtimeMode(on);
  }

  util::Status SetExecutableTiming(const api::PackageReference* executable,
                                   const api::Timing& timing) override;

  util::Status RemoveExecutableTiming(const api::PackageReference* executable) {
    return DoRemoveExecutableTiming(
        static_cast<const driver::PackageReference*>(executable)
            ->MainExecutableReference());
  }

  util::Status SetExecutionPreference(const api::PackageReference* package,
                                      ExecutionPreference preference) override {
    return util::OkStatus();
  }

  void SetTelemeterInterface(
      api::TelemeterInterface* telemeter_interface) override {
    telemeter_interface_ = telemeter_interface;
  };

  void UpdateOperationalSettings(const OperationalSettings& settings)
      LOCKS_EXCLUDED(submit_mutex_) override;

 protected:
  Driver(api::Chip chip, std::unique_ptr<PackageRegistry> executable_registry,
         const api::DriverOptions& driver_options,
         std::unique_ptr<driver_shared::TimeStamper> timestamper);

  // The base driver implementation does the necessary state checks and
  // validations before issuing the following calls that are implemented by the
  // derived class.

  virtual util::Status DoOpen(bool debug_mode)
      EXCLUSIVE_LOCKS_REQUIRED(state_mutex_) = 0;

  virtual util::Status DoClose(bool in_error, api::Driver::ClosingMode mode)
      EXCLUSIVE_LOCKS_REQUIRED(state_mutex_) = 0;

  // Cancels pending requests and waits for active requests to finish.
  virtual util::Status DoCancelAndWaitRequests(bool in_error)
      SHARED_LOCKS_REQUIRED(state_mutex_) = 0;

  virtual util::StatusOr<MappedDeviceBuffer> DoMapBuffer(const Buffer& buffer,
                                                         DmaDirection direction)
      SHARED_LOCKS_REQUIRED(state_mutex_) = 0;

  virtual util::StatusOr<std::shared_ptr<TpuRequest>> DoCreateRequest(
      const std::shared_ptr<Request> parent_request,
      const ExecutableReference* executable, TpuRequest::RequestType type)
      SHARED_LOCKS_REQUIRED(state_mutex_) = 0;

  virtual util::Status DoSetExecutableTiming(
      const ExecutableReference* executable, const api::Timing& timing) = 0;

  virtual util::Status DoRemoveExecutableTiming(
      const ExecutableReference* executable) {
    return util::FailedPreconditionError("Unsupported operation");
  }

  // TODO by just using RT scheduler everywhere, we can avoid the
  // complexity of having a capability query here.
  virtual bool HasImplementedRealtimeMode() const { return false; }

  virtual util::Status DoSetRealtimeMode(bool on) = 0;

  virtual util::Status DoSubmit(std::shared_ptr<TpuRequest> request)

      SHARED_LOCKS_REQUIRED(state_mutex_) = 0;

  virtual Buffer DoMakeBuffer(size_t size_bytes) const = 0;

  // Returns the upper bound estimation of driver on the number of cycles of
  // work remaining on the device.
  virtual int64 MaxRemainingCycles() const = 0;

  // Notifies that the driver / device has entered an error state.
  void NotifyFatalError(const util::Status& status);

  // Unregisters all the currently registered models.
  util::Status UnregisterAll() { return executable_registry_->UnregisterAll(); }

  // Unmaps all mapped parameters. This method typically needs to get called
  // before closing the MMU mapper.
  util::Status UnmapAllParameters() {
    return executable_registry_->UnmapAllParameters();
  }

  // Handler for when TPU watchdog expires. This signals an unexpected state in
  // TPU.
  void HandleWatchdogTimeout();

  // Gets called when a single TpuRequest has finished execution on the device.
  // This needs to be called in all sub-classes of Driver. It should be called
  // after MaxRemainingCycles is updated.
  void HandleTpuRequestCompletion();

  // Get the telemeter interface pointer.
  api::TelemeterInterface* GetTelemeterInterface() {
    return telemeter_interface_;
  }

  // Returns the oldest submitted request that's still active.
  virtual util::StatusOr<std::shared_ptr<TpuRequest>>
  GetOldestActiveRequest() const = 0;

 private:
  // Driver state. Transitions:
  //  kClosed -> kOpen -> kClosing -> kClosed.
  enum State {
    kOpen,     // Driver is Open.
    kClosing,  // Driver is Closing.
    kClosed,   // Driver is Closed. (Initial state.)
  };

  // Attempts a state transition to the given state.
  util::Status SetState(State next_state)
      EXCLUSIVE_LOCKS_REQUIRED(state_mutex_);

// Generate string to display for bad driver state errors.
  std::string BadStateMessage(State expected_state) const
      SHARED_LOCKS_REQUIRED(state_mutex_);

  // Internal helper for mapping parameters.
  util::Status MapParameters(PackageReference& package_ref)
      SHARED_LOCKS_REQUIRED(state_mutex_)
          EXCLUSIVE_LOCKS_REQUIRED(submit_mutex_);

  // Prepares and submits a single inference TpuRequest from the provided
  // request. It returns an error if there are no remaining TpuRequests to be
  // submitted.
  util::Status SubmitInferenceRequest(std::shared_ptr<Request> request)
      SHARED_LOCKS_REQUIRED(state_mutex_)
          EXCLUSIVE_LOCKS_REQUIRED(submit_mutex_);

  // Reset the state of cached parameters. This does not do anything to TPU
  // memory, only invalidates the cache state in driver which then results in
  // reloading parameters if needed.
  void ResetCachedParameters() SHARED_LOCKS_REQUIRED(state_mutex_)
      EXCLUSIVE_LOCKS_REQUIRED(submit_mutex_);

  // Checks if we need to load to-be-cached parameters to the TPU.
  util::StatusOr<bool> NeedsParameterCaching(
      const std::shared_ptr<Request>& request) const
      SHARED_LOCKS_REQUIRED(state_mutex_)
          EXCLUSIVE_LOCKS_REQUIRED(submit_mutex_);

  // Submits a parameter caching request and updates the records.
  util::Status SubmitParameterCachingRequest(
      const std::shared_ptr<Request>& request)
      SHARED_LOCKS_REQUIRED(state_mutex_)
          EXCLUSIVE_LOCKS_REQUIRED(submit_mutex_);

  // Schedules pending requests (if any) up to the limit we are allowed to have
  // tasks pending in the DMA scheduler. It returns OK status if there are no
  // more requests to be scheduled. It returns an error if there are any errors
  // in submitting requests.
  util::Status TrySchedulePendingRequests() SHARED_LOCKS_REQUIRED(state_mutex_)
      EXCLUSIVE_LOCKS_REQUIRED(submit_mutex_);

  // If a request is for a package with specified latency tolerance, it returns
  // a deadline_exceeded error if driver cannot guarantee that it finishes the
  // request in less than the tolerable latency.
  util::Status CheckLatencyTolerance(const std::shared_ptr<Request>& request)
      SHARED_LOCKS_REQUIRED(state_mutex_)
          EXCLUSIVE_LOCKS_REQUIRED(submit_mutex_);

  // Cleans up the priority queues by cancelling all pending requests.
  util::Status CancelAllPendingRequests() EXCLUSIVE_LOCKS_REQUIRED(state_mutex_)
      LOCKS_EXCLUDED(submit_mutex_);

  // Returns true if we can schedule one more inference for the provided request
  // given the current state of DMA scheduler, how long it takes for this
  // request on TPU and what our threshold for keeping the pipeline busy is.
  // This function should not be called for P0 requests. It always returns true
  // If there is no more work in DMA scheduler.
  util::StatusOr<bool> CanScheduleTpuRequest(
      const std::shared_ptr<Request>& request)
      SHARED_LOCKS_REQUIRED(state_mutex_)
          EXCLUSIVE_LOCKS_REQUIRED(submit_mutex_);

  // Updates scheduler with static timing estimation from registered executable.
  util::Status UpdateInitialTiming(
      const api::PackageReference* api_package_reference)
      LOCKS_EXCLUDED(submit_mutex_);

  // Runs the scheduler thread.
  void SchedulerWorker();

  // Maintains integrity of the driver state.
  mutable SharedMutex state_mutex_;

  // Guarantees that multiple requests will be submitted in the order provided.
  // NOTE: state_mutex_ cannot be acquired after submit_mutex_ is locked.
  mutable std::mutex submit_mutex_;

  // Counts the number of clients that opened this driver.
  int num_clients_ GUARDED_BY(state_mutex_){0};

  // Driver state.
  State state_ GUARDED_BY(state_mutex_){kClosed};

  // Chip that this driver controls.
  const api::Chip chip_;

  // Executable registry. Null, when device is in closed state.
  std::unique_ptr<PackageRegistry> executable_registry_;

  // Driver clock for timestamp reporting
  std::unique_ptr<driver_shared::TimeStamper> time_stamper_;

  // Registered fatal Error Callback.
  FatalErrorCallback fatal_error_callback_;

  // Registered thermal warning Callback.
  ThermalWarningCallback thermal_warning_callback_;

  // True, if device is in error state.
  std::atomic<bool> in_error_{false};

  // The currently active parameter-caching token. This token determines if a
  // new submission will require reloading cached parameters in TPU SRAM.
  uint64 current_parameter_caching_token_ GUARDED_BY(submit_mutex_);

  // A set of parameter-caching ExecutableReferences that shows if that model
  // has already cached its parameters on TPU SRAM, and the cache is still
  // valid.
  std::unordered_set<const ExecutableReference*> currently_cached_refs_
      GUARDED_BY(submit_mutex_);

  // Specifies if the driver is currently open in debug mode.
  bool debug_mode_;

  // A simple ID generator for requests.
  std::atomic<int> next_id_{0};

  // Current operational settings of the driver. Protected by submit_mutex to
  // avoid undefined behavior when it changes while an inference is being
  // submitted.
  OperationalSettings operational_settings_ GUARDED_BY(submit_mutex_);

  // The maximum amount of work (in terms of nanoseconds spent on TPU) that can
  // be scheduled in the DMA scheduler at any given point in time. -1 means no
  // maximum and all tasks get scheduled immediately. Exceptions are:
  //   1. P0 requests.
  //   2. When a single inference takes longer than this time and there is no
  //      other task scheduled (avoid starvation).
  const double max_scheduled_work_ns_;

  // The default telemeter implementation (all logging are NOPs). This is used
  // by default if no telemeter interface is set via SetTelemeterInterface.
  DefaultTelemeter default_telemeter_;

  // The interface to log telemetry. This object is owned by the caller.
  // telemeter_interface_ is initialized to default_telemeter_ in the
  // constructor, and can be set to the suitable telemter implementation via
  // SetTelemeterInterface().
  api::TelemeterInterface* telemeter_interface_;

  // A map of priority to queue of requests waiting to get scheduled. Priorities
  // are always 0 or larger and the larger the number the lower the priority.
  std::map<int, std::queue<std::shared_ptr<Request>>> pending_requests_;

  // The thread that runs scheduler for pending requests.
  std::thread scheduler_thread_;

  // Mutex to protect scheduler state.
  std::mutex scheduler_mutex_;

  // Condition variable to wake up the scheduler for doing more work or quitting
  // at destruction time.
  std::condition_variable scheduler_wakeup_;

  // If we want the scheduler to check and submit more of the pending requests (
  // if scheduling constraints are met of course).
  bool schedule_more_requests_ GUARDED_BY(scheduler_mutex_){false};

  // If we are destructing the class. This is used for the scheduler thread to
  // know when to quit.
  bool destructing_ GUARDED_BY(scheduler_mutex_){false};
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_DRIVER_H_
