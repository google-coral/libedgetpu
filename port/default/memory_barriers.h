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

#ifndef DARWINN_PORT_DEFAULT_MEMORY_BARRIERS_H_
#define DARWINN_PORT_DEFAULT_MEMORY_BARRIERS_H_

#define barrier() asm volatile("" ::: "memory")
#define isb(option) asm volatile("" ::: "memory")
#define dsb(option) asm volatile("" ::: "memory")
#define dmb(option) asm volatile("" ::: "memory")
#define dfb(option) asm volatile("" ::: "memory")
#define rmb() asm volatile("" ::: "memory")
#define wmb() asm volatile("" ::: "memory")

#endif  // DARWINN_PORT_DEFAULT_MEMORY_BARRIERS_H_
