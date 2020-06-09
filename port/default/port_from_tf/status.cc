/* Copyright 2015 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "port/default/port_from_tf/status.h"

#include <assert.h>
#include <stdio.h>

#include "port/default/error_codes.h"

namespace platforms {
namespace darwinn {
namespace util {

Status::Status(error::Code code, const std::string& msg) {
  assert(code != error::OK);
  state_ = std::unique_ptr<State>(new State);
  state_->code = code;
  state_->msg = msg;
}

void Status::Update(const Status& new_status) {
  if (ok()) {
    *this = new_status;
  }
}

void Status::SlowCopyFrom(const State* src) {
  if (src == nullptr) {
    state_ = nullptr;
  } else {
    state_ = std::unique_ptr<State>(new State(*src));
  }
}

const std::string& Status::empty_string() {
  static std::string* empty = new std::string;
  return *empty;
}

std::string Status::ToString() const {
  if (state_ == nullptr) {
    return "OK";
  } else {
    char tmp[30];
    const char* type;
    switch (code()) {
      case error::CANCELLED:
        type = "Cancelled";
        break;
      case error::UNKNOWN:
        type = "Unknown";
        break;
      case error::INVALID_ARGUMENT:
        type = "Invalid argument";
        break;
      case error::DEADLINE_EXCEEDED:
        type = "Deadline exceeded";
        break;
      case error::NOT_FOUND:
        type = "Not found";
        break;
      case error::ALREADY_EXISTS:
        type = "Already exists";
        break;
      case error::PERMISSION_DENIED:
        type = "Permission denied";
        break;
      case error::UNAUTHENTICATED:
        type = "Unauthenticated";
        break;
      case error::RESOURCE_EXHAUSTED:
        type = "Resource exhausted";
        break;
      case error::FAILED_PRECONDITION:
        type = "Failed precondition";
        break;
      case error::ABORTED:
        type = "Aborted";
        break;
      case error::OUT_OF_RANGE:
        type = "Out of range";
        break;
      case error::UNIMPLEMENTED:
        type = "Unimplemented";
        break;
      case error::INTERNAL:
        type = "Internal";
        break;
      case error::UNAVAILABLE:
        type = "Unavailable";
        break;
      case error::DATA_LOSS:
        type = "Data loss";
        break;
      default:
        snprintf(tmp, sizeof(tmp), "Unknown code(%d)",
                 static_cast<int>(code()));
        type = tmp;
        break;
    }
    std::string result(type);
    result += ": ";
    result += state_->msg;
    return result;
  }
}

void Status::IgnoreError() const {
  // no-op
}

std::ostream& operator<<(std::ostream& os, const Status& x) {
  os << x.ToString();
  return os;
}

std::string* CheckOpHelperOutOfLine(const Status& v, const char* msg) {
  std::string r("Non-OK-status: ");
  r += msg;
  r += " status: ";
  r += v.ToString();
  // Leaks string but this is only to be used in a fatal error message
  return new std::string(r);
}

}  // namespace util
}  // namespace darwinn
}  // namespace platforms
