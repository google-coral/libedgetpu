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

#include "driver/single_queue_dma_scheduler.h"

#include <string>
#include <utility>

#include "api/driver.h"
#include "api/watchdog.h"
#include "driver/tpu_request.h"
#include "port/errors.h"
#include "port/logging.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"
#include "port/tracing.h"

namespace platforms {
namespace darwinn {
namespace driver {

Status SingleQueueDmaScheduler::ValidateOpenState(bool is_open) const {
  if (is_open_ != is_open) {
    return FailedPreconditionError(
        StringPrintf("Bad state: expected=%d, actual=%d", is_open, is_open_));
  }
  return Status();  // OK
}

Status SingleQueueDmaScheduler::Open() {
  StdMutexLock lock(&mutex_);
  if (!IsEmptyLocked()) {
    return FailedPreconditionError("DMA queues are not empty");
  }
  RETURN_IF_ERROR(ValidateOpenState(/*is_open=*/false));
  is_open_ = true;
  RETURN_IF_ERROR(watchdog_->Deactivate());
  return Status();  // OK
}

Status SingleQueueDmaScheduler::Close(api::Driver::ClosingMode mode) {
  {
    StdMutexLock lock(&mutex_);
    RETURN_IF_ERROR(ValidateOpenState(/*is_open=*/true));
    while (!pending_dmas_.empty()) {
      pending_dmas_.pop();
    }
  }

  Status status;
  status.Update(CancelPendingRequests());
  if (mode == api::Driver::ClosingMode::kAsap) {
    status.Update(CancelActiveRequests());
  } else {
    status.Update(CloseActiveDmas());
  }

  StdMutexLock lock(&mutex_);
  is_open_ = false;
  return status;
}

Status SingleQueueDmaScheduler::Submit(std::shared_ptr<TpuRequest> request) {
  TRACE_SCOPE("SingleQueueDmaScheduler::Submit");
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateOpenState(/*is_open=*/true));

  RETURN_IF_ERROR(request->NotifyRequestSubmitted());
  VLOG(3) << StringPrintf("Request[%d]: Submitted", request->id());
  ASSIGN_OR_RETURN(auto dmas, request->GetDmaInfos());
  pending_tasks_.push_back({std::move(request), std::move(dmas)});

  return Status();  // OK
}

StatusOr<DmaDescriptorType> SingleQueueDmaScheduler::PeekNextDma() const {
  TRACE_SCOPE("SingleQueueDmaScheduler::PeekNextDma");
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateOpenState(/*is_open=*/true));
  if (pending_dmas_.empty() && pending_tasks_.empty()) {
    return DmaDescriptorType::kLocalFence;
  }

  if (pending_dmas_.empty()) {
    return pending_tasks_.front().dmas.front().type();
  } else {
    return pending_dmas_.front().info->type();
  }
}

StatusOr<DmaInfo*> SingleQueueDmaScheduler::GetNextDma() {
  TRACE_SCOPE("SingleQueueDmaScheduler::GetNextDma");
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateOpenState(/*is_open=*/true));
  if (pending_dmas_.empty() && pending_tasks_.empty()) {
    return nullptr;
  }

  if (pending_dmas_.empty()) {
    auto& task = pending_tasks_.front();
    RETURN_IF_ERROR(task.request->NotifyRequestActive());
    TpuRequest* request = task.request.get();
    for (auto& dma : task.dmas) {
      pending_dmas_.push({&dma, request});
    }
    active_tasks_.push_back(std::move(task));
    pending_tasks_.pop_front();
    RETURN_IF_ERROR(watchdog_->Activate().status());
  }

  // If fenced, return empty DMAs.
  const auto& pending_front = pending_dmas_.front();
  if (pending_front.info->type() == DmaDescriptorType::kLocalFence ||
      pending_front.info->type() == DmaDescriptorType::kGlobalFence) {
    return nullptr;
  }

  pending_front.info->MarkActive();
  VLOG(7) << StringPrintf("Request[%d]: Scheduling DMA[%d]",
                          pending_front.request->id(),
                          pending_front.info->id());

  auto* next_dma = pending_front.info;
  pending_dmas_.pop();
  return next_dma;
}

Status SingleQueueDmaScheduler::NotifyDmaCompletion(DmaInfo* dma_info) {
  TRACE_SCOPE("SingleQueueDmaScheduler::NotifyDmaCompletion");
  if (!dma_info->IsActive()) {
    const auto dma_dump = dma_info->Dump();
    return FailedPreconditionError(
        StringPrintf("Cannot complete inactive DMA: %s", dma_dump.c_str()));
  }

  {
    StdMutexLock lock(&mutex_);
    RETURN_IF_ERROR(ValidateOpenState(/*is_open=*/true));

    dma_info->MarkCompleted();
    VLOG(7) << StringPrintf("Completing DMA[%d]", dma_info->id());
  }

  RETURN_IF_ERROR(HandleCompletedTasks());

  StdMutexLock lock(&mutex_);
  wait_active_dmas_complete_.notify_all();
  if (pending_dmas_.empty()) {
    return Status();  // OK
  }

  const auto& pending_front = pending_dmas_.front();
  if (pending_front.info->type() != DmaDescriptorType::kLocalFence) {
    return Status();  // OK
  }

  // Clear local fence if completed.
  RETURN_IF_ERROR(HandleActiveTasks());
  if (pending_front.info->IsCompleted()) {
    VLOG(7) << StringPrintf("Request[%d]: Local fence done",
                            pending_front.request->id());
    pending_dmas_.pop();
  }
  return Status();  // OK
}

Status SingleQueueDmaScheduler::NotifyRequestCompletion() {
  TRACE_SCOPE("SingleQueueDmaScheduler::NotifyRequestCompletion");

  // This region holds the lock since it needs to deal with task and DMA queues.
  // As a result, we may need to call NotifyCompletion() on the request for
  // which we do not need the lock. In such case that request will be moved to
  // request_to_be_notified.
  std::shared_ptr<TpuRequest> request_to_be_notified;
  {
    StdMutexLock lock(&mutex_);

    RETURN_IF_ERROR(ValidateOpenState(/*is_open=*/true));
    if (active_tasks_.empty()) {
      return FailedPreconditionError("No active request to complete");
    }

    // Requests are always handled in FIFO order.
    TpuRequest* completed_request = active_tasks_.front().request.get();
    if (!pending_dmas_.empty()) {
      const auto& pending_front = pending_dmas_.front();

      if (pending_front.request == completed_request) {
        // Clear global fence if exists.
        if (pending_front.info->type() == DmaDescriptorType::kGlobalFence) {
          VLOG(7) << StringPrintf("Request[%d]: Global fence done",
                                  completed_request->id());
          pending_front.info->MarkCompleted();
          pending_dmas_.pop();
        } else {
          return FailedPreconditionError(
              StringPrintf("Request[%d] is completing while DMAs are pending.",
                           completed_request->id()));
        }
      }
    }

    RETURN_IF_ERROR(HandleActiveTasks());
    Task next_front = std::move(active_tasks_.front());
    active_tasks_.pop_front();

    RETURN_IF_ERROR(watchdog_->Signal());
    if (active_tasks_.empty()) {
      RETURN_IF_ERROR(watchdog_->Deactivate());
    }

    if (next_front.dmas.empty() && completed_tasks_.empty()) {
      request_to_be_notified = std::move(next_front.request);
    } else {
      completed_tasks_.push_back(std::move(next_front));
    }
  }

  if (request_to_be_notified) {
    RETURN_IF_ERROR(request_to_be_notified->NotifyCompletion(OkStatus()));
    VLOG(3) << StringPrintf("Request[%d]: Completed",
                            request_to_be_notified->id());
    wait_active_requests_complete_.notify_all();
  }

  return OkStatus();
}

Status SingleQueueDmaScheduler::CancelPendingRequests() {
  Status status;
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateOpenState(/*is_open=*/true));
  status.Update(CancelTaskQueue(pending_tasks_));
  return status;
}

Status SingleQueueDmaScheduler::WaitActiveRequests() {
  TRACE_SCOPE("SingleQueueDmaScheduler::WaitActiveRequests");
  StdCondMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateOpenState(/*is_open=*/true));
  while (!completed_tasks_.empty() || !active_tasks_.empty()) {
    VLOG(3) << StringPrintf("Waiting for %zd more active requests",
                            completed_tasks_.size() + active_tasks_.size());
    wait_active_requests_complete_.wait(lock);
  }
  return Status();  // OK
}

int64 SingleQueueDmaScheduler::MaxRemainingCycles() const {
  StdMutexLock lock(&mutex_);
  int64 cycles = 0;
  for (const auto& task : pending_tasks_) {
    cycles += task.request->executable_reference().EstimatedCycles();
  }
  for (const auto& task : active_tasks_) {
    cycles += task.request->executable_reference().EstimatedCycles();
  }
  return cycles;
}

Status SingleQueueDmaScheduler::HandleCompletedTasks() {
  TRACE_SCOPE("SingleQueueDmaScheduler::HandleCompletedTasks");

  std::vector<std::shared_ptr<TpuRequest>> completed_requests;
  bool notify = false;

  // We need to lock the mutex for doing queue operations but running request
  // callbacks don't need that lock. Therefore we push the completed requests
  // into a vector and run their NotifyCompletion methods outside of the lock
  // zone.
  {
    StdMutexLock lock(&mutex_);

    if (completed_tasks_.empty()) {
      return OkStatus();
    }

    completed_tasks_.front().dmas.remove_if(
        [](const DmaInfo& dma_info) { return dma_info.IsCompleted(); });

    // Complete tasks, whose DMAs are all completed.
    while (completed_tasks_.front().dmas.empty()) {
      auto& front_task = completed_tasks_.front();
      VLOG(3) << StringPrintf("Request[%d]: Completed",
                              front_task.request->id());
      completed_requests.push_back(std::move(front_task.request));
      completed_tasks_.pop_front();

      if (completed_tasks_.empty()) {
        notify = true;
        break;
      }

      completed_tasks_.front().dmas.remove_if(
          [](const DmaInfo& dma_info) { return dma_info.IsCompleted(); });
    }
  }

  for (auto& request : completed_requests) {
    RETURN_IF_ERROR(request->NotifyCompletion(OkStatus()));
  }

  if (notify) {
    wait_active_requests_complete_.notify_all();
  }

  return OkStatus();
}

Status SingleQueueDmaScheduler::HandleActiveTasks() {
  TRACE_SCOPE("SingleQueueDmaScheduler::HandleActiveTasks");
  if (active_tasks_.empty()) {
    return Status();  // OK
  }

  auto& front_task = active_tasks_.front();
  front_task.dmas.remove_if(
      [](const DmaInfo& dma_info) { return dma_info.IsCompleted(); });

  if (front_task.dmas.empty()) {
    return Status();  // OK
  }

  auto& front_dma = front_task.dmas.front();
  // If first remaining DMA is local fence, mark it completed.
  if (front_dma.type() == DmaDescriptorType::kLocalFence) {
    front_dma.MarkCompleted();
  }
  return Status();  // OK
}

Status SingleQueueDmaScheduler::CloseActiveDmas() {
  TRACE_SCOPE("SingleQueueDmaScheduler::CloseActiveDmas");
  StdCondMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateOpenState(/*is_open=*/true));
  while (!completed_tasks_.empty()) {
    completed_tasks_.front().dmas.remove_if(
        [](const DmaInfo& info) { return !info.IsActive(); });
    if (completed_tasks_.front().dmas.empty()) {
      completed_tasks_.pop_front();
    }

    if (completed_tasks_.empty()) {
      break;
    }
    wait_active_dmas_complete_.wait(lock);
  }
  while (!active_tasks_.empty()) {
    active_tasks_.front().dmas.remove_if(
        [](const DmaInfo& info) { return !info.IsActive(); });
    if (active_tasks_.front().dmas.empty()) {
      active_tasks_.pop_front();
      RETURN_IF_ERROR(watchdog_->Signal());
    }

    if (active_tasks_.empty()) {
      RETURN_IF_ERROR(watchdog_->Deactivate());
      break;
    }

    wait_active_dmas_complete_.wait(lock);
  }
  return Status();  // OK
}

Status SingleQueueDmaScheduler::CancelActiveRequests() {
  Status status;
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateOpenState(/*is_open=*/true));

  status.Update(CancelTaskQueue(active_tasks_));
  status.Update(CancelTaskQueue(completed_tasks_));
  while (!pending_dmas_.empty()) {
    pending_dmas_.pop();
  }

  RETURN_IF_ERROR(watchdog_->Deactivate());

  return status;
}

Status SingleQueueDmaScheduler::CancelTaskQueue(std::deque<Task>& tasks) {
  Status status;
  while (!tasks.empty()) {
    status.Update(tasks.front().request->Cancel());
    tasks.pop_front();
  }
  return status;
}

StatusOr<std::shared_ptr<TpuRequest>>
SingleQueueDmaScheduler::GetOldestActiveRequest() const {
  StdMutexLock lock(&mutex_);
  if (active_tasks_.empty()) {
    return UnknownError(
        "No requests active when querying for oldest active request.");
  }

  return active_tasks_.front().GetTpuRequest();
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
