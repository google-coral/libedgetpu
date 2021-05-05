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

#include "driver/registers/socket_registers.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>

#include "port/cleanup.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/logging.h"
#include "port/ptr_util.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/statusor.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

SocketRegisters::SocketRegisters(const std::string& ip_address, int port)
    : ip_address_(ip_address), port_(port) {}

SocketRegisters::~SocketRegisters() {
  if (socket_fd_ != -1) {
    LOG(WARNING)
        << "Destroying SocketRegisters - Close() has not yet been called!";
    Status status = Close();
    if (!status.ok()) {
      LOG(ERROR) << status;
    }
  }
}

Status SocketRegisters::Open() {
  StdMutexLock lock(&mutex_);
  if (socket_fd_ != -1) {
    return FailedPreconditionError("Socket already open.");
  }

  VLOG(1) << StringPrintf("Opening socket at %s:%d", ip_address_.c_str(),
                          port_);

  if ((socket_fd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    return UnavailableError(StringPrintf("socket failed (%d).", errno));
  }

  // Clean up on error.
  auto socket_closer = MakeCleanup(
      [this]() EXCLUSIVE_LOCKS_REQUIRED(mutex_) { close(socket_fd_); });

  // Setup server address.
  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port_);

  if (inet_pton(AF_INET, ip_address_.c_str(), &server_addr.sin_addr) <= 0) {
    return FailedPreconditionError(
        StringPrintf("Invalid ip address: %s", ip_address_.c_str()));
  }

  // Make connection.
  if (connect(socket_fd_, reinterpret_cast<sockaddr*>(&server_addr),
              sizeof(server_addr)) < 0) {
    return UnavailableError(StringPrintf("connect failed (%d).", errno));
  }

  socket_closer.release();
  return Status();  // OK
}

Status SocketRegisters::Close() {
  StdMutexLock lock(&mutex_);
  if (socket_fd_ == -1) {
    return FailedPreconditionError("Socket already closed.");
  }

  close(socket_fd_);
  return Status();  // OK
}

Status SocketRegisters::Write(uint64 offset, uint64 value) {
  VLOG(2) << StringPrintf(
      "Register write 0x%llx to 0x%llx",
      static_cast<unsigned long long>(value),    // NOLINT(runtime/int)
      static_cast<unsigned long long>(offset));  // NOLINT(runtime/int)
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(Send('w'));
  RETURN_IF_ERROR(Send(offset));
  RETURN_IF_ERROR(Send(value));
  return Status();  // OK
}

StatusOr<uint64> SocketRegisters::Read(uint64 offset) {
  VLOG(2) << StringPrintf(
      "Register read from 0x%llx",
      static_cast<unsigned long long>(offset));  // NOLINT(runtime/int)
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(Send('r'));
  RETURN_IF_ERROR(Send(offset));
  uint64 value;
  if (recv(socket_fd_, &value, sizeof(value), MSG_WAITALL) < 0) {
    return UnavailableError(StringPrintf("recv failed (%d).", errno));
  }
  return value;
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
