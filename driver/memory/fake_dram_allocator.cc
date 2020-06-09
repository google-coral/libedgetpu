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

#include "driver/memory/fake_dram_allocator.h"

namespace platforms {
namespace darwinn {
namespace driver {

FakeDramBuffer::FakeDramBuffer(size_t size_bytes) : size_bytes_(size_bytes) {
  ptr_ = malloc(size_bytes);
}

FakeDramBuffer::~FakeDramBuffer() { free(ptr_); }

util::Status FakeDramBuffer::ReadFrom(void* source) {
  memcpy(ptr_, source, size_bytes_);
  return util::OkStatus();
}

util::Status FakeDramBuffer::WriteTo(void* destination) {
  memcpy(destination, ptr_, size_bytes_);
  return util::OkStatus();
}

util::StatusOr<std::shared_ptr<DramBuffer>> FakeDramAllocator::AllocateBuffer(
    size_t size_bytes) {
  return {std::make_shared<FakeDramBuffer>(size_bytes)};
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
