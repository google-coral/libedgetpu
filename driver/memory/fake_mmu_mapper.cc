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

#include "driver/memory/fake_mmu_mapper.h"

#include "driver/hardware_structures.h"
#include "driver/memory/address_utilities.h"
#include "port/integral_types.h"
#include "port/statusor.h"
#include "port/std_mutex_lock.h"

namespace platforms {
namespace darwinn {
namespace driver {

Status FakeMmuMapper::DoMap(const void *buffer, int num_pages,
                            uint64 device_virtual_address,
                            DmaDirection direction) {
  StdMutexLock lock(&mutex_);
  CHECK(IsPageAligned(buffer));
  CHECK(IsPageAligned(device_virtual_address));

  const uint8 *host_addr_start = static_cast<const uint8 *>(buffer);
  for (int i = 0; i < num_pages; ++i) {
    const uint8 *host_addr = i * kHostPageSize + host_addr_start;
    uint64 device_addr = i * kHostPageSize + device_virtual_address;

    CHECK(device_to_host_.find(device_addr) == device_to_host_.end());
    device_to_host_.emplace(device_addr, host_addr);
  }

  return Status();  // OK
}

Status FakeMmuMapper::DoMap(int fd, int num_pages,
                            uint64 device_virtual_address,
                            DmaDirection direction) {
  return DoMap(reinterpret_cast<void *>(fd * kHostPageSize), num_pages,
               device_virtual_address, direction);
}

Status FakeMmuMapper::DoUnmap(const void *buffer, int num_pages,
                              uint64 device_virtual_address) {
  StdMutexLock lock(&mutex_);
  CHECK(IsPageAligned(buffer));
  CHECK(IsPageAligned(device_virtual_address));

  for (int i = 0; i < num_pages; ++i) {
    uint64 device_addr = i * kHostPageSize + device_virtual_address;

    // TODO: Validate that the device virtual address and buffer
    // corresponds to the buffer that was originally mapped.
    CHECK(device_to_host_.find(device_addr) != device_to_host_.end());
    device_to_host_.erase(device_addr);
  }

  return Status();  // OK
}

Status FakeMmuMapper::DoUnmap(int fd, int num_pages,
                              uint64 device_virtual_address) {
  return DoUnmap(reinterpret_cast<void *>(fd * kHostPageSize), num_pages,
                 device_virtual_address);
}

StatusOr<void *> FakeMmuMapper::TranslateDeviceAddress(
    uint64 device_virtual_address) const {
  uint64 aligned_device_addr = GetPageAddress(device_virtual_address);

  StdMutexLock lock(&mutex_);
  auto iter = device_to_host_.find(aligned_device_addr);
  if (iter == device_to_host_.end()) {
    return NotFoundError("Device address not mapped.");
  }

  uint8 *aligned_host_addr = const_cast<uint8 *>(iter->second);
  auto host_address = aligned_host_addr + GetPageOffset(device_virtual_address);

  CHECK(host_address != nullptr);  // StatusOr doesn't like nullptr!
  return host_address;
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
