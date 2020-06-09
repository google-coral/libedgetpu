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

// Copyright (C) 1999 and onwards Google, Inc.
//
// Various portability macros, type definitions, and inline functions
// This file is used for both C and C++!
//
// These are weird things we need to do to get this compiling on
// random systems (and on SWIG).
//
// MOE:begin_strip
// This file is open source. You may export it with your open source projects
// as long as you use MOE to strip proprietary comments.
// MOE:end_strip

#ifndef DARWINN_PORT_DEFAULT_ALIGNED_MALLOC_H_
#define DARWINN_PORT_DEFAULT_ALIGNED_MALLOC_H_

#if defined(_WIN32)
#include "port/default/aligned_malloc_windows.h"
#else
#include "port/default/aligned_malloc_default.h"
#endif

#endif  // DARWINN_PORT_DEFAULT_ALIGNED_MALLOC_H_
