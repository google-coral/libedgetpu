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

#ifndef DARWINN_PORT_MUTEX_H_
#define DARWINN_PORT_MUTEX_H_

#include "port/defs.h"

// NOTE: This abstraction is brittle. Anything beyond Mutex#Lock(),
// Mutex#Unlock() is not guaranteed to work consistently across all platforms.

#if DARWINN_PORT_FIRMWARE
#include "third_party/safertos_addons/mutex.h"

namespace platforms {
namespace darwinn {

using Mutex = safertos_addons::Mutex;

}  // namespace darwinn
}  // namespace platforms

#elif DARWINN_PORT_USE_GOOGLE3
#include "absl/synchronization/mutex.h"

namespace platforms {
namespace darwinn {

using Mutex = absl::Mutex;

}  // namespace darwinn
}  // namespace platforms

#elif DARWINN_PORT_DEFAULT

#include "port/default/mutex.h"

#endif  // DARWINN_PORT_DEFAULT
#endif  // DARWINN_PORT_MUTEX_H_
