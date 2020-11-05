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

#ifndef DARWINN_DRIVER_KERNEL_KERNEL_REGISTERS_H_
#define DARWINN_DRIVER_KERNEL_KERNEL_REGISTERS_H_

#include <mutex>  // NOLINT
#include <string>
#include <vector>

#include "driver/registers/registers.h"
#include "port/fileio.h"
#include "port/integral_types.h"
#include "port/status.h"
#include "port/statusor.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Kernel implementation of the register interface.
class KernelRegisters : public Registers {
 public:
  struct MmapRegion {
    uint64 offset;
    uint64 size;
  };

  KernelRegisters(const std::string& device_path,
                  const std::vector<MmapRegion>& mmap_region, bool read_only);

  KernelRegisters(const std::string& device_path, uint64 mmap_offset,
                  uint64 mmap_size, bool read_only);

  ~KernelRegisters() override;

  // Overrides from registers.h
  util::Status Open() LOCKS_EXCLUDED(mutex_) override;
  util::Status Close() LOCKS_EXCLUDED(mutex_) override;
  util::Status Write(uint64 offset, uint64 value)
      LOCKS_EXCLUDED(mutex_) override;
  util::StatusOr<uint64> Read(uint64 offset) LOCKS_EXCLUDED(mutex_) override;
  util::Status Write32(uint64 offset, uint32 value)
      LOCKS_EXCLUDED(mutex_) override;
  util::StatusOr<uint32> Read32(uint64 offset) LOCKS_EXCLUDED(mutex_) override;

 protected:
  struct MappedRegisterRegion {
    uint64 offset;
    uint64 size;
    uint64* registers;
  };

  // Acquires the lock and maps CSR offset.
  util::StatusOr<uint8*> LockAndGetMappedOffset(uint64 offset,
                                                int alignment) const
      LOCKS_EXCLUDED(mutex_);

  // Returns the reference to the mapped regions.
  std::vector<MappedRegisterRegion>& GetMmapRegion() { return mmap_region_; }

  // Unmaps all device BARs ranges previously mapped to user mode space VAs.
  void UnmapAllRegions() EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Maps and returns user mode space VA for device BARs range described by
  // region.offset and region.size.
  virtual util::StatusOr<uint64*> MapRegion(FileDescriptor fd,
                                            const MappedRegisterRegion& region,
                                            bool read_only) = 0;

  // Unmaps device BARs range described by region.offset and region.size
  // which was previously mapped to region.registers user mode space VA.
  virtual util::Status UnmapRegion(FileDescriptor fd,
                                   const MappedRegisterRegion& region) = 0;

 private:
  // Maps CSR offset to virtual address without acquiring the lock.
  util::StatusOr<uint8*> GetMappedOffset(uint64 offset, int alignment) const
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Device path.
  const std::string device_path_;

  // mmap() region.
  std::vector<MappedRegisterRegion> mmap_region_ GUARDED_BY(mutex_);

  // true, if read only. false otherwise.
  const bool read_only_;

  // File descriptor of the opened device.
  FileDescriptor fd_ GUARDED_BY(mutex_){INVALID_FD_VALUE};

  // Mutex that guards fd_;
  mutable std::mutex mutex_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_KERNEL_KERNEL_REGISTERS_H_
