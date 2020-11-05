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

#include "driver/kernel/linux/kernel_event_linux.h"

#include <errno.h>
#include <inttypes.h>
#include <fcntl.h>
#include <unistd.h>

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

KernelEventLinux::KernelEventLinux(FileDescriptor event_fd, Handler handler)
    : KernelEvent(event_fd, handler),
      event_fd_(event_fd) {
  std::thread event_thread(&KernelEventLinux::Monitor, this,
                           std::move(handler));
  thread_ = std::move(event_thread);
}

KernelEventLinux::~KernelEventLinux() {
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

void KernelEventLinux::Monitor(const Handler& handler) {
  VLOG(5) << StringPrintf("event_fd=%d. Monitor thread begin.", event_fd_);
  TRACE_START_THREAD("KernelEventHandlerMonitor");

  while (IsEnabled()) {
    // Wait for events (blocking).
    uint64_t num_events = 0;
    int result = read(event_fd_, &num_events, sizeof(num_events));
    if (result != sizeof(num_events)) {
      LOG(WARNING) << StringPrintf("event_fd=%d. Read failed (%d).", event_fd_,
                                   result);
      break;
    }

    VLOG(5) << StringPrintf(
        "event_fd=%d. Monitor thread got num_events=%" PRId64 ".", event_fd_,
        num_events);
    if (IsEnabled()) {
      for (int i = 0; i < num_events; ++i) {
        handler();
      }
    }
  }

  VLOG(5) << StringPrintf("event_fd=%d. Monitor thread exit.", event_fd_);
}

bool KernelEventLinux::IsEnabled() const {
  StdMutexLock lock(&mutex_);
  return enabled_;
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
