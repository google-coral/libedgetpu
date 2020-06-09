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

#include "driver/allocator.h"

#include <functional>
#include <memory>

#include "api/allocated_buffer.h"
#include "api/buffer.h"
#include "port/integral_types.h"
#include "port/ptr_util.h"

namespace platforms {
namespace darwinn {
namespace driver {

Buffer Allocator::MakeBuffer(size_t size_bytes) {
  auto free_cb = [this](void* ptr) { Free(ptr); };

  uint8* ptr = static_cast<uint8*>(Allocate(size_bytes));
  return Buffer(
      std::make_shared<AllocatedBuffer>(ptr, size_bytes, std::move(free_cb)));
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
