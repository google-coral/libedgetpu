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

#include "driver/memory/mmio_address_space.h"

#include "api/buffer.h"
#include "driver/memory/address_utilities.h"
#include "port/errors.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/statusor.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"
#include "port/tracing.h"

namespace platforms {
namespace darwinn {
namespace driver {

util::Status MmioAddressSpace::Map(const Buffer& buffer, uint64 device_address,
                                   DmaDirection direction) {
  TRACE_SCOPE("MmioAddressSpace::Map");
  CHECK(IsPageAligned(device_address));

  StdMutexLock lock(&mutex_);

  // If already mapped, fail.
  // TODO: Add a finer grained check, e.g., overlap, if necessary?
  if (mapped_.find(device_address) != mapped_.end()) {
    return util::InvalidArgumentError(
        "Trying to map a segment that is already mapped.");
  }

  RETURN_IF_ERROR(mmu_mapper_->Map(buffer, device_address, direction));

  // Track mapped segments.
  // Make a copy of Buffer since the given buffer may change later.
  auto insert_result = mapped_.insert(
      {device_address, buffer});
  CHECK(insert_result.second);

  // buffer.ptr() may or may not be vaild.
  // TODO: print out buffer address if the buffer has valid ptr().
  VLOG(4) << StringPrintf(
      "MapMemory() page-aligned : device_address = 0x%016llx",
      static_cast<unsigned long long>(device_address));  // NOLINT(runtime/int)

  return util::Status();  // OK
}

util::Status MmioAddressSpace::Unmap(uint64 device_address,
                                     int num_released_pages) {
  TRACE_SCOPE("MmioAddressSpace::Unmap");
  // TODO: verify num_released_pages if the Buffer is backed by host
  // memory.
  CHECK(IsPageAligned(device_address));

  StdMutexLock lock(&mutex_);

  auto find_result = mapped_.find(device_address);
  if (find_result == mapped_.end()) {
    return util::InvalidArgumentError(
        "Trying to ummap a segment that is not already mapped.");
  }

  // Need to pass the Buffer object as the MMU mapper might require the backing
  // file descriptor underneath.
  RETURN_IF_ERROR(mmu_mapper_->Unmap(find_result->second, device_address));

  // buffer.ptr() may or may not be vaild.
  // TODO: print out buffer address if the buffer has valid ptr().
  VLOG(4) << StringPrintf(
      "UnmapMemory() page-aligned : device_address = 0x%016llx, num_pages = %d",
      static_cast<unsigned long long>(device_address),  // NOLINT(runtime/int)
      num_released_pages);

  mapped_.erase(find_result);

  return util::Status();  // OK
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
