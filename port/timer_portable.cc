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

#include "port/errors.h"
#include "port/stringprintf.h"
#include "port/timer.h"

namespace platforms {
namespace darwinn {
namespace api {

#if defined(__linux__)

Timer::Timer() {
  fd_ = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
  CHECK_GE(fd_, 0) << StringPrintf("Failed to create timerfd: %s",
                                   strerror(errno));
}

Timer::~Timer() { close(fd_); }

Status Timer::Set(int64 nanos) {
  itimerspec spec = {
      .it_interval =
          {
              .tv_sec = 0,
              .tv_nsec = 0,
          },
      .it_value =
          {
              .tv_sec = static_cast<time_t>(nanos / 1000000000),
              .tv_nsec = static_cast<int32>(nanos % 1000000000),
          },
  };

  int return_code = timerfd_settime(fd_, 0, &spec, nullptr);
  if (return_code != 0) {
    return InternalError(
        StringPrintf("Failed to set timer: %s", strerror(errno)));
  }

  return OkStatus();
}

StatusOr<uint64> Timer::Wait() {
  uint64 expirations;
  size_t bytes_read = read(fd_, &expirations, sizeof(uint64));
  if (errno == EINTR) {
    return 0;
  }
  if (bytes_read != sizeof(uint64)) {
    return InternalError(StringPrintf("Timer read failed (%zu bytes read): %s",
                                      bytes_read, strerror(errno)));
  }

  return expirations;
}

#else

Timer::Timer() = default;

Timer::~Timer() = default;

Status Timer::Set(int64 nanos) {
  std::lock_guard<std::mutex> guard(mutex_);

  deadline_ = nanos == 0 ? Clock::time_point{Clock::duration::max()}
                         : Clock::now() + std::chrono::nanoseconds(nanos);
  deadline_set_.notify_all();
  return OkStatus();
}

StatusOr<uint64> Timer::Wait() {
  while (true) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto now = Clock::now();
    if (now >= deadline_ || deadline_set_.wait_for(lock, deadline_ - now) ==
                                std::cv_status::timeout)
      return 1;
  }
}

#endif

}  // namespace api
}  // namespace darwinn
}  // namespace platforms
