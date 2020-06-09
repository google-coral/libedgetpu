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

#ifndef DARWINN_PORT_DEFAULT_PORT_FROM_TF_STATUS_H_
#define DARWINN_PORT_DEFAULT_PORT_FROM_TF_STATUS_H_

#include <stddef.h>
#include <functional>
#include <iosfwd>
#include <memory>
#include <string>

#include "port/default/error_codes.h"
#include "port/default/port_from_tf/logging.h"

namespace platforms {
namespace darwinn {
namespace util {

/// Denotes success or failure of a call.
class Status {
 public:
  /// Create a success status.
  Status() {}

  /// \brief Create a status with the specified error code and msg as a
  /// human-readable string containing more detailed information.
  Status(error::Code code, const std::string& msg);

  /// Copy the specified status.
  Status(const Status& s);
  void operator=(const Status& s);

  static Status OK() { return Status(); }

  /// Returns true iff the status indicates success.
  bool ok() const { return (state_ == NULL); }

  error::Code code() const { return ok() ? error::OK : state_->code; }
  error::Code CanonicalCode() const { return code(); }

  const std::string& error_message() const {
    return ok() ? empty_string() : state_->msg;
  }
  const std::string& message() const {
    return ok() ? empty_string() : state_->msg;
  }

  bool operator==(const Status& x) const;
  bool operator!=(const Status& x) const;

  /// \brief If `ok()`, stores `new_status` into `*this`.  If `!ok()`,
  /// preserves the current status, but may augment with additional
  /// information about `new_status`.
  ///
  /// Convenient way of keeping track of the first error encountered.
  /// Instead of:
  ///   `if (overall_status.ok()) overall_status = new_status`
  /// Use:
  ///   `overall_status.Update(new_status);`
  void Update(const Status& new_status);

  /// \brief Return a string representation of this status suitable for
  /// printing. Returns the string `"OK"` for success.
  std::string ToString() const;

  // Ignores any errors. This method does nothing except potentially suppress
  // complaints from any tools that are checking that errors are not dropped on
  // the floor.
  void IgnoreError() const;

 private:
  static const std::string& empty_string();
  struct State {
    error::Code code;
    std::string msg;
  };
  // OK status has a `NULL` state_.  Otherwise, `state_` points to
  // a `State` structure containing the error code and message(s)
  std::unique_ptr<State> state_;

  void SlowCopyFrom(const State* src);
};

inline Status OkStatus() { return Status(); }

inline Status::Status(const Status& s)
    : state_((s.state_ == NULL) ? NULL : new State(*s.state_)) {}

inline void Status::operator=(const Status& s) {
  // The following condition catches both aliasing (when this == &s),
  // and the common case where both s and *this are ok.
  if (state_ != s.state_) {
    SlowCopyFrom(s.state_.get());
  }
}

inline bool Status::operator==(const Status& x) const {
  return (this->state_ == x.state_) || (ToString() == x.ToString());
}

inline bool Status::operator!=(const Status& x) const { return !(*this == x); }

/// @ingroup core
std::ostream& operator<<(std::ostream& os, const Status& x);

typedef std::function<void(const Status&)> StatusCallback;

extern std::string* CheckOpHelperOutOfLine(const Status& v, const char* msg);

inline std::string* CheckOpHelper(Status v, const char* msg) {
  if (v.ok()) return nullptr;
  return CheckOpHelperOutOfLine(v, msg);
}

#define DO_CHECK_OK(val, level) \
  while (auto _result = CheckOpHelper(val, #val)) LOG(level) << *(_result)

#define CHECK_OK(val) DO_CHECK_OK(val, FATAL)
#define QCHECK_OK(val) DO_CHECK_OK(val, QFATAL)

// DEBUG only version of CHECK_OK.  Compiler still parses 'val' even in opt
// mode.
#ifndef NDEBUG
#define DCHECK_OK(val) CHECK_OK(val)
#else
#define DCHECK_OK(val) \
  while (false && (Status::OK() == (val))) LOG(FATAL)
#endif

}  // namespace util
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_PORT_DEFAULT_PORT_FROM_TF_STATUS_H_
