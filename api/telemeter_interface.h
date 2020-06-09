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

#ifndef DARWINN_API_TELEMETER_INTERFACE_H_
#define DARWINN_API_TELEMETER_INTERFACE_H_

#include <string>

#include "api/execution_context_interface.h"

namespace platforms {
namespace darwinn {
namespace api {

// This class collects data related to on-device execution on the TPU, and may
// report them back to a server for further analysis. The data could be
// related to performance or execution failure.
// This class is thread-safe.
class TelemeterInterface {
 public:
  virtual ~TelemeterInterface() = default;

  // Logs the watchdog timeout event.
  virtual void LogWatchdogTimeout(const ExecutionContextInterface& context) = 0;
};

}  // namespace api
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_API_TELEMETER_INTERFACE_H_
