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

#include "port/default/status_macros.h"

#include "port/default/logging.h"
#include "port/default/strcat.h"

namespace platforms {
namespace darwinn {
namespace util {
namespace status_macros {

// TODO: Implement
static std::string CurrentStackTrace() { return ""; }

static Status MakeStatus(error::Code code, const std::string& message) {
  return Status(code, message);
}

// Log the error at the given severity, optionally with a stack trace.
// If log_severity is NUM_SEVERITIES, nothing is logged.
static void LogError(const Status& status, const char* filename, int line,
                     int log_severity, bool should_log_stack_trace) {
  if (PREDICT_TRUE(log_severity != NUM_SEVERITIES)) {
    darwinn::internal::LogMessage log_message(filename, line, log_severity);
    log_message << status;
    if (should_log_stack_trace) {
      log_message << "\n" << CurrentStackTrace();
    }
    // Logging actually happens in LogMessage destructor.
  }
}

// Make a Status with a code, error message and payload,
// and also send it to LOG(<log_severity>) using the given filename
// and line (unless should_log is false, or log_severity is
// NUM_SEVERITIES).  If should_log_stack_trace is true, the stack
// trace is included in the log message (ignored if should_log is
// false).
static Status MakeError(const char* filename, int line, error::Code code,
                        const std::string& message, bool should_log,
                        int log_severity, bool should_log_stack_trace) {
  if (PREDICT_FALSE(code == error::OK)) {
    DCHECK(false) << "Cannot create error with status OK";
    code = error::UNKNOWN;
  }
  const Status status = MakeStatus(code, message);
  if (PREDICT_TRUE(should_log)) {
    LogError(status, filename, line, log_severity, should_log_stack_trace);
  }
  return status;
}

// This method is written out-of-line rather than in the header to avoid
// generating a lot of inline code for error cases in all callers.
void MakeErrorStream::CheckNotDone() const { impl_->CheckNotDone(); }

MakeErrorStream::Impl::Impl(const char* file, int line, error::Code code,
                            MakeErrorStream* error_stream,
                            bool is_logged_by_default)
    : file_(file),
      line_(line),
      code_(code),
      is_done_(false),
      should_log_(is_logged_by_default),
      log_severity_(ERROR),
      should_log_stack_trace_(false),
      make_error_stream_with_output_wrapper_(error_stream) {}

MakeErrorStream::Impl::Impl(const Status& status,
                            PriorMessageHandling prior_message_handling,
                            const char* file, int line,
                            MakeErrorStream* error_stream)
    : file_(file),
      line_(line),
      // Make sure we show some error, even if the call is incorrect.
      code_(!status.ok() ? status.code() : error::UNKNOWN),
      prior_message_handling_(prior_message_handling),
      prior_message_(status.error_message()),
      is_done_(false),
      // Error code type is not visible here, so we can't call
      // IsLoggedByDefault.
      should_log_(true),
      log_severity_(ERROR),
      should_log_stack_trace_(false),
      make_error_stream_with_output_wrapper_(error_stream) {
  DCHECK(!status.ok()) << "Attempted to append/prepend error text to status OK";
}

MakeErrorStream::Impl::~Impl() {
  // Note: error messages refer to the public MakeErrorStream class.

  DCHECK(is_done_) << "MakeErrorStream destructed without getting Status: "
                   << file_ << ":" << line_ << " " << stream_.str();
}

Status MakeErrorStream::Impl::GetStatus() {
  // Note: error messages refer to the public MakeErrorStream class.

  // Getting a Status object out more than once is not harmful, but
  // it doesn't match the expected pattern, where the stream is constructed
  // as a temporary, loaded with a message, and then casted to Status.
  DCHECK(!is_done_) << "MakeErrorStream got Status more than once: " << file_
                    << ":" << line_ << " " << stream_.str();

  is_done_ = true;

  const std::string& stream_str = stream_.str();
  const std::string str = prior_message_handling_ == kAppendToPriorMessage
                              ? StrCat(prior_message_, stream_str)
                              : StrCat(stream_str, prior_message_);
  if (PREDICT_FALSE(str.empty())) {
    return MakeError(
        file_, line_, code_,
        StrCat(str, "Error without message at ", file_, ":", line_),
        true /* should_log */, ERROR /* log_severity */,
        should_log_stack_trace_);
  } else {
    return MakeError(file_, line_, code_, str, should_log_, log_severity_,
                     should_log_stack_trace_);
  }
}

void MakeErrorStream::Impl::CheckNotDone() const {
  DCHECK(!is_done_) << "MakeErrorStream shift called after getting Status: "
                    << file_ << ":" << line_ << " " << stream_.str();
}

}  // namespace status_macros
}  // namespace util
}  // namespace darwinn
}  // namespace platforms
