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

#ifndef DARWINN_DRIVER_SHARED_REGISTERS_H_
#define DARWINN_DRIVER_SHARED_REGISTERS_H_

#include "port/integral_types.h"
#include "port/status.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {

namespace driver_shared {

// An interface to device registers.
// TODO Move this to a shared location with 1.0.
class Registers {
 public:
  Registers() = default;
  virtual ~Registers() = default;

  // This class is neither copyable nor movable.
  Registers(const Registers&) = delete;
  Registers& operator=(const Registers&) = delete;

  // Open / Close the register interface. Any register access can only happen
  // when the interface is open.
  virtual util::Status Open() = 0;
  virtual util::Status Close() = 0;

  // Write / Read from a register at the given 64 bit aligned offset.
  // Offset may be implementation dependent.
  virtual util::Status Write(uint64 offset, uint64 value) = 0;
  virtual util::StatusOr<uint64> Read(uint64 offset) = 0;

  // Polls the sepcified register until it has the given value or until it
  // takes longer than the provided timeout in nanosecondss.
  // Polls forever if timeout is zero or negative.
  virtual util::Status Poll(uint64 offset, uint64 expected_value,
                            int64 timeout_ns = 0) = 0;

  // 32-bit version of above. Usually, it is same as running 64 bit version.
  virtual util::Status Write32(uint64 offset, uint32 value) = 0;
  virtual util::StatusOr<uint32> Read32(uint64 offset) = 0;
  virtual util::Status Poll32(uint64 offset, uint32 expected_value,
                              int64 timeout_ns = 0) = 0;
};

}  // namespace driver_shared
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_SHARED_REGISTERS_H_
