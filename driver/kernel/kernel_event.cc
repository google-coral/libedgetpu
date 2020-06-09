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

#include "driver/kernel/kernel_event.h"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/eventfd.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <thread>  // NOLINT

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

KernelEvent::KernelEvent(int event_fd, Handler handler) : event_fd_(event_fd) {
  std::thread event_thread(&KernelEvent::Monitor, this, event_fd,
                           std::move(handler));
  thread_ = std::move(event_thread);
}

KernelEvent::~KernelEvent() {
  // Mark as disabled.
  {
    StdMutexLock lock(&mutex_);
    enabled_ = false;
  }

  // Write a fake event to force read() to return.
  uint64 num_events = 1;
  int result = write(event_fd_, &num_events, sizeof(num_events));
  if (result != sizeof(num_events)) {
    LOG(WARNING) << StringPrintf("event_fd=%d. Fake event write failed (%d).",
                                 event_fd_, result);
  }

  // Wait for thread to exit.
  thread_.join();
}

bool KernelEvent::IsEnabled() const {
  StdMutexLock lock(&mutex_);
  return enabled_;
}

void KernelEvent::Monitor(int event_fd, const Handler& handler) {
  VLOG(5) << StringPrintf("event_fd=%d. Monitor thread begin.", event_fd);
  TRACE_START_THREAD("KernelEventHandlerMonitor");

  while (IsEnabled()) {
    // Wait for events (blocking).
    uint64_t num_events = 0;
    int result = read(event_fd, &num_events, sizeof(num_events));
    if (result != sizeof(num_events)) {
      LOG(WARNING) << StringPrintf("event_fd=%d. Read failed (%d).", event_fd,
                                   result);
      break;
    }

    VLOG(5) << StringPrintf(
        "event_fd=%d. Monitor thread got num_events=%" PRId64 ".", event_fd,
        num_events);
    if (IsEnabled()) {
      for (int i = 0; i < num_events; ++i) {
        handler();
      }
    }
  }

  VLOG(5) << StringPrintf("event_fd=%d. Monitor thread exit.", event_fd);
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
