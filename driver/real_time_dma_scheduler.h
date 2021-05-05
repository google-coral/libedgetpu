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

#ifndef DARWINN_DRIVER_REAL_TIME_DMA_SCHEDULER_H_
#define DARWINN_DRIVER_REAL_TIME_DMA_SCHEDULER_H_

#include <condition_variable>  // NOLINT
#include <list>
#include <memory>
#include <mutex>  // NOLINT
#include <queue>
#include <unordered_map>
#include <vector>

#include "api/driver.h"
#include "api/package_reference.h"
#include "api/timing.h"
#include "api/watchdog.h"
#include "driver/dma_info.h"
#include "driver/dma_scheduler.h"
#include "driver/package_registry.h"
#include "driver/single_queue_dma_scheduler.h"
#include "driver/tpu_request.h"
#include "driver_shared/time_stamper/time_stamper.h"
#include "port/errors.h"
#include "port/ptr_util.h"
#include "port/status.h"
#include "port/std_mutex_lock.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Manages DMA with best-effort QoS. Works as a gating function to the
// underlying single queue DMA.
class RealTimeDmaScheduler : public DmaScheduler {
 public:
  RealTimeDmaScheduler() = delete;
  RealTimeDmaScheduler(std::unique_ptr<api::Watchdog> watchdog,
                       std::unique_ptr<driver_shared::TimeStamper> time_stamper)
      : backing_scheduler_(
            gtl::MakeUnique<SingleQueueDmaScheduler>(std::move(watchdog))),
        time_stamper_(std::move(time_stamper)) {}
  ~RealTimeDmaScheduler() override = default;

  // Implements DmaScheduler interfaces.
  Status Open() override;
  Status Close(api::Driver::ClosingMode mode) override;

  // Submits a request. Forwards everything to the backing DMA scheduler (Single
  // queue DMA) in normal mode; accepts the request if real-time constraint can
  // be met, i.e. not impacting other service guarantees, otherwise rejects the
  // request in real-time mode. For details, please refer to
  // go/darwinn-qos-design.
  Status Submit(std::shared_ptr<TpuRequest> request) override
      LOCKS_EXCLUDED(mutex_);

  // DmaScheduler interface. Simply forward them to the backing scheduler.
  Status NotifyRequestCompletion() override;
  Status CancelPendingRequests() override {
    return backing_scheduler_->CancelPendingRequests();
  }
  Status WaitActiveRequests() override {
    return backing_scheduler_->WaitActiveRequests();
  }
  // Implements lower level DMA routines. They should be directly forwarded to
  // the backing driver.
  StatusOr<DmaDescriptorType> PeekNextDma() const override {
    return backing_scheduler_->PeekNextDma();
  }
  StatusOr<DmaInfo *> GetNextDma() override {
    return backing_scheduler_->GetNextDma();
  }
  Status NotifyDmaCompletion(DmaInfo *dma_info) override {
    return backing_scheduler_->NotifyDmaCompletion(dma_info);
  }
  bool IsEmpty() const override {
    return backing_scheduler_->IsEmpty();
  }
  int64 MaxRemainingCycles() const override {
    return backing_scheduler_->MaxRemainingCycles();
  }
  StatusOr<std::shared_ptr<TpuRequest>> GetOldestActiveRequest()
      const override {
    return backing_scheduler_->GetOldestActiveRequest();
  }

  // Enters/leaves real-time mode. Note timing is preserved across toggling.
  void SetRealtimeMode(bool on) LOCKS_EXCLUDED(mutex_);

  // Clears all timing information.
  void ResetTiming() LOCKS_EXCLUDED(mutex_) {
    StdMutexLock lock(&mutex_);
    inference_timings_.clear();
  }

  // Sets expected arrival rates, max execution time and tolerance (in
  // milliseconds) for an executable reference.
  // -1 in any of the fields of api::Timing means keeping that individual value
  // unchanged but updating the rest.
  Status SetExecutableTiming(const ExecutableReference *executable,
                             const api::Timing &timing) LOCKS_EXCLUDED(mutex_);

  // Removes timing information for a registered model.
  Status RemoveExecutableTiming(const ExecutableReference *executable)
      LOCKS_EXCLUDED(mutex_);

  // Returns the arrival rate and FPS of a given executable reference.
  StatusOr<api::Timing> GetExecutableTiming(
      const ExecutableReference *executable) const LOCKS_EXCLUDED(mutex_);

  // Returns the arrival time of last request for a given executable reference.
  int64 GetLastArrivalTime(const ExecutableReference *executable) const
      LOCKS_EXCLUDED(mutex_);

  StatusOr<int> GetExecutableFPS(const ExecutableReference *executable) const {
    ASSIGN_OR_RETURN(const auto timing, GetExecutableTiming(executable));
    return timing.fps;
  }

  StatusOr<int> GetExecutableMaxExecutionTimeMs(
      const ExecutableReference *executable) const {
    ASSIGN_OR_RETURN(const auto timing, GetExecutableTiming(executable));
    return timing.max_execution_time_ms;
  }

  StatusOr<int> GetExecutableToleranceMs(
      const ExecutableReference *executable) const {
    ASSIGN_OR_RETURN(const auto timing, GetExecutableTiming(executable));
    return timing.tolerance_ms;
  }

 private:
  // Tracks timing requirements and statistics for a registered executable.
  // See go/darwinn-qos-design on the algorithm.
  struct TimingInternal : public api::Timing {
    TimingInternal() : api::Timing() {}
    explicit TimingInternal(api::Timing timing) : api::Timing{timing} {}

    // Returns max execution time in microseconds.
    int64 max_execution_time_us() const { return max_execution_time_ms * 1000; }

    // Returns tolerance in microseconds.
    int64 tolerance_us() const { return tolerance_ms * 1000; }

    // Returns per frame time in microseconds, or error when FPS == 0.
    StatusOr<int64> frame_time_us() const {
      if (fps == 0) {
        return InvalidArgumentError("Can't calculate frame time of 0 FPS");
      }
      return 1e6 / fps;
    }

    // Returns if a timing configuration is real-time.
    bool HasRealTimeRequirements() const { return fps > 0; }

    int64 last_arrival_time_us{0};
    int64 last_completion_time_us{0};
  };

  // The underlying single queue DMA scheduler. Thread-safe by itself.
  std::unique_ptr<SingleQueueDmaScheduler> backing_scheduler_;

  // Time-stamper for tracking submission and completion time for requests.
  std::unique_ptr<driver_shared::TimeStamper> time_stamper_;

  // Tracks registered executables.
  std::unordered_map<const ExecutableReference *, TimingInternal>
      inference_timings_ GUARDED_BY(mutex_);

  // Real-time mode? In non-real-time mode, this scheduler behaves the same as
  // the underlying scheduler (single queue DMA).
  bool real_time_mode_ GUARDED_BY(mutex_){false};

  // Currently booked time by all scheduled inferences.
  int64 time_booked_us_ GUARDED_BY(mutex_){0};

  // Guards the inference timings.
  mutable std::mutex mutex_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_SINGLE_QUEUE_DMA_SCHEDULER_H_
