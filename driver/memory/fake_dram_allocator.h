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

#ifndef DARWINN_DRIVER_MEMORY_FAKE_DRAM_ALLOCATOR_H_
#define DARWINN_DRIVER_MEMORY_FAKE_DRAM_ALLOCATOR_H_

#include "api/dram_buffer.h"
#include "driver/memory/dram_allocator.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Pretends to be an on-chip DRAM buffer while it actually is a host DRAM
// buffer. This is useful for reference driver and such.
class FakeDramBuffer : public DramBuffer {
 public:
  FakeDramBuffer(size_t size_bytes);
  ~FakeDramBuffer() override;

  int fd() const override { return 1; }
  size_t size_bytes() const override { return size_bytes_; }

  util::Status ReadFrom(void* source) override;
  util::Status WriteTo(void* destination) override;

 private:
  // Size of the buffer.
  size_t size_bytes_;

  // Pointer to start of the buffer.
  void* ptr_;
};

// A DRAM allocator that creates fake DRAM buffers. This is useful for reference
// driver and such.
class FakeDramAllocator : public DramAllocator {
 public:
  FakeDramAllocator() = default;
  ~FakeDramAllocator() override = default;

  // This class is neither copyable nor movable.
  FakeDramAllocator(const FakeDramAllocator&) = delete;
  FakeDramAllocator& operator=(const FakeDramAllocator&) = delete;

  util::Status Open() override { return util::OkStatus(); }
  util::Status Close() override { return util::OkStatus(); }

  util::StatusOr<std::shared_ptr<DramBuffer>> AllocateBuffer(
      size_t size_bytes) override;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_MEMORY_FAKE_DRAM_ALLOCATOR_H_
