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

#ifndef DARWINN_DRIVER_SHARED_TIME_STAMPER_DRIVER_TIME_STAMPER_H_
#define DARWINN_DRIVER_SHARED_TIME_STAMPER_DRIVER_TIME_STAMPER_H_

#include "driver_shared/time_stamper/time_stamper.h"

namespace platforms {
namespace darwinn {

namespace driver_shared {

// Microsec-resolution monotonic clock.
class DriverTimeStamper : public TimeStamper {
 public:
  DriverTimeStamper() = default;
  ~DriverTimeStamper() = default;

  int64 GetTimeNanoSeconds() const override;
};

}  // namespace driver_shared
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_SHARED_TIME_STAMPER_DRIVER_TIME_STAMPER_H_
