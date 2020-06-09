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

#ifndef DARWINN_API_TIMING_H_
#define DARWINN_API_TIMING_H_
#include "port/stringprintf.h"
namespace platforms {
namespace darwinn {
namespace api {

// Timing information for real-time/QoS scheduler when applicable.
struct Timing {
  // Inference arrival rate, in FPS.
  int fps{0};
  // Max execution time (MET), in milliseconds.
  int max_execution_time_ms{0};
  // Tolerance, or how much an inference can be delayed. Also in milliseconds.
  // 0 <= Tolerance <= (1/FPS - MET).
  int tolerance_ms{0};

  std::string Dump() const {
    return StringPrintf("(%d FPS; max execution time %d ms; tolerance %d ms)",
                        fps, max_execution_time_ms, tolerance_ms);
  }
};

// Trivial equality operator for Timing.
inline bool operator==(const Timing& lhs, const Timing& rhs) {
  return lhs.fps == rhs.fps &&
         lhs.max_execution_time_ms == rhs.max_execution_time_ms &&
         lhs.tolerance_ms == rhs.tolerance_ms;
}

inline bool operator!=(const Timing& lhs, const Timing& rhs) {
  return !(lhs == rhs);
}

}  // namespace api
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_API_TIMING_H_
