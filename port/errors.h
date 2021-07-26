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

#ifndef DARWINN_PORT_ERRORS_H_
#define DARWINN_PORT_ERRORS_H_

#include "port/defs.h"

// IWYU pragma: begin_exports
#if DARWINN_PORT_USE_GOOGLE3
#include "absl/status/status.h"
#include "util/task/canonical_errors.h"

namespace platforms {
namespace darwinn {

// Allows portable clients to use platforms::darwinn::*Errors.
using ::absl::AbortedError;
using ::absl::AlreadyExistsError;
using ::absl::CancelledError;
using ::absl::DataLossError;
using ::absl::DeadlineExceededError;
using ::absl::FailedPreconditionError;
using ::absl::InternalError;
using ::absl::InvalidArgumentError;
using ::absl::IsAborted;
using ::absl::IsAlreadyExists;
using ::absl::IsCancelled;
using ::absl::IsDataLoss;
using ::absl::IsDeadlineExceeded;
using ::absl::IsFailedPrecondition;
using ::absl::IsInternal;
using ::absl::IsInvalidArgument;
using ::absl::IsNotFound;
using ::absl::IsOutOfRange;
using ::absl::IsPermissionDenied;
using ::absl::IsResourceExhausted;
using ::absl::IsUnauthenticated;
using ::absl::IsUnavailable;
using ::absl::IsUnimplemented;
using ::absl::IsUnknown;
using ::absl::NotFoundError;
using ::absl::OutOfRangeError;
using ::absl::PermissionDeniedError;
using ::absl::ResourceExhaustedError;
using ::absl::UnauthenticatedError;
using ::absl::UnavailableError;
using ::absl::UnimplementedError;
using ::absl::UnknownError;

}  // namespace darwinn
}  // namespace platforms

#else  // !DARWINN_PORT_USE_GOOGLE3
#include "port/default/errors.h"
#endif  // DARWINN_PORT_USE_GOOGLE3
// IWYU pragma: end_exports

#endif  // DARWINN_PORT_ERRORS_H_
