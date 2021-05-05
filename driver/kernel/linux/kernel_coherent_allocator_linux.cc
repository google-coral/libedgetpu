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

#include "driver/kernel/linux/kernel_coherent_allocator_linux.h"

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "driver/kernel/gasket_ioctl.h"
#include "port/errors.h"
#include "port/logging.h"
#include "port/statusor.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

KernelCoherentAllocatorLinux::KernelCoherentAllocatorLinux(
    const std::string &device_path, int alignment_bytes, size_t size_bytes)
    : KernelCoherentAllocator(device_path, alignment_bytes, size_bytes) {}

StatusOr<char *> KernelCoherentAllocatorLinux::Map(FileDescriptor fd,
                                                   size_t size_bytes,
                                                   uint64 dma_address) {
  // Map the memory range so as it can be accessed by user.
  char *mem_base =
      static_cast<char *>(mmap(nullptr, size_bytes, PROT_READ | PROT_WRITE,
                               MAP_SHARED | MAP_LOCKED, fd, dma_address));
  if (mem_base == MAP_FAILED) {
    return FailedPreconditionError(
        StringPrintf("CoherentAllocator Could not mmap size %zu.", size_bytes));
  }

  return mem_base;
}

Status KernelCoherentAllocatorLinux::Unmap(FileDescriptor fd, char *mem_base,
                                           size_t size_bytes) {
  if (munmap(mem_base, size_bytes)) {
    return FailedPreconditionError(
        StringPrintf("Error unmapping coherent memory. %s", strerror(errno)));
  }
  return Status();  // OK
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
