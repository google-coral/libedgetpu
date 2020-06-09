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

#ifndef DARWINN_PORT_DEFAULT_PORT_FROM_TF_ERRORS_H_
#define DARWINN_PORT_DEFAULT_PORT_FROM_TF_ERRORS_H_

#include "port/default/error_codes.h"
#include "port/default/port_from_tf/logging.h"
#include "port/default/port_from_tf/macros.h"
#include "port/default/port_from_tf/status.h"
#include "port/default/strcat.h"

namespace platforms {
namespace darwinn {
namespace util {

typedef error::Code Code;

// Append some context to an error message.  Each time we append
// context put it on a new line, since it is possible for there
// to be several layers of additional context.
template <typename... Args>
void AppendToMessage(Status* status, Args... args) {
  *status =
      Status(status->code(), StrCat(status->error_message(), "\n\t", args...));
}

// Convenience functions for generating and using error status.
// Example usage:
//   status.Update(error::InvalidArgumentError("The ", foo, " isn't right."));
//   if (error::IsInvalidArgument(status)) { ... }
//   switch (status.code()) { case error::INVALID_ARGUMENT: ... }

#define DECLARE_ERROR(FUNC, CONST)                                            \
  template <typename... Args>                                                 \
  Status FUNC##Error(Args... args) {                                          \
    return Status(::platforms::darwinn::util::error::CONST, StrCat(args...)); \
  }                                                                           \
  inline bool Is##FUNC(const Status& status) {                                \
    return status.code() == ::platforms::darwinn::util::error::CONST;         \
  }

DECLARE_ERROR(Cancelled, CANCELLED)
DECLARE_ERROR(InvalidArgument, INVALID_ARGUMENT)
DECLARE_ERROR(NotFound, NOT_FOUND)
DECLARE_ERROR(AlreadyExists, ALREADY_EXISTS)
DECLARE_ERROR(ResourceExhausted, RESOURCE_EXHAUSTED)
DECLARE_ERROR(Unavailable, UNAVAILABLE)
DECLARE_ERROR(FailedPrecondition, FAILED_PRECONDITION)
DECLARE_ERROR(OutOfRange, OUT_OF_RANGE)
DECLARE_ERROR(Unimplemented, UNIMPLEMENTED)
DECLARE_ERROR(Internal, INTERNAL)
DECLARE_ERROR(Aborted, ABORTED)
DECLARE_ERROR(DeadlineExceeded, DEADLINE_EXCEEDED)
DECLARE_ERROR(DataLoss, DATA_LOSS)
DECLARE_ERROR(Unknown, UNKNOWN)
DECLARE_ERROR(PermissionDenied, PERMISSION_DENIED)
DECLARE_ERROR(Unauthenticated, UNAUTHENTICATED)

#undef DECLARE_ERROR

// The CanonicalCode() for non-errors.
using ::platforms::darwinn::util::error::OK;

}  // namespace util
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_PORT_DEFAULT_PORT_FROM_TF_ERRORS_H_
