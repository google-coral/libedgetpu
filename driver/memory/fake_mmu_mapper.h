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

#ifndef DARWINN_DRIVER_MEMORY_FAKE_MMU_MAPPER_H_
#define DARWINN_DRIVER_MEMORY_FAKE_MMU_MAPPER_H_

#include <map>
#include <mutex>

#include "driver/memory/dma_direction.h"
#include "driver/memory/mmu_mapper.h"
#include "port/integral_types.h"
#include "port/statusor.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// A fake MMU mapper implementation that does not accurately model
// the underlying hardware, but behaves the same way.
class FakeMmuMapper : public MmuMapper {
 public:
  FakeMmuMapper() {}
  ~FakeMmuMapper() override {}

  // This class is neither copyable nor movable.
  FakeMmuMapper(const FakeMmuMapper&) = delete;
  FakeMmuMapper& operator=(const FakeMmuMapper&) = delete;

  // Overrides from MmuMapper
  Status Open(int num_simple_page_table_entries_requested) override {
    return Status();  // OK
  }
  Status Close() override { return Status(); }
  StatusOr<void*> TranslateDeviceAddress(
      uint64 device_virtual_address) const override;

 protected:
  Status DoMap(const void* buffer, int num_pages, uint64 device_virtual_address,
               DmaDirection direction) override;
  Status DoUnmap(const void* buffer, int num_pages,
                 uint64 device_virtual_address) override;

  // Fake mapping: assuming physical address = fd * kHostPageSize.
  Status DoMap(int fd, int num_pages, uint64 device_virtual_address,
               DmaDirection direction) override;
  Status DoUnmap(int fd, int num_pages, uint64 device_virtual_address) override;

  // "Page table" to track device addr to host mappings.
  std::map<uint64, const uint8*> device_to_host_ GUARDED_BY(mutex_);

  // Guards device_to_host_.
  mutable std::mutex mutex_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_MEMORY_FAKE_MMU_MAPPER_H_
