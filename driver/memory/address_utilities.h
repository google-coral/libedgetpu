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

// Few utilities for manipulating host and device addresses.
// Note: In general, host addresses are pointers (void* buffer), where as device
// addresses are uint64.

#ifndef DARWINN_DRIVER_MEMORY_ADDRESS_UTILITIES_H_
#define DARWINN_DRIVER_MEMORY_ADDRESS_UTILITIES_H_

#include <stddef.h>

#include "driver/hardware_structures.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/math_util.h"
#include "port/status.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Get the offset into a page for a given address.
static inline uint64 GetPageOffset(uint64 address) {
  return address & (kHostPageSize - 1);
}

// Get the offset into a page for a given address.
static inline uint64 GetPageOffset(const void *buffer) {
  return GetPageOffset(reinterpret_cast<uintptr_t>(buffer));
}

// Returns true, if page aligned.
static inline bool IsPageAligned(const void *buffer) {
  return (GetPageOffset(buffer) == 0);
}

// Returns true, if page aligned.
static inline bool IsPageAligned(uint64 address) {
  return (GetPageOffset(address) == 0);
}

// Get the number of pages required to back a given address range.
static inline uint64 GetNumberPages(const void *buffer, size_t size_bytes) {
  return MathUtil::CeilOfRatio(GetPageOffset(buffer) + size_bytes,
                               kHostPageSize);
}

// Get the number of pages required to back a given address range.
static inline uint64 GetNumberPages(uint64 address, size_t size_bytes) {
  return MathUtil::CeilOfRatio(GetPageOffset(address) + size_bytes,
                               kHostPageSize);
}

// Get the page-aligned address for a given address
static inline uint64 GetPageAddress(uint64 address) {
  return address - GetPageOffset(address);
}

// Get the page-aligned address for a given buffer.
static inline const void *GetPageAddressForBuffer(const void *buffer) {
  return static_cast<const char *>(buffer) - GetPageOffset(buffer);
}

// Get the page address (in terms of kHostPageSize) for a given page number.
static constexpr uint64 GetPageAddressFromNumber(uint64 page_num) {
  return (page_num << kHostPageShiftBits);
}

// Get the page number (in terms of kHostPageSize) for a given address.
static constexpr uint64 GetPageNumberFromAddress(uint64 address) {
  return address >> kHostPageShiftBits;
}

// Returns whether the given address satisifes the given alighment.
static inline util::Status IsAligned(const uint8 *buffer,
                                     uint64 alignment_bytes) {
  if ((reinterpret_cast<uintptr_t>(buffer) % alignment_bytes) != 0) {
    return util::FailedPreconditionError(
        StringPrintf("Buffer is not aligned. address=%p, alignment=%llu.",
                     buffer, static_cast<unsigned long long>(alignment_bytes)));
  }
  return util::Status();  // OK
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_MEMORY_ADDRESS_UTILITIES_H_
