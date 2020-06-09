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

#ifndef DARWINN_DRIVER_TIME_STAMPER_TIME_STAMPER_H_
#define DARWINN_DRIVER_TIME_STAMPER_TIME_STAMPER_H_

#include "port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Abstract class for timestamping. Make it a class so a stateful mock can be
// used in tests.
// TODO move timestamper to a common location to share between
// driver and driver2.
class TimeStamper {
 public:
  // Multiplier factors to help convert between various timer resolutions.
  static constexpr int64 kNanoSecondsPerMicroSecond = 1000;
  static constexpr int64 kNanoSecondsPerMilliSecond =
      1000 * kNanoSecondsPerMicroSecond;
  static constexpr int64 kNanoSecondsPerSecond =
      1000 * kNanoSecondsPerMilliSecond;
  static constexpr int64 kMicroSecondsPerSecond =
      kNanoSecondsPerSecond / kNanoSecondsPerMicroSecond;
  static constexpr int64 kMilliSecondsPerSecond =
      kNanoSecondsPerSecond / kNanoSecondsPerMilliSecond;
  static constexpr int64 kInvalidTimestamp = -1;

  TimeStamper() = default;

  // This class is neither copyable nor movable.
  TimeStamper(const TimeStamper &) = delete;
  TimeStamper &operator=(const TimeStamper &) = delete;

  virtual ~TimeStamper() = default;

  // Returns a monotonically-increasing timestamp. Base resolution is nano
  // second. Default implementations are provided for other resolutions.
  // However, if it is not possible to provide nano second resolution,
  // implementations can choose to override lower resolution methods explictly.
  virtual int64 GetTimeNanoSeconds() const = 0;

  virtual int64 GetTimeMicroSeconds() const {
    return GetTimeNanoSeconds() / kNanoSecondsPerMicroSecond;
  }

  virtual int64 GetTimeMilliSeconds() const {
    return GetTimeNanoSeconds() / kNanoSecondsPerMilliSecond;
  }

  virtual int64 GetTimeSeconds() const {
    return GetTimeNanoSeconds() / kNanoSecondsPerSecond;
  }
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_TIME_STAMPER_TIME_STAMPER_H_
