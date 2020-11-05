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

#include "driver/kernel/windows/kernel_coherent_allocator_windows.h"

#include "driver/kernel/gasket_ioctl.h"
#include "port/errors.h"
#include "port/logging.h"
#include "port/statusor.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

KernelCoherentAllocatorWindows::KernelCoherentAllocatorWindows(
    const std::string &device_path, int alignment_bytes, size_t size_bytes)
    : KernelCoherentAllocator(device_path, alignment_bytes, size_bytes) {}

util::StatusOr<char *> KernelCoherentAllocatorWindows::Map(FileDescriptor fd,
                                                           size_t size_bytes,
                                                           uint64 dma_address) {
  gasket_address_map_ioctl apex_memmap_ioctl;
  memset(&apex_memmap_ioctl, 0x00, sizeof(apex_memmap_ioctl));
  apex_memmap_ioctl.dev_dma_addr = dma_address;
  apex_memmap_ioctl.size = size_bytes;
  int rc = ioctl(fd, GASKET_IOCTL_MAP_UMDMA_VIEW, &apex_memmap_ioctl);
  if (rc != 0) {
    return util::FailedPreconditionError(
        StringPrintf("CoherentAllocator Could not map size %zu.", size_bytes));
  }
  return (char *)apex_memmap_ioctl.virtaddr;
}

util::Status KernelCoherentAllocatorWindows::Unmap(FileDescriptor fd,
                                                   char *mem_base,
                                                   size_t size_bytes) {
  gasket_address_map_ioctl apex_memmap_ioctl;
  memset(&apex_memmap_ioctl, 0x00, sizeof(apex_memmap_ioctl));
  apex_memmap_ioctl.virtaddr = reinterpret_cast<uint64_t *>(mem_base);
  int rc = ioctl(fd, GASKET_IOCTL_UNMAP_UMDMA_VIEW, &apex_memmap_ioctl);
  if (rc != 0) {
    return util::FailedPreconditionError(StringPrintf(
        "CoherentAllocator Could not unmap coherent %p.", mem_base));
  }
  return util::Status();  // OK
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
