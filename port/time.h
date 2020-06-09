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

#ifndef DARWINN_PORT_TIME_H_
#define DARWINN_PORT_TIME_H_

#include "port/defs.h"
#include "port/integral_types.h"

#if DARWINN_PORT_USE_GOOGLE3

#include "absl/time/clock.h"
#include "absl/time/time.h"

// Returns the current timestamp in nanoseconds.
inline int64 GetCurrentTimeNanos() {
  return absl::GetCurrentTimeNanos();
}

// Returns the current timestamp in microseconds.
inline int64 GetCurrentTimeMicros() {
  return reinterpret_cast<int64>(absl::GetCurrentTimeNanos() / 1000);
}

// Sleep for the specified amount of seconds.
inline void Sleep(int seconds) {
  absl::SleepFor(absl::Seconds(seconds));
}

// Sleep for the specified amount of microseconds.
inline void Microsleep(int microseconds) {
  absl::SleepFor(absl::Microseconds(microseconds));
}

#else  // !DARWINN_PORT_USE_GOOGLE3


#include <chrono>  // NOLINT
#include <thread>  // NOLINT

// Returns the current timestamp in nanoseconds.
inline platforms::darwinn::int64 GetCurrentTimeNanos() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::system_clock::now().time_since_epoch())
      .count();
}

// Returns the current timestamp in microseconds.
inline platforms::darwinn::int64 GetCurrentTimeMicros() {
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

// Sleep for the specified amount of seconds.
inline void Sleep(int seconds) {
  std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

// Sleep for the specified amount of microseconds.
inline void Microsleep(int microseconds) {
  std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

#endif  // DARWINN_PORT_USE_GOOGLE3


#endif  // DARWINN_PORT_TIME_H_
