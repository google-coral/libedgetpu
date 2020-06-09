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

#include "driver/memory/mmu_mapper.h"

#include "driver/memory/address_utilities.h"
#include "port/tracing.h"

namespace platforms {
namespace darwinn {
namespace driver {

util::Status MmuMapper::Map(const Buffer &buffer, uint64 device_virtual_address,
                            DmaDirection direction) {
  TRACE_SCOPE("MmuMapper::Map");
  // Buffers backed by file descriptors do not have valid ptr().
  const void *ptr = buffer.FileDescriptorBacked() ? nullptr : buffer.ptr();
  if (buffer.IsPtrType() && ptr == nullptr) {
    return util::InvalidArgumentError("Cannot map a Buffer of nullptr.");
  }

  const size_t size_bytes = buffer.size_bytes();
  if (size_bytes == 0) {
    return util::InvalidArgumentError("Cannot map 0 bytes.");
  }

  auto num_requested_pages = GetNumberPages(ptr, size_bytes);

  // Buffers backed by file descriptors are handled differently.
  if (buffer.FileDescriptorBacked()) {
    return DoMap(buffer.fd(), num_requested_pages, device_virtual_address,
                 direction);
  } else {
    auto aligned_buffer_addr = GetPageAddressForBuffer(ptr);
    return DoMap(aligned_buffer_addr, num_requested_pages,
                 device_virtual_address, direction);
  }
}

util::Status MmuMapper::Unmap(const Buffer &buffer,
                              uint64 device_virtual_address) {
  TRACE_SCOPE("MmuMapper::Unmap");
  // Buffers backed by file descriptors do not have valid ptr().
  const void *ptr = buffer.FileDescriptorBacked() ? nullptr : buffer.ptr();
  if (buffer.IsPtrType() && ptr == nullptr) {
    return util::InvalidArgumentError("Cannot unmap a Buffer of nullptr.");
  }

  const size_t size_bytes = buffer.size_bytes();
  if (size_bytes == 0) {
    return util::InvalidArgumentError("Cannot unmap 0 bytes.");
  }

  auto num_mapped_pages = GetNumberPages(ptr, size_bytes);

  // Buffers backed by file descriptors are handled differently.
  if (buffer.FileDescriptorBacked()) {
    return DoUnmap(buffer.fd(), num_mapped_pages, device_virtual_address);
  } else {
    auto aligned_buffer_addr = GetPageAddressForBuffer(ptr);
    return DoUnmap(aligned_buffer_addr, num_mapped_pages,
                   device_virtual_address);
  }
}


}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
