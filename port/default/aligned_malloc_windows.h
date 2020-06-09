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

#ifndef DARWINN_PORT_DEFAULT_ALIGNED_MALLOC_WINDOWS_H_
#define DARWINN_PORT_DEFAULT_ALIGNED_MALLOC_WINDOWS_H_

#include <malloc.h>  // for _aligned_{malloc/free}()

namespace platforms {
namespace darwinn {

inline void *aligned_malloc(size_t size, int minimum_alignment) {
  return _aligned_malloc(size, minimum_alignment);
}

inline void aligned_free(void *aligned_memory) {
  _aligned_free(aligned_memory);
}

}  // namespace darwinn
}  // namespace platforms


#endif  // DARWINN_PORT_DEFAULT_ALIGNED_MALLOC_WINDOWS_H_
