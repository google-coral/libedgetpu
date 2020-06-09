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

#ifndef DARWINN_API_EXECUTION_CONTEXT_INTERFACE_H_
#define DARWINN_API_EXECUTION_CONTEXT_INTERFACE_H_

namespace platforms {
namespace darwinn {
namespace api {

// This class stores information related to on-device execution on the TPU. This
// empty base interface may be inherited to store any kind of execution related
// info. Info may include ML model information, process name, etc. This class is
// NOT thread-safe.
class ExecutionContextInterface {
 public:
  virtual ~ExecutionContextInterface() = default;
};

}  // namespace api
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_API_EXECUTION_CONTEXT_INTERFACE_H_
