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

#ifndef DARWINN_DRIVER_MEMORY_MMU_MAPPER_H_
#define DARWINN_DRIVER_MEMORY_MMU_MAPPER_H_

#include "api/buffer.h"
#include "driver/device_buffer.h"
#include "driver/memory/dma_direction.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Abstract class for mapping memory on device MMU.
class MmuMapper {
 public:
  virtual ~MmuMapper() = default;

  // Opens / Closes the MMU interface.
  // - Reserve |num_simple_page_table_entries_requested| page table entries for
  // simple indexing. Remaining entries will be used for extended addressing.
  virtual Status Open(int num_simple_page_table_entries_requested) = 0;
  virtual Status Close() = 0;

  // Maps |num_pages| from the memory backing |buffer| to
  // |device_virtual_address|.
  Status Map(const Buffer &buffer, uint64 device_virtual_address) {
    return Map(buffer, device_virtual_address, DmaDirection::kBidirectional);
  }

  // Same as above but with a hint indicating the buffer transfer direction.
  Status Map(const Buffer &buffer, uint64 device_virtual_address,
             DmaDirection direction);

  // Unmaps previously mapped Buffer.
  Status Unmap(const Buffer &buffer, uint64 device_virtual_address);

  // Translates device address to host virtual address. This function is
  // typically not implemented and will return an UNIMPLEMENTED Status. It is
  // only useful when MMU needs to be modeled directly (as is the case when
  // using IpCore without the HIB, or with no MMU).
  //
  // Note that the device address here is the address that is output by the
  // hardware, which may be physical or virtual, depending if an MMU is present
  // or not.
  virtual StatusOr<void *> TranslateDeviceAddress(uint64 device_address) const {
    return UnimplementedError("Translate not supported.");
  }

  // Determines if a virtual address (obtained from TranslateDeviceAddress)
  // points into the extended page tables of this MMU. If so, reads
  // "size_in_bytes" bytes of data from "address" to "buffer" and returns true.
  // Generally this is false, except in certain simulations where the MMU is
  // modeled directly.
  virtual bool TryReadExtendedPageTable(const void *address, void *buffer,
                                        int size_in_bytes) const {
    return false;
  }

 protected:
  // Maps |num_pages| from |buffer| (the host virtual address) to
  // |device_virtual_address|. All addresses must be page aligned.
  // Called by public version of Map when the buffer is backed by host memory.
  virtual Status DoMap(const void *buffer, int num_pages,
                       uint64 device_virtual_address,
                       DmaDirection direction) = 0;

  // Maps file descriptor to |device_virtual_address|. Default = unimplemented.
  virtual Status DoMap(int fd, int num_pages, uint64 device_virtual_address,
                       DmaDirection direction) {
    return UnimplementedError("File descriptor-backed mapping not supported.");
  }

  // Unmaps previously mapped addresses.
  // Called by public version of Unmap when the buffer is backed by host memory.
  virtual Status DoUnmap(const void *buffer, int num_pages,
                         uint64 device_virtual_address) = 0;

  // Unmaps previously mapped file descriptor based buffer. Default =
  // unimplemented.

  virtual Status DoUnmap(int fd, int num_pages, uint64 device_virtual_address) {
    return UnimplementedError(
        "File descriptor-backed unmapping not supported.");
  }
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_MEMORY_MMU_MAPPER_H_
