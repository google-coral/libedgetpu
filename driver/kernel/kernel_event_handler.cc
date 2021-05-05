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

#include "driver/kernel/kernel_event_handler.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <thread>  // NOLINT
#include <utility>

#include "driver/kernel/kernel_event.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"
#include "port/tracing.h"

namespace platforms {
namespace darwinn {
namespace driver {

KernelEventHandler::KernelEventHandler(const std::string& device_path,
                                       int num_events)
    : device_path_(device_path), num_events_(num_events) {
  event_fds_.resize(num_events_, INVALID_FD_VALUE);
  events_.resize(num_events_);
}

Status KernelEventHandler::Open() {
  StdMutexLock lock(&mutex_);
  if (fd_ != INVALID_FD_VALUE) {
    return FailedPreconditionError("Device already open.");
  }

  fd_ = open(device_path_.c_str(), O_RDWR);
  if (fd_ < 0) {
    return FailedPreconditionError(
        StringPrintf("Device open failed : %d (%s)", fd_, strerror(errno)));
  }

  for (int i = 0; i < num_events_; ++i) {
    event_fds_[i] = InitializeEventFd(i);
    events_[i].reset();
  }

  return Status();  // OK
}

Status KernelEventHandler::Close() {
  StdMutexLock lock(&mutex_);
  if (fd_ == INVALID_FD_VALUE) {
    return FailedPreconditionError("Device not open.");
  }

  Status status;
  for (int i = 0; i < num_events_; ++i) {
    events_[i].reset();
    status.Update(ReleaseEventFd(fd_, event_fds_[i], i));
  }

  close(fd_);
  fd_ = INVALID_FD_VALUE;

  return status;
}

Status KernelEventHandler::RegisterEvent(int event_id,
                                         KernelEvent::Handler handler) {
  StdMutexLock lock(&mutex_);
  if (fd_ == INVALID_FD_VALUE) {
    return FailedPreconditionError("Device not open.");
  }

  RETURN_IF_ERROR(SetEventFd(fd_, event_fds_[event_id], event_id));

  // Enable events.
  events_[event_id] =
      CreateKernelEvent(event_fds_[event_id], std::move(handler));

  return Status();  // OK;
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
