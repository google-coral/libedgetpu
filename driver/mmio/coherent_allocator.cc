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

#include "driver/mmio/coherent_allocator.h"

#include "api/buffer.h"
#include "port/aligned_malloc.h"
#include "port/errors.h"
#include "port/status_macros.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace {

constexpr const size_t kDefaultMaxCoherentBytes = 0x10000;
constexpr const size_t kDefaultAlignmentBytes = 8;

}  // namespace

CoherentAllocator::CoherentAllocator(int alignment_bytes, size_t size_bytes)
    : alignment_bytes_(alignment_bytes), total_size_bytes_(size_bytes) {
  CHECK_GT(total_size_bytes_, 0);
}

CoherentAllocator::CoherentAllocator()
    : CoherentAllocator(kDefaultAlignmentBytes, kDefaultMaxCoherentBytes) {}

util::Status CoherentAllocator::Open() {
  StdMutexLock lock(&mutex_);
  if (coherent_memory_base_ != nullptr) {
    return util::FailedPreconditionError("Device already open.");
  }

  ASSIGN_OR_RETURN(coherent_memory_base_, DoOpen(total_size_bytes_));

  return util::Status();  // OK
}

util::StatusOr<char *> CoherentAllocator::DoOpen(size_t size_bytes) {
  char *mem_base =
      static_cast<char *>(aligned_malloc(total_size_bytes_, alignment_bytes_));
  if (mem_base == nullptr) {
    return util::FailedPreconditionError(
        StringPrintf("Could not malloc %zu bytes.", total_size_bytes_));
  }
  memset(mem_base, 0, size_bytes);
  return mem_base;  // OK
}

util::StatusOr<Buffer> CoherentAllocator::Allocate(size_t size_bytes) {
  StdMutexLock lock(&mutex_);
  if (size_bytes == 0) {
    return util::FailedPreconditionError("Allocate null size.");
  }

  if (coherent_memory_base_ == nullptr) {
    return util::FailedPreconditionError("Not Opened.");
  }

  if ((allocated_bytes_ + size_bytes) > total_size_bytes_) {
    return util::FailedPreconditionError(StringPrintf(
        "CoherentAllocator: Allocate size = %zu and no memory (total = %zu).",
        size_bytes, total_size_bytes_));
  }

  char *p = coherent_memory_base_ + allocated_bytes_;

  // Power of 2 pointer arithmetic: align the block boundary on chip specific
  // byte alignment
  size_t mask = alignment_bytes_ - 1;
  allocated_bytes_ += (size_bytes + mask) & ~mask;

  return Buffer(p, size_bytes);
}

util::Status CoherentAllocator::DoClose(char *mem_base, size_t size_bytes) {
  if (mem_base != nullptr) {
    aligned_free(mem_base);
  }
  return util::Status();  // OK
}

util::Status CoherentAllocator::Close() {
  StdMutexLock lock(&mutex_);
  auto status = DoClose(coherent_memory_base_, total_size_bytes_);
  // Resets state.
  allocated_bytes_ = 0;
  coherent_memory_base_ = nullptr;

  return status;
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
