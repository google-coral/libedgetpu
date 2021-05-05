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

#include "driver/registers/registers.h"

#include "port/integral_types.h"
#include "port/status.h"
#include "port/status_macros.h"

namespace platforms {
namespace darwinn {
namespace driver {

Status Registers::Poll(uint64 offset, uint64 expected_value, int64 timeout_us) {
  return SpinReadHelper(offset, expected_value, timeout_us,
                        [this](uint64 offset) { return Read(offset); });
}

Status Registers::Poll32(uint64 offset, uint32 expected_value,
                         int64 timeout_us) {
  return SpinReadHelper(offset, expected_value, timeout_us,
                        [this](uint64 offset) { return Read32(offset); });
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
