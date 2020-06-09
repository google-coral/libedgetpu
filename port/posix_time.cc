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

#include "port/posix_time.h"

#include <sys/time.h>
#include <time.h>

namespace platforms {
namespace darwinn {

namespace {

// Seconds to nanoseconds.
constexpr uint64 kSecondsToNanos = 1000ULL * 1000ULL * 1000ULL;

}  // namespace

uint64 GetRealTimeNanos() {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (static_cast<uint64>(ts.tv_sec) * kSecondsToNanos +
          static_cast<uint64>(ts.tv_nsec));
}

uint64 GetBootTimeNanos() {
  struct timespec ts;
  clock_gettime(CLOCK_BOOTTIME, &ts);
  return (static_cast<uint64>(ts.tv_sec) * kSecondsToNanos +
          static_cast<uint64>(ts.tv_nsec));
}

}  // namespace darwinn
}  // namespace platforms
