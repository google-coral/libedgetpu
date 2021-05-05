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

#ifndef DARWINN_DRIVER_REGISTERS_SOCKET_REGISTERS_H_
#define DARWINN_DRIVER_REGISTERS_SOCKET_REGISTERS_H_

#include <errno.h>
#include <sys/socket.h>
#include <mutex>  // NOLINT
#include <string>

#include "driver/registers/registers.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/status.h"
#include "port/statusor.h"
#include "port/stringprintf.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Socket implementation of the register interface that sends requests through
// socket and receives the results back through socket.
//
// Commands are sent as following:
// 1. 'r' or 'w' depending on read/write.
// 2. Offset for both read/write.
// 3. If write, value to write.
class SocketRegisters : public Registers {
 public:
  SocketRegisters(const std::string& ip_address, int port);
  ~SocketRegisters() override;

  // This class is neither copyable nor movable.
  SocketRegisters(const SocketRegisters&) = delete;
  SocketRegisters& operator=(const SocketRegisters&) = delete;

  // Overrides from registers.h
  Status Open() LOCKS_EXCLUDED(mutex_) override;
  Status Close() LOCKS_EXCLUDED(mutex_) override;
  Status Write(uint64 offset, uint64 value) LOCKS_EXCLUDED(mutex_) override;
  StatusOr<uint64> Read(uint64 offset) LOCKS_EXCLUDED(mutex_) override;
  Status Write32(uint64 offset, uint32 value) LOCKS_EXCLUDED(mutex_) override {
    return Write(offset, value);
  }
  StatusOr<uint32> Read32(uint64 offset) LOCKS_EXCLUDED(mutex_) override {
    return Read(offset);
  }

 private:
  template <typename T>
  Status Send(const T& message) EXCLUSIVE_LOCKS_REQUIRED(mutex_) {
    if (send(socket_fd_, &message, sizeof(message), /*flags=*/0) < 0) {
      return UnavailableError(StringPrintf("send failed (%d).", errno));
    }
    return Status();  // OK
  }

  // IP address.
  const std::string ip_address_;

  // Port number.
  const int port_;

  // Mutex that guards socket_fd_;
  std::mutex mutex_;

  // Socket descriptor.
  int socket_fd_ GUARDED_BY(mutex_){-1};
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_REGISTERS_SOCKET_REGISTERS_H_
