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

#ifndef DARWINN_DRIVER_REGISTERS_REGISTERS_H_
#define DARWINN_DRIVER_REGISTERS_REGISTERS_H_

#include "driver_shared/registers.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/statusor.h"
#include "port/time.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Interface for CSR access.
class Registers : public driver_shared::Registers {
 public:
  // To indicate the polling functions should poll forever.
  static constexpr int64 kInfiniteTimeout = -1;

  virtual ~Registers() = default;

  // Polls the specified register until it has the given value.
  util::Status Poll(uint64 offset, uint64 expected_value) {
    return Poll(offset, expected_value, kInfiniteTimeout);
  }

  // Polls the sepcified register until it has the given value or until it
  // takes longer than the provided timeout in microseconds.
  // Polls forever if timeout is zero or negative. Same behavior as no timeout
  // version.
  virtual util::Status Poll(uint64 offset, uint64 expected_value,
                            int64 timeout_us);

  // 32-bit version of above. Usually, it is same as running 64 bit version.
  util::Status Poll32(uint64 offset, uint32 expected_value) {
    return Poll32(offset, expected_value, kInfiniteTimeout);
  }
  virtual util::Status Poll32(uint64 offset, uint32 expected_value,
                              int64 timeout_us);

 protected:
  // Helper function for spin reads a register until received expected value or
  // reached timeout.  Polls forever if timeout is zero or negative.
  // Subclass to use this by passing the specific register read function.
  template <typename IntType, typename FuncType>
  util::Status SpinReadHelper(uint64 offset, IntType expected_value,
                              int64 timeout_us, const FuncType& read_func) {
    int64 start_time_us, end_time_us;
    if (timeout_us > 0) {
      start_time_us = GetCurrentTimeMicros();
    }

    ASSIGN_OR_RETURN(auto actual_value, read_func(offset));
    while (actual_value != expected_value) {
      if (timeout_us > 0) {
        end_time_us = GetCurrentTimeMicros();
        if (end_time_us - start_time_us > timeout_us) {
          return util::DeadlineExceededError("Register poll timeout.");
        }
      }
      ASSIGN_OR_RETURN(actual_value, read_func(offset));
    }
    return util::OkStatus();
  }
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_REGISTERS_REGISTERS_H_
