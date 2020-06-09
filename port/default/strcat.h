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

#ifndef DARWINN_PORT_STRCAT_H_
#define DARWINN_PORT_STRCAT_H_

#include <sstream>
#include <string>

namespace platforms {
namespace darwinn {

template <typename... Args>
std::string StrCat(Args const&... args) {
  std::ostringstream stream;
  int temp[]{0, ((void)(stream << args), 0)...};
  (void)temp;
  return stream.str();
}

template <typename... Args>
void StrAppend(std::string* dest, Args const&... args) {
  dest->append(StrCat(args...));  // NOLINT
}

}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_PORT_STRCAT_H_
