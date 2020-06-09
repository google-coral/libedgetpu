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

#ifndef DARWINN_PORT_DEMANGLE_H_
#define DARWINN_PORT_DEMANGLE_H_

#include "port/defs.h"

#if DARWINN_PORT_USE_GOOGLE3

#include <string>
#include <typeinfo>

#include "base/demangle.h"

// Demangle given type.  On success, return true and write the
// demangled symbol name to `out`.  Otherwise, return false.
// `out` is modified even if demangling is unsuccessful.
template <typename T>
bool Demangle(char *out, int out_size) {
  std::string mangled("_Z");
  mangled.append(typeid(T).name());
  return ::Demangle(mangled.c_str(), out, out_size);
}

#else  // !DARWINN_PORT_USE_GOOGLE3

// Demangle given type.  On success, return true and write the
// demangled symbol name to `out`.  Otherwise, return false.
// `out` is modified even if demangling is unsuccessful.
template <typename T>
bool Demangle(char *out, int out_size) {
  return false;
}

#endif  // !DARWINN_PORT_USE_GOOGLE3

#endif  // DARWINN_PORT_DEMANGLE_H_
