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

#ifndef DARWINN_PORT_STATUSOR_H_
#define DARWINN_PORT_STATUSOR_H_

#include "port/defs.h"

#if DARWINN_PORT_USE_GOOGLE3
#include "absl/status/statusor.h"

// TODO: Remove the following once DarwiNN clients removed util.
// Darwinn 1.0 code relies on util:: namespace to be portable across google3 and
// android.
namespace util {
template <typename T>
using StatusOr = absl::StatusOr<T>;
}  // namespace util

namespace platforms {
namespace darwinn {

// Defines platforms::darwinn::StatusOr, which points to absl statusor in
// google3.
template <typename T>
using StatusOr = absl::StatusOr<T>;
}  // namespace darwinn
}  // namespace platforms

#else  // !DARWINN_PORT_USE_GOOGLE3
#include "port/default/statusor.h"  // IWYU pragma: export
#endif  // DARWINN_PORT_USE_GOOGLE3

#endif  // DARWINN_PORT_STATUSOR_H_
