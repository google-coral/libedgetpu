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

#ifndef DARWINN_DRIVER_MEMORY_MMIO_ADDRESS_SPACE_H_
#define DARWINN_DRIVER_MEMORY_MMIO_ADDRESS_SPACE_H_

#include <stddef.h>
#include <map>
#include <mutex>  // NOLINT

#include "api/buffer.h"
#include "driver/memory/address_space.h"
#include "driver/memory/address_utilities.h"
#include "driver/memory/dma_direction.h"
#include "driver/memory/mmu_mapper.h"
#include "port/integral_types.h"
#include "port/logging.h"
#include "port/statusor.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// A class to manage a DarwiNN virtual address space segment when mmio is used.
class MmioAddressSpace : public AddressSpace {
 public:
  MmioAddressSpace(uint64 device_virtual_address_start,
                   uint64 device_virtual_address_size_bytes,
                   MmuMapper* mmu_mapper)
      : AddressSpace(),
        device_virtual_address_start_(device_virtual_address_start),
        device_virtual_address_size_bytes_(device_virtual_address_size_bytes),
        mmu_mapper_(mmu_mapper) {
    CHECK(mmu_mapper != nullptr);
    CHECK(IsPageAligned(device_virtual_address_start));
    CHECK(IsPageAligned(device_virtual_address_size_bytes));
  }

  // This class is neither copyable nor movable.
  MmioAddressSpace(const MmioAddressSpace&) = delete;
  MmioAddressSpace& operator=(const MmioAddressSpace&) = delete;

  virtual ~MmioAddressSpace() = default;

 protected:
  // Maps the entire given Buffer, and stores the mapping information. Returns
  // an error if trying to map already mapped Buffer.
  Status Map(const Buffer& buffer, uint64 device_address,
             DmaDirection direction) LOCKS_EXCLUDED(mutex_);

  // Checks and unmaps device virtual address. |device_address| must be
  // page-aligned.
  Status Unmap(uint64 device_address, int num_released_pages)
      LOCKS_EXCLUDED(mutex_);

  // Member accessors for inherited classes.
  uint64 device_virtual_address_start() const {
    return device_virtual_address_start_;
  }
  uint64 device_virtual_address_size_bytes() const {
    return device_virtual_address_size_bytes_;
  }

  // Returns last device virtual address.
  uint64 GetLastDeviceVirtualAddress() const {
    return device_virtual_address_start_ + device_virtual_address_size_bytes_;
  }

 private:
  // Device address space start.
  const uint64 device_virtual_address_start_;

  // Device address space size in bytes.
  const uint64 device_virtual_address_size_bytes_;

  // Underlying MMU mapper.
  MmuMapper* const mmu_mapper_ GUARDED_BY(mutex_);

  // Guards |mmu_mapper_| and |mapped_|.
  mutable std::mutex mutex_;

  // Tracks already mapped segments.
  // key - aligned device virtual address.
  // value - {host address, number of mapped pages}
  std::map<uint64, Buffer> mapped_ GUARDED_BY(mutex_);
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_MEMORY_MMIO_ADDRESS_SPACE_H_
