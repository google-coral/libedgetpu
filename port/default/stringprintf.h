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

// Copyright 2002 and onwards Google Inc.
// Author: Sanjay Ghemawat
//
// Printf variants that place their output in a C++ string.
//
// Usage:
//      string result = StringPrintf("%d %s\n", 10, "hello");
//
// While StringF and StreamF are recommended for use, they are difficult to
// port. Fallback to StringPrintf as an alternative as it is easier to port.

#ifndef DARWINN_PORT_DEFAULT_STRINGPRINTF_H_
#define DARWINN_PORT_DEFAULT_STRINGPRINTF_H_

#include <stdarg.h>
#include <string>
#include <vector>

#include "port/default/macros.h"

namespace platforms {
namespace darwinn {

// Returns a C++ string
std::string StringPrintf(const char* format, ...)
    // Tell the compiler to do printf format string checking.
    PRINTF_ATTRIBUTE(1,2);

}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_PORT_DEFAULT_STRINGPRINTF_H_
