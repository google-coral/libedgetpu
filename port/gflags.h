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

#ifndef DARWINN_PORT_GFLAGS_H_
#define DARWINN_PORT_GFLAGS_H_

#include "port/defs.h"

#if defined(DARWINN_PORT_ANDROID_SYSTEM) || \
    defined(DARWINN_PORT_ANDROID_EMULATOR)
// There is no gflags implementation in Android runtime. Provide a dummy
// implementation here.
#define ABSL_FLAG(type, name, val, desc) type FLAGS_##name = val
namespace absl {
  template<typename T>
  T GetFlag(T t) { return t; }
}
inline void ParseFlags(int argc, char* argv[]) {}
#else
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

inline void ParseFlags(int argc, char* argv[]) {
  absl::ParseCommandLine(argc, argv);
}
#endif

#endif  // DARWINN_PORT_GFLAGS_H_
