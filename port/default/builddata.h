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

#ifndef DARWINN_PORT_DEFAULT_BUILDDATA_H_
#define DARWINN_PORT_DEFAULT_BUILDDATA_H_

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#if defined(_WIN32)
#define COMPILER_VERSION "MSVC " TOSTRING(_MSC_FULL_VER)
#elif defined(__GNUC__)
#define COMPILER_VERSION __VERSION__
#else
#define COMPILER_VERSION "Unknown"
#endif

struct BuildData {
  static const char* BuildLabel() {
    return "COMPILER=" COMPILER_VERSION ",DATE=" __DATE__ ",TIME=" __TIME__;
  }
};

#endif  // DARWINN_PORT_DEFAULT_BUILDDATA_H_
