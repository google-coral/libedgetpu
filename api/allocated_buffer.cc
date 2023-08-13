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

#include <cstddef>

#include "api/allocated_buffer.h"

#include "port/logging.h"

namespace platforms {
namespace darwinn {

AllocatedBuffer::AllocatedBuffer(unsigned char* ptr, size_t size_bytes,
                                 FreeCallback free_callback)
    : ptr_(ptr),
      size_bytes_(size_bytes),
      free_callback_(std::move(free_callback)) {
  CHECK(ptr != nullptr);
}

AllocatedBuffer::~AllocatedBuffer() { free_callback_(ptr_); }

}  // namespace darwinn
}  // namespace platforms
