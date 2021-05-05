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

#ifndef DARWINN_DRIVER_SINGLE_QUEUE_DMA_SCHEDULER_H_
#define DARWINN_DRIVER_SINGLE_QUEUE_DMA_SCHEDULER_H_

#include <condition_variable>  // NOLINT
#include <list>
#include <memory>
#include <mutex>  // NOLINT
#include <queue>
#include <vector>

#include "api/driver.h"
#include "api/watchdog.h"
#include "driver/dma_info.h"
#include "driver/dma_scheduler.h"
#include "driver/tpu_request.h"
#include "port/status.h"
#include "port/std_mutex_lock.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Manages the processing order of DMAs with single queue. All DMAs are
// serialized. Thread-safe.
class SingleQueueDmaScheduler : public DmaScheduler {
 public:
  SingleQueueDmaScheduler(std::unique_ptr<api::Watchdog> watchdog)
      : watchdog_(std::move(watchdog)) {}
  ~SingleQueueDmaScheduler() override = default;

  // Implements DmaScheduler interfaces.
  Status Open() override LOCKS_EXCLUDED(mutex_);
  Status Close(api::Driver::ClosingMode mode) override LOCKS_EXCLUDED(mutex_);
  Status Submit(std::shared_ptr<TpuRequest> request) override
      LOCKS_EXCLUDED(mutex_);
  StatusOr<DmaDescriptorType> PeekNextDma() const override
      LOCKS_EXCLUDED(mutex_);
  StatusOr<DmaInfo*> GetNextDma() override LOCKS_EXCLUDED(mutex_);
  Status NotifyDmaCompletion(DmaInfo* dma_info) override LOCKS_EXCLUDED(mutex_);
  Status NotifyRequestCompletion() override LOCKS_EXCLUDED(mutex_);
  Status CancelPendingRequests() override LOCKS_EXCLUDED(mutex_);
  Status WaitActiveRequests() override LOCKS_EXCLUDED(mutex_);
  bool IsEmpty() const override LOCKS_EXCLUDED(mutex_) {
    StdMutexLock lock(&mutex_);
    return IsEmptyLocked();
  }
  int64 MaxRemainingCycles() const override LOCKS_EXCLUDED(mutex_);
  StatusOr<std::shared_ptr<TpuRequest>> GetOldestActiveRequest() const override
      LOCKS_EXCLUDED(mutex_);

 private:
  // A data structure for managing Request and associated DMAs.
  struct Task {
    Task(std::shared_ptr<TpuRequest> request, std::list<DmaInfo>&& dmas)
        : request(std::move(request)), dmas(std::move(dmas)) {}

    // This type is movable.
    Task(Task&& other)
        : request(std::move(other.request)), dmas(std::move(other.dmas)) {}
    Task& operator=(Task&& other) {
      if (this != &other) {
        request = std::move(other.request);
        dmas = std::move(other.dmas);
      }
      return *this;
    }

    // Returns the associated TpuRequest.
    std::shared_ptr<TpuRequest> GetTpuRequest() const {
      return request;
    }

    // Request.
    std::shared_ptr<TpuRequest> request;

    // DMAs to be performed to serve request. std::list is intentionally used to
    // have valid pointers while other members removed.
    std::list<DmaInfo> dmas;
  };

  // A data structure for keeping track of DMA and its associated request.
  struct PendingDma {
    // DMA.
    DmaInfo* info;

    // Related request.
    TpuRequest* request;
  };

  // Validates whether in "is_open" state.
  Status ValidateOpenState(bool is_open) const EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Locked version of IsEmpty().
  bool IsEmptyLocked() const EXCLUSIVE_LOCKS_REQUIRED(mutex_) {
    return pending_tasks_.empty() && active_tasks_.empty() &&
           pending_dmas_.empty();
  }

  // Handles all completed DMAs related cleanups for given tasks.
  Status HandleCompletedTasks() LOCKS_EXCLUDED(mutex_);
  Status HandleActiveTasks() EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Waits until all active DMAs are completed.
  Status CloseActiveDmas() LOCKS_EXCLUDED(mutex_);

  // Cancels all the active DMAs and requests.
  Status CancelActiveRequests() LOCKS_EXCLUDED(mutex_);

  // Cancels all the tasks in a provided queue.
  Status CancelTaskQueue(std::deque<Task>& tasks)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Guards all the related queues.
  mutable std::mutex mutex_;

  // A notification to wait for all active requests to complete.
  std::condition_variable wait_active_requests_complete_;

  // A notification to wait for all active dmas to complete.
  std::condition_variable wait_active_dmas_complete_;

  // Tracks open state.
  bool is_open_ GUARDED_BY(mutex_){false};

  // Pending tasks that have not yet performed any DMAs to DarwiNN device.
  std::deque<Task> pending_tasks_ GUARDED_BY(mutex_);

  // Active tasks that have delivered DMAs fully or partially to DarwiNN device.
  std::deque<Task> active_tasks_ GUARDED_BY(mutex_);

  // Completed tasks that may have few active on-going DMAs.
  std::deque<Task> completed_tasks_ GUARDED_BY(mutex_);

  // DMAs belonging to active requests that are not yet served.
  std::queue<PendingDma> pending_dmas_ GUARDED_BY(mutex_);

  // A watchdog passed down from the driver to keep track of TPU being active.
  // DmaScheduler is responsible for activating the watchdog whenever a task
  // enters active queue and de-activating it when the queue is empty.
  std::unique_ptr<api::Watchdog> watchdog_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_SINGLE_QUEUE_DMA_SCHEDULER_H_
