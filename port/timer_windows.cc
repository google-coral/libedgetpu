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

#include "port/timer.h"

#include <windows.h>
#include <synchapi.h>

#include "port/errors.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace api {

Timer::Timer() {
  timer_handle_ = CreateWaitableTimer(NULL, FALSE, NULL);
  CHECK(timer_handle_) << StringPrintf("CreateWaitableTimer failed: %d",
                                       GetLastError());
}

Timer::~Timer() { CloseHandle(timer_handle_); }

util::Status Timer::Set(int64 nanos) {
  // Negative times represent relative times.
  // SetWaitableTimer units are 100ns.
  LARGE_INTEGER dueTime;
  dueTime.QuadPart = -(nanos / 100);
  bool ret = SetWaitableTimer(timer_handle_, &dueTime, 0, NULL, NULL, NULL);
  if (!ret) {
    return util::InternalError(
        StringPrintf("Failed to set timer: %d", GetLastError()));
  }
  return util::OkStatus();
}

util::StatusOr<uint64> Timer::Wait() {
  if (WaitForSingleObject(timer_handle_, INFINITE) == WAIT_OBJECT_0) {
    return 1;
  }
  return 0;
}

}  // namespace api
}  // namespace darwinn
}  // namespace platforms
