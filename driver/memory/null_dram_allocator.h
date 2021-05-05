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

#ifndef DARWINN_DRIVER_MEMORY_NULL_DRAM_ALLOCATOR_H_
#define DARWINN_DRIVER_MEMORY_NULL_DRAM_ALLOCATOR_H_

#include "api/dram_buffer.h"
#include "driver/memory/dram_allocator.h"
#include "port/errors.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {
namespace driver {

// A DRAM allocator to be used for chips that do not have an on-chip DRAM.
class NullDramAllocator : public DramAllocator {
 public:
  NullDramAllocator() = default;
  ~NullDramAllocator() override = default;

  // This class is neither copyable nor movable.
  NullDramAllocator(const NullDramAllocator&) = delete;
  NullDramAllocator& operator=(const NullDramAllocator&) = delete;

  // Always returns an error.

  Status Open() override { return OkStatus(); }
  Status Close() override { return OkStatus(); }

  StatusOr<std::shared_ptr<DramBuffer>> AllocateBuffer(
      size_t size_bytes) override {
    return FailedPreconditionError("No on-chip DRAM available.");
  }
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_MEMORY_NULL_DRAM_ALLOCATOR_H_
