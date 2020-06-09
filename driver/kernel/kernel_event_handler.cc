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
#include <sys/eventfd.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string>
#include <thread>  // NOLINT
#include <utility>

#include "driver/kernel/kernel_event.h"
#include "driver/kernel/linux_gasket_ioctl.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/ptr_util.h"
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
  event_fds_.resize(num_events_, -1);
  events_.resize(num_events_);
}

util::Status KernelEventHandler::Open() {
  StdMutexLock lock(&mutex_);
  if (fd_ != -1) {
    return util::FailedPreconditionError("Device already open.");
  }

  fd_ = open(device_path_.c_str(), O_RDWR);
  if (fd_ < 0) {
    return util::FailedPreconditionError(
        StringPrintf("Device open failed : %d (%s)", fd_, strerror(errno)));
  }

  for (int i = 0; i < num_events_; ++i) {
    event_fds_[i] = eventfd(0, EFD_CLOEXEC);
    events_[i].reset();
  }

  return util::Status();  // OK
}

util::Status KernelEventHandler::Close() {
  StdMutexLock lock(&mutex_);
  if (fd_ == -1) {
    return util::FailedPreconditionError("Device not open.");
  }

  for (int i = 0; i < num_events_; ++i) {
    events_[i].reset();
    close(event_fds_[i]);
  }

  close(fd_);
  fd_ = -1;

  return util::Status();  // OK
}

util::Status KernelEventHandler::SetEventFd(int event_fd, int event_id) const {
  gasket_interrupt_eventfd interrupt;
  interrupt.interrupt = event_id;
  interrupt.event_fd = event_fd;
  if (ioctl(fd_, GASKET_IOCTL_SET_EVENTFD, &interrupt) != 0) {
    return util::FailedPreconditionError(StringPrintf(
        "Setting Event Fd Failed : %d (%s)", fd_, strerror(errno)));
  }

  VLOG(5) << StringPrintf("Set event fd : event_id:%d -> event_fd:%d, ",
                          event_id, event_fd);

  return util::Status();  // OK
}

util::Status KernelEventHandler::RegisterEvent(int event_id,
                                               KernelEvent::Handler handler) {
  StdMutexLock lock(&mutex_);
  if (fd_ == -1) {
    return util::FailedPreconditionError("Device not open.");
  }

  RETURN_IF_ERROR(SetEventFd(event_fds_[event_id], event_id));

  // Enable events.
  events_[event_id] =
      gtl::MakeUnique<KernelEvent>(event_fds_[event_id], std::move(handler));

  return util::Status();  // OK;
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
