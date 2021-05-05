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
#include "util/task/canonical_errors.h"

namespace platforms {
namespace darwinn {

// Allows portable clients to use platforms::darwinn::*Errors.
using ::util::AbortedError;
using ::util::AlreadyExistsError;
using ::util::CancelledError;
using ::util::DataLossError;
using ::util::DeadlineExceededError;
using ::util::FailedPreconditionError;
using ::util::InternalError;
using ::util::InvalidArgumentError;
using ::util::IsAborted;
using ::util::IsAlreadyExists;
using ::util::IsCancelled;
using ::util::IsDataLoss;
using ::util::IsDeadlineExceeded;
using ::util::IsFailedPrecondition;
using ::util::IsInternal;
using ::util::IsInvalidArgument;
using ::util::IsNotFound;
using ::util::IsOutOfRange;
using ::util::IsPermissionDenied;
using ::util::IsResourceExhausted;
using ::util::IsUnauthenticated;
using ::util::IsUnavailable;
using ::util::IsUnimplemented;
using ::util::IsUnknown;
using ::util::NotFoundError;
using ::util::OutOfRangeError;
using ::util::PermissionDeniedError;
using ::util::ResourceExhaustedError;
using ::util::UnauthenticatedError;
using ::util::UnavailableError;
using ::util::UnimplementedError;
using ::util::UnknownError;

}  // namespace darwinn
}  // namespace platforms

#else  // !DARWINN_PORT_USE_GOOGLE3
#include "port/default/errors.h"
#endif  // DARWINN_PORT_USE_GOOGLE3
// IWYU pragma: end_exports

#endif  // DARWINN_PORT_ERRORS_H_
