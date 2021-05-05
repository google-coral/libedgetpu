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

#include "driver/request.h"

#include "api/request.h"
#include "driver_shared/time_stamper/time_stamper.h"
#include "port/math_util.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/statusor.h"
#include "port/std_mutex_lock.h"
#include "port/time.h"
#include "port/tracing.h"

namespace platforms {
namespace darwinn {
namespace driver {
Request::Request(int id, const PackageReference& package_ref,
                 const driver_shared::TimeStamper& timestamper)
    : id_(id),
      package_ref_(package_ref),
      main_executable_ref_(*package_ref.MainExecutableReference()),
      hardware_batch_size_(package_ref.MainExecutableReference()->BatchSize()),
      current_time_(timestamper) {
  timing_.created_ns = timestamper.GetTimeNanoSeconds();
  timing_.submitted_ns = -1;
  timing_.completed_ns = -1;
}

Status Request::AddInput(const std::string& name, const Buffer& input) {
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateState(kInitial));

  RETURN_IF_ERROR(main_executable_ref_.ValidateInput(name, input));
  VLOG(3) << StringPrintf("Adding input \"%s\" with %zu bytes.", name.c_str(),
                          input.size_bytes());
  inputs_[name].push_back(input);
  return OkStatus();
}

Status Request::AddOutput(const std::string& name, const Buffer output) {
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateState(kInitial));

  RETURN_IF_ERROR(main_executable_ref_.ValidateOutput(name, output));
  VLOG(3) << StringPrintf("Adding output \"%s\" with %zu bytes.", name.c_str(),
                          output.size_bytes());
  outputs_[name].push_back(output);
  return OkStatus();
}

Status Request::SetPriority(int priority) {
  if (priority < 0) {
    return InvalidArgumentError(StringPrintf(
        "Priority must be 0 or greater. %d was provided.", priority));
  }

  StdMutexLock lock(&mutex_);
  priority_ = priority;
  return OkStatus();
}

StatusOr<Request::Timing> Request::GetTiming() const {
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateState(kDone));
  return timing_;
}

int Request::GetPriority() const {
  StdMutexLock lock(&mutex_);
  return priority_;
}

Status Request::SetDone(Done done) {
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateState(kInitial));

  if (done_) {
    return InvalidArgumentError("Done callback is already set.");
  }

  done_ = std::move(done);
  return OkStatus();
}

Status Request::Prepare() {
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateState(kInitial));

  if (!done_) {
    return InvalidArgumentError("Done callback is not set.");
  }

  // Batch size is inferred from the number of input and output buffers provided
  // for each input and output layer. There are special cases where an
  // executable may have no inputs and outputs (e.g. test executables) in which
  // case we assume batch size 1.
  if (main_executable_ref_.NumInputLayers() == 0 &&
      main_executable_ref_.NumOutputLayers() == 0) {
    request_batch_size_ = 1;
    required_tpu_request_count_ = 1;
    pending_tpu_requests_ = 1;
    return SetState(kPrepared);
  }

  int batch_size = -1;

  for (const auto& name : main_executable_ref_.InputLayerNames()) {
    if (inputs_.find(name) == inputs_.end()) {
      return InvalidArgumentError(
          StringPrintf("Unable to find input for layer %s.", name.c_str()));
    }

    if (batch_size == -1) {
      batch_size = inputs_[name].size();
      continue;
    }

    if (inputs_[name].size() != batch_size) {
      return InvalidArgumentError(
          StringPrintf("Mismatched number of input buffers for \"%s\". "
                       "expected=%d, actual=%zu.",
                       name.c_str(), batch_size, inputs_[name].size()));
    }
  }

  for (const auto& name : main_executable_ref_.OutputLayerNames()) {
    if (outputs_.find(name) == outputs_.end()) {
      return InvalidArgumentError(
          StringPrintf("Unable to find output for layer %s.", name.c_str()));
    }

    if (batch_size == -1) {
      batch_size = outputs_[name].size();
      continue;
    }

    if (outputs_[name].size() != batch_size) {
      return InvalidArgumentError(
          StringPrintf("Mismatched number of output buffers for \"%s\". "
                       "expected=%d, actual=%zu.",
                       name.c_str(), batch_size, outputs_[name].size()));
    }
  }

  if (batch_size <= 0) {
    return InvalidArgumentError("No input/output buffers found.");
  }

  request_batch_size_ = batch_size;
  required_tpu_request_count_ =
      MathUtil::CeilOfRatio(request_batch_size_, hardware_batch_size_);
  pending_tpu_requests_ = required_tpu_request_count_;

  VLOG(2) << StringPrintf(
      "Request prepared, total batch size: %d, total TPU requests required: "
      "%d.",
      request_batch_size_, required_tpu_request_count_);
  return SetState(kPrepared);
}

StatusOr<int> Request::RemainingTpuRequestCount() const {
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateState(kPrepared));
  return required_tpu_request_count_ - tpu_requests_prepared_;
}

Status Request::PrepareTpuRequest(std::shared_ptr<TpuRequest> tpu_request) {
  TRACE_SCOPE("Request::PrepareTpuRequest");
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateState(kPrepared));

  if (main_executable_ref_.NumInputLayers() == 0 &&
      main_executable_ref_.NumOutputLayers() == 0) {
    return PrepareNoIORequest(tpu_request);
  } else {
    return PrepareIORequest(tpu_request);
  }
}

Status Request::PrepareNoIORequest(std::shared_ptr<TpuRequest> tpu_request) {
  TRACE_SCOPE("Request::PrepareNoIORequest");
  if (request_batch_size_ != 1) {
    return InvalidArgumentError(
        StringPrintf("Executable batch size is 1, yet %d sets of input/outputs "
                     "are provided.",
                     request_batch_size_));
  }

  if (tpu_requests_prepared_ >= 1) {
    return FailedPreconditionError(
        StringPrintf("%d are already prepared yet prepare was called again.",
                     tpu_requests_prepared_));
  }

  auto done = [this](int id, const Status& status) {
    TpuRequestDone(id, status);
  };
  RETURN_IF_ERROR(tpu_request->SetDone(std::move(done)));

  tpu_requests_prepared_ = 1;
  return OkStatus();
}

Status Request::PrepareIORequest(std::shared_ptr<TpuRequest> tpu_request) {
  TRACE_SCOPE("Request::PrepareIORequest");
  if (tpu_requests_prepared_ >= required_tpu_request_count_) {
    return InternalError(
        StringPrintf("Software batch (expected size=%d, actual size=%d) "
                     "already saturated with prepared TPU requests",
                     required_tpu_request_count_, tpu_requests_prepared_));
  }

  for (int j = 0; j < hardware_batch_size_; ++j) {
    const int buffer_index = tpu_requests_prepared_ * hardware_batch_size_ + j;
    if (buffer_index >= request_batch_size_) {
      CHECK_EQ(tpu_requests_prepared_ + 1, required_tpu_request_count_);
      break;
    }

    for (const auto& name : main_executable_ref_.InputLayerNames()) {
      RETURN_IF_ERROR(
          tpu_request->AddInput(name, inputs_.at(name)[buffer_index]));
    }

    for (const auto& name : main_executable_ref_.OutputLayerNames()) {
      RETURN_IF_ERROR(
          tpu_request->AddOutput(name, outputs_.at(name)[buffer_index]));
    }
  }

  auto done = [this](int id, const Status& status) {
    TpuRequestDone(id, status);
  };
  RETURN_IF_ERROR(tpu_request->SetDone(std::move(done)));

  // In order not to confuse the TPU, if the last TpuRequest does not have
  // enough input/outputs to support the entire native batch size, add dummy
  // ones to break even.
  if (tpu_requests_prepared_ + 1 == required_tpu_request_count_) {
    const int num_noop_buffers =
        (required_tpu_request_count_ * hardware_batch_size_) -
        request_batch_size_;
    if (num_noop_buffers > 0) {
      for (const auto& name : main_executable_ref_.InputLayerNames()) {
        RETURN_IF_ERROR(tpu_request->AddNoopInputs(name, num_noop_buffers));
      }
      for (const auto& name : main_executable_ref_.OutputLayerNames()) {
        RETURN_IF_ERROR(tpu_request->AddNoopOutputs(name, num_noop_buffers));
      }
    }
  }

  ++tpu_requests_prepared_;
  return OkStatus();
}

void Request::NotifySubmission(TpuRequest::RequestType type) {
  StdMutexLock lock(&mutex_);
  auto time_now = current_time_.GetTimeNanoSeconds();
  if (timing_.submitted_ns == -1) {
    timing_.submitted_ns = time_now;  // Update parent submission time.
  }
  timing_.detail_timing.push_back(TimingEvent(
      time_now, type, api::Request::TimingEvent::EventType::SUBMITTED));
}

void Request::NotifyCompletion(TpuRequest::RequestType type) {
  StdMutexLock lock(&mutex_);
  // Update parent completion time.
  timing_.completed_ns = current_time_.GetTimeNanoSeconds();
  timing_.detail_timing.push_back(
      TimingEvent(timing_.completed_ns, type,
                  api::Request::TimingEvent::EventType::COMPLETED));
}

void Request::TpuRequestDone(int id, const Status& status) {
  // TODO Improve handling of this error.
  CHECK_OK(HandleTpuRequestsDone(status, 1));
}

Status Request::HandleTpuRequestsDone(const Status& status,
                                      int num_requests_done) {
  Done done;
  int64 request_id;
  Status done_status;

  {
    StdMutexLock lock(&mutex_);
    // TODO Improve handling of this error.
    RETURN_IF_ERROR(ValidateState(kPrepared));

    if (num_requests_done > pending_tpu_requests_) {
      return InternalError(
          StringPrintf("Number of done requests (%d) exceeds number of pending "
                       "requests (%d).",
                       num_requests_done, pending_tpu_requests_));
    }

    pending_tpu_requests_ -= num_requests_done;
    done_status_.Update(status);
    if (pending_tpu_requests_ > 0) {
      return OkStatus();
    }

    RETURN_IF_ERROR(SetState(kDone));

    done = std::move(done_);
    done_ = nullptr;
    request_id = id_;
    done_status = done_status_;
  }

  done(request_id, done_status);
  return OkStatus();
}

Status Request::SetState(State next_state) {
  switch (state_) {
    case kInitial:
      if (next_state == kPrepared) {
        state_ = next_state;
        return OkStatus();
      }
      break;

    case kPrepared:
      if (next_state == kDone) {
        state_ = next_state;
        return OkStatus();
      }
      break;

    case kDone:
      return FailedPreconditionError(
          StringPrintf("Cannot set state from done to %d.", next_state));
  }

  // Illegal state transition.
  return FailedPreconditionError(StringPrintf(
      "Invalid state transition. current=%d, next=%d.", state_, next_state));
}

Status Request::ValidateState(State state) const {
  if (state_ != state) {
    return FailedPreconditionError(
        StringPrintf("Invalid state. Expected=%d, Actual=%d.", state, state_));
  }
  return OkStatus();
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
