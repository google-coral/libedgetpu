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

#include "driver/kernel/windows/kernel_event_windows.h"

#include <errno.h>
#include <fcntl.h>

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

KernelEventWindows::KernelEventWindows(FileDescriptor event_fd, Handler handler)
    : KernelEvent(event_fd, handler),
      event_fd_(event_fd) {
  std::thread event_thread(&KernelEventWindows::Monitor, this,
                           std::move(handler));
  thread_ = std::move(event_thread);
}

KernelEventWindows::~KernelEventWindows() {
  // Mark as disabled.
  {
    StdMutexLock lock(&mutex_);
    enabled_ = false;
  }

  // Signal a fake event to force WaitForSingleObject() to return.
  BOOL result = SetEvent(event_fd_);
  if (!result) {
    DWORD gle = GetLastError();
    LOG(WARNING) << StringPrintf("SetEvent failed! event_fd=%p gle=%d",
                                 event_fd_, gle);
  }

  // Wait for thread to exit.
  thread_.join();
}

void KernelEventWindows::Monitor(const Handler& handler) {
  VLOG(5) << StringPrintf("event_fd=%p. Monitor thread begin.", event_fd_);
  TRACE_START_THREAD("KernelEventHandlerMonitor");

  while (IsEnabled()) {
    // Wait for events (blocking).
    DWORD result = WaitForSingleObject(event_fd_, INFINITE);
    if (result != WAIT_OBJECT_0) {
      VLOG(5) << StringPrintf(
          "WaitForSingleObject failed "
          "evend_fd=%p result=%d gle=%d",
          event_fd_, result, GetLastError());
      break;
    }

    BOOL reset = ResetEvent(event_fd_);
    if (!reset) {
      VLOG(5) << StringPrintf("ResetEvent failed evend_fd=%p gle=%d", event_fd_,
                              GetLastError());
      break;
    }

    VLOG(5) << StringPrintf("event_fd=%p. Monitor thread got event.",
                            event_fd_);
    if (IsEnabled()) {
      handler();
    }
  }

  VLOG(5) << StringPrintf("event_fd=%p. Monitor thread exit.", event_fd_);
}

bool KernelEventWindows::IsEnabled() const {
  StdMutexLock lock(&mutex_);
  return enabled_;
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
