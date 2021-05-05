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

#ifndef DARWINN_PORT_TIMER_LINUX_H_
#define DARWINN_PORT_TIMER_LINUX_H_

#include "port/integral_types.h"
#include "port/status.h"
#include "port/statusor.h"

#if defined(__linux__)
#include <sys/timerfd.h>
#include <unistd.h>
#else
#include <chrono>              // NOLINT(build/c++11)
#include <condition_variable>  // NOLINT(build/c++11)
#include <mutex>               // NOLINT(build/c++11)
#endif

namespace platforms {
namespace darwinn {
namespace api {

// A simple interface for countdown timers.
class Timer {
 public:
  Timer();
  virtual ~Timer();

  // This class is neither copyable nor movable.
  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;

  // Sets the timer to the specified nanoseconds. Countdown immediately starts
  // after setting. Setting to 0 will de-activate the timer.
  virtual Status Set(int64 nanos);

  // Waits for the timer to reach 0 and returns. If timer is de-activated before
  // reaching 0 or never activated, this call will never return.
  virtual StatusOr<uint64> Wait();

 private:
#if defined(__linux__)
  // File handle for timerfd.
  int fd_;
#else
  using Clock = std::chrono::steady_clock;
  // Deadline until Wait() method is blocked.
  Clock::time_point deadline_{Clock::duration::max()};
  // Mutex which guards condition variable.
  std::mutex mutex_;
  // Condition variable to wait on (until deadline is reached).
  std::condition_variable deadline_set_;
#endif
};

}  // namespace api
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_PORT_TIMER_LINUX_H_
