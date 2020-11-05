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

#ifndef DARWINN_PORT_STATUS_H_
#define DARWINN_PORT_STATUS_H_

#include "port/defs.h"

// IWYU pragma: begin_exports
#if DARWINN_PORT_USE_GOOGLE3
#include "util/task/status.h"
#else  // !DARWINN_PORT_USE_GOOGLE3
#include "port/default/status.h"
#endif  // DARWINN_PORT_USE_GOOGLE3
// IWYU pragma: end_exports

namespace platforms {
namespace darwinn {

#if DARWINN_PORT_USE_GOOGLE3
using StatusCode = ::absl::StatusCode;
#else   // !DARWINN_PORT_USE_GOOGLE3
using StatusCode = util::error::Code;
#endif  // DARWINN_PORT_USE_GOOGLE3

}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_PORT_STATUS_H_
