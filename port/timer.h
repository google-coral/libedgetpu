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

#ifndef DARWINN_PORT_TIMER_H_
#define DARWINN_PORT_TIMER_H_

#if defined(_WIN32)
#include "port/timer_windows.h"
#elif defined(__APPLE__)
#include "port/timer_darwin.h"
#else
#include "port/timer_linux.h"
#endif  // defined(_WIN32)

#endif  // DARWINN_PORT_TIMER_H_
