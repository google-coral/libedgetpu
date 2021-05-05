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

#ifndef DARWINN_DRIVER_MMIO_COHERENT_ALLOCATOR_H_
#define DARWINN_DRIVER_MMIO_COHERENT_ALLOCATOR_H_

#include <stddef.h>
#include <mutex>  // NOLINT

#include "api/buffer.h"
#include "port/status.h"
#include "port/statusor.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Manage Device Specific DMA-able Coherent memory.
class CoherentAllocator {
 public:
  CoherentAllocator();
  CoherentAllocator(int alignment_bytes, size_t size_bytes);
  virtual ~CoherentAllocator() = default;

  // Opens coherent allocator.
  Status Open() LOCKS_EXCLUDED(mutex_);

  // Closes coherent allocator.
  Status Close() LOCKS_EXCLUDED(mutex_);

  // Returns a chunk of coherent memory.
  StatusOr<Buffer> Allocate(size_t size_bytes) LOCKS_EXCLUDED(mutex_);

 protected:
  // Implements Open.
  virtual StatusOr<char *> DoOpen(size_t size_bytes);

  // Implements close.
  virtual Status DoClose(char *mem_base, size_t size_bytes);

 private:
  // Alignment bytes for host memory.
  const int alignment_bytes_;

  // User-space virtual address of memory block.
  char *coherent_memory_base_{nullptr};

  // Total size of coherent memory region.
  const size_t total_size_bytes_;

  // Coherent Bytes allocated so far.
  size_t allocated_bytes_ GUARDED_BY(mutex_){0};

  // Guards all APIs functions Open/Close/Allocate.
  mutable std::mutex mutex_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_MMIO_COHERENT_ALLOCATOR_H_
