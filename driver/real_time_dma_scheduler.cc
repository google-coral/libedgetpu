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

#include "driver/real_time_dma_scheduler.h"

#include "absl/strings/str_format.h"
#include "port/errors.h"
#include "port/logging.h"
#include "port/statusor.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

Status RealTimeDmaScheduler::Open() { return backing_scheduler_->Open(); }

Status RealTimeDmaScheduler::Close(api::Driver::ClosingMode mode) {
  ResetTiming();
  return backing_scheduler_->Close(mode);
}

Status RealTimeDmaScheduler::Submit(std::shared_ptr<TpuRequest> request) {
  StdMutexLock lock(&mutex_);
  if (!real_time_mode_) {
    return backing_scheduler_->Submit(request);
  }

  const auto* executable_ref = &request->executable_reference();
  const int64 time_now_us = time_stamper_->GetTimeMicroSeconds();

  // TODO: We allocate Timing information every time a new
  // executable requesting inference, but we need to decide a good way and when
  // to cleanup the timing information.
  auto& cur_timing = inference_timings_[executable_ref];

  // Updating the arrival time regardless whether we can schedule it or not.
  cur_timing.last_arrival_time_us = time_now_us;

  // A normal process w/o max execution time cannot be scheduled at this time.
  // TODO: move such processes into a queue that would be submitted
  // when leaving real-time mode.
  if (cur_timing.max_execution_time_ms == 0) {
    if (cur_timing.fps != 0) {
      // FPS > 0, and 0 MET: ill-formed Timing information.
      return InvalidArgumentError(
          "Unable to submit under real-time mode. "
          "Ill-formed timing information: FPS > 0 but MET == 0.");
    }
    return DeadlineExceededError(
        "Normal process without MET cannot be scheduled in real-time mode.");
  }

  // Can we submit? Computing T_deadline using the basic algorithm.
  int64 deadline_us = INT64_MAX;
  time_booked_us_ = std::max(time_now_us, time_booked_us_);

  for (const auto& inference_timing : inference_timings_) {
    if (inference_timing.first == executable_ref) {
      continue;  // No need to count self.
    }
    const TimingInternal& timing = inference_timing.second;
    if (!timing.HasRealTimeRequirements()) {
      // For normal models, assuming no ETA of next request.
      continue;
    }
    if (timing.last_arrival_time_us == 0) {
      continue;  // The model has no past inferences. Skip.
    }

    ASSIGN_OR_RETURN(const int64 frame_time_us, timing.frame_time_us());
    const int64 time_next_us =
        timing.last_arrival_time_us + frame_time_us +
        std::min(timing.tolerance_us(),
                 frame_time_us - timing.max_execution_time_us());
    // If we missed two frames already, assume it's no longer arriving.
    // TODO: verify if that assumption holds.
    if (time_next_us + 2 * frame_time_us < time_now_us) {
      continue;
    }

    deadline_us = std::min(deadline_us, time_next_us);
  }

  if (deadline_us > time_booked_us_ + cur_timing.max_execution_time_us()) {
    // Ok to schedule; add this to booked time.
    time_booked_us_ += cur_timing.max_execution_time_us();
    return backing_scheduler_->Submit(request);
  } else {
    return DeadlineExceededError(
        "The request cannot be scheduled within given time budget.");
  }
}

void RealTimeDmaScheduler::SetRealtimeMode(bool on) {
  StdMutexLock lock(&mutex_);
  real_time_mode_ = on;
}

int64 RealTimeDmaScheduler::GetLastArrivalTime(
    const ExecutableReference* executable) const {
  StdMutexLock lock(&mutex_);

  auto found = inference_timings_.find(executable);
  if (found == inference_timings_.end()) {
    return 0;
  } else {
    return found->second.last_arrival_time_us;
  }
}

Status RealTimeDmaScheduler::RemoveExecutableTiming(
    const ExecutableReference* executable) {
  if (executable == nullptr) {
    return InvalidArgumentError("Null executable reference.");
  }

  StdMutexLock lock(&mutex_);
  // It is not an error if we don't have timing for an executable: for older
  // binaries they might not have the estimated clock cycle information and thus
  // may or may not have been registered here.
  inference_timings_.erase(executable);
  return OkStatus();
}

Status RealTimeDmaScheduler::SetExecutableTiming(
    const ExecutableReference* executable, const api::Timing& timing) {
  VLOG(3) << "RealTimeDmaScheduler: received timing setting: " << timing.Dump();
  if (!executable) {
    return InvalidArgumentError("Null executable reference.");
  }

  api::Timing candidate_timing = timing;
  StdMutexLock lock(&mutex_);
  auto timing_entry = inference_timings_.find(executable);
  if (timing_entry != inference_timings_.end()) {
    // If we have initial timing information, we could allow incremental
    // updates on individual timing fields.
    TimingInternal& existing_timing = timing_entry->second;
    if (candidate_timing.fps < 0) {
      candidate_timing.fps = existing_timing.fps;
    }
    if (candidate_timing.max_execution_time_ms < 0) {
      candidate_timing.max_execution_time_ms =
          existing_timing.max_execution_time_ms;
    }
    if (candidate_timing.tolerance_ms < 0) {
      candidate_timing.tolerance_ms = existing_timing.tolerance_ms;
    }
  } else {
    // Setting this executable's initial timing.
    // FPS can be zero, which means a normal process w/o expected arrival rate.
    // However, they cannot be negative.
    if (candidate_timing.fps < 0 ||
        candidate_timing.max_execution_time_ms < 0 ||
        candidate_timing.tolerance_ms < 0) {
      return InvalidArgumentError("Bad timing value(s).");
    }
  }

  const TimingInternal timing_internal(candidate_timing);

  // Checks applicable to models with service guarantee requirements.
  if (timing_internal.HasRealTimeRequirements()) {
    ASSIGN_OR_RETURN(const int64 frame_time_us,
                     timing_internal.frame_time_us());
    // If FPS > 0, MET has to be > 0.
    if (timing_internal.max_execution_time_ms == 0) {
      return InvalidArgumentError(StringPrintf(
          "Invalid max execution time: %dms.", timing.max_execution_time_ms));
    }
    if (frame_time_us < timing_internal.max_execution_time_us()) {
      return InvalidArgumentError(absl::StrFormat(
          "Max execution time (%lldus) exceeds frame time (%lldus).",
          timing_internal.max_execution_time_us(), frame_time_us));
    }
    // Tolerance cannot be greater than frame time - MET (i.e. cannot push
    // expected finish time beyond one frame).
    if (timing_internal.tolerance_us() >
        (frame_time_us - timing_internal.max_execution_time_us())) {
      return InvalidArgumentError(absl::StrFormat(
          "Invalid tolerance (%lldus). Needs to be less than %lldus to fit in "
          "one frame.",
          timing_internal.tolerance_us(),
          frame_time_us - timing_internal.max_execution_time_us()));
    }
  }

  inference_timings_[executable] = timing_internal;
  VLOG(3) << "RealTimeDmaScheduler: applied timing setting: "
          << timing_internal.Dump();
  return OkStatus();
}

StatusOr<api::Timing> RealTimeDmaScheduler::GetExecutableTiming(
    const ExecutableReference* executable) const {
  StdMutexLock lock(&mutex_);

  if (!executable) {
    return InvalidArgumentError("Null executable reference.");
  }

  auto found = inference_timings_.find(executable);
  if (found == inference_timings_.end()) {
    return NotFoundError(
        "Given executable reference has no associated timing information.");
  } else {
    return found->second;
  }
}

Status RealTimeDmaScheduler::NotifyRequestCompletion() {
  // TODO: update MET dynamically.
  return backing_scheduler_->NotifyRequestCompletion();
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
