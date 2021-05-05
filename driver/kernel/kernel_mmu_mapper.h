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

#ifndef DARWINN_DRIVER_KERNEL_KERNEL_MMU_MAPPER_H_
#define DARWINN_DRIVER_KERNEL_KERNEL_MMU_MAPPER_H_

#include <mutex>  // NOLINT

#ifndef _WIN32
#include <sys/ioctl.h>
#endif

#include "driver/memory/dma_direction.h"
#include "driver/memory/mmu_mapper.h"
#include "port/fileio.h"
#include "port/integral_types.h"
#include "port/status.h"
#include "port/statusor.h"
#include "port/std_mutex_lock.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Kernel implementation of the MMU mapper interface.
class KernelMmuMapper : public MmuMapper {
 public:
  explicit KernelMmuMapper(const std::string &device_path);
  ~KernelMmuMapper() override = default;

  // Overrides from mmu_mapper.h
  Status Open(int num_simple_page_table_entries_requested) override;
  Status Close() override;

 protected:
  Status DoMap(const void *buffer, int num_pages, uint64 device_virtual_address,
               DmaDirection direction) override;
  Status DoUnmap(const void *buffer, int num_pages,
                 uint64 device_virtual_address) override;
  Status DoMap(int fd, int num_pages, uint64 device_virtual_address,
               DmaDirection direction) override;
  Status DoUnmap(int fd, int num_pages, uint64 device_virtual_address) override;

  // Calls ioctl on the device file descriptor owned by this instance.
  // Forwards the parameters to the ioctl too; returns -1 on closed device and
  // the return value of ioctl otherwise.
  template <typename... Params>
  int DoIoctl(Params &&... params) {
    // TODO : At this moment this mutex is there to guard uses of fd,
    // but if later we find there's a lot of concurrent threaded map/unmap
    // activity we could consider trying to allow them to run in parallel. If
    // this macro is to be called on ioctls that aren't necessarily
    // mutually-exclusive / can run in parallel (at the runtime level)
    // then may want to make appropriate locking the caller's responsibility.

    StdMutexLock lock(&mutex_);
    if (fd_ != INVALID_FD_VALUE) {
      return ioctl(fd_, std::forward<Params>(params)...);
    } else {
      VLOG(4) << "Invalid file descriptor.";
      return INVALID_FD_VALUE;
    }
  }

 private:
  // Device path.
  const std::string device_path_;

  // File descriptor of the opened device.
  FileDescriptor fd_ GUARDED_BY(mutex_){INVALID_FD_VALUE};

  // Mutex that guards fd_;
  mutable std::mutex mutex_;

  // Indicates whether the kernel driver supports GASKET_IOCTL_MAP_BUFFER_FLAGS.
  bool map_flags_supported_ GUARDED_BY(mutex_){true};
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_KERNEL_KERNEL_MMU_MAPPER_H_
