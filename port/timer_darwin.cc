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

#include "port/timer_darwin.h"

namespace platforms {
namespace darwinn {
namespace api {

util::Status Timer::Set(int64 nanos) {
  std::lock_guard<std::mutex> guard(mutex_);

  deadline_ = nanos == 0 ? Clock::time_point{Clock::duration::max()}
                         : Clock::now() + std::chrono::nanoseconds(nanos);
  deadline_set_.notify_all();
  return util::OkStatus();
}

util::StatusOr<uint64> Timer::Wait() {
  while (true) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto now = Clock::now();
    if (now >= deadline_ || deadline_set_.wait_for(lock, deadline_ - now) ==
                                std::cv_status::timeout)
      return 1;
  }
}

}  // namespace api
}  // namespace darwinn
}  // namespace platforms
