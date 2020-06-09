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

#include "driver/kernel/kernel_registers.h"

#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

#include "absl/strings/str_format.h"
#include "driver/registers/registers.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/statusor.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

KernelRegisters::KernelRegisters(const std::string& device_path,
                                 const std::vector<MmapRegion>& mmap_region,
                                 bool read_only)
    : device_path_(device_path), read_only_(read_only) {
  for (const auto& region : mmap_region) {
    mmap_region_.push_back({region.offset, region.size, nullptr});
  }
}

KernelRegisters::KernelRegisters(const std::string& device_path,
                                 uint64 mmap_offset, uint64 mmap_size,
                                 bool read_only)
    : KernelRegisters(device_path, {{mmap_offset, mmap_size}}, read_only) {}

KernelRegisters::~KernelRegisters() {
  for (auto& region : mmap_region_) {
    if (region.registers != nullptr) {
      const int ret = munmap(region.registers, region.size);
      if (ret != 0) {
        LOG(ERROR) << "Error unmapping registers: " << strerror(errno);
      }
      region.registers = nullptr;
    }
  }

  if (fd_ != -1) {
    LOG(WARNING)
        << "Destroying KernelRegisters - Close() had not yet been called!";
    util::Status status = Close();
    if (!status.ok()) {
      LOG(ERROR) << status;
    }
  }
}

util::Status KernelRegisters::Open() {
  StdMutexLock lock(&mutex_);
  if (fd_ != -1) {
    return util::FailedPreconditionError("Device already open.");
  }

  VLOG(1) << StringPrintf("Opening %s. read_only=%d", device_path_.c_str(),
                          read_only_);
  int mode = O_RDWR;
  if (read_only_) {
    mode = O_RDONLY;
  }

  fd_ = open(device_path_.c_str(), mode);
  if (fd_ < 0) {
    return util::FailedPreconditionError(
        StringPrintf("Device open failed : %d (%s)", fd_, strerror(errno)));
  }

  int protections = PROT_READ | PROT_WRITE;
  if (read_only_) {
    protections = PROT_READ;
  }

  for (auto& region : mmap_region_) {
    VLOG(1) << StringPrintf("mmap_offset=0x%016llx, mmap_size=%lld",
                            static_cast<long long>(region.offset),
                            static_cast<long long>(region.size));

    region.registers =
        static_cast<uint64*>(mmap(nullptr, region.size, protections,
                                  MAP_SHARED | MAP_LOCKED, fd_, region.offset));
    if (region.registers == MAP_FAILED) {
      close(fd_);
      fd_ = -1;
      region.registers = nullptr;

      return util::FailedPreconditionError(
          StringPrintf("Could not mmap: %s.", strerror(errno)));
    }
    VLOG(3) << "Got map addr at 0x" << std::hex << region.registers;
  }

  return util::Status();  // OK
}

util::Status KernelRegisters::Close() {
  StdMutexLock lock(&mutex_);
  if (fd_ == -1) {
    return util::FailedPreconditionError("Device not open.");
  }

  for (auto& region : mmap_region_) {
    if (region.registers != nullptr) {
      VLOG(1) << StringPrintf(
          "Closing %s. mmap_offset=0x%016llx, mmap_size=%lld, read_only=%d",
          device_path_.c_str(),
          static_cast<long long>(region.offset),  // NOLINT(runtime/int)
          static_cast<long long>(region.size),    // NOLINT(runtime/int)
          read_only_);
      const int ret = munmap(region.registers, region.size);
      if (ret != 0) {
        LOG(ERROR) << "Error unmapping registers: " << strerror(errno);
      }
      region.registers = nullptr;
    }
  }

  close(fd_);
  fd_ = -1;

  return util::Status();  // OK
}

util::StatusOr<uint8*> KernelRegisters::LockAndGetMappedOffset(
    uint64 offset, int alignment) const {
  StdMutexLock lock(&mutex_);
  return GetMappedOffset(offset, alignment);
}

inline util::StatusOr<uint8*> KernelRegisters::GetMappedOffset(
    uint64 offset, int alignment) const {
  const size_t end_of_region = offset + alignment;

  if (end_of_region < offset) {
    return util::OutOfRangeError(
        StringPrintf("Offset (0x%016llx) + size_bytes is larger than 64-bit",
                     static_cast<long long>(offset)));
  }

  for (const auto& region : mmap_region_) {
    if ((offset >= region.offset) &&
        ((end_of_region - region.offset) <= region.size)) {
      if (region.registers != nullptr) {
        return reinterpret_cast<uint8*>(region.registers) + offset -
               region.offset;
      } else {
        return util::InternalError("Region not mapped yet");
      }
    }
  }

  return util::OutOfRangeError(absl::StrFormat(
      "Offset (0x%016llx) is not covered by any region", offset));
}

util::Status KernelRegisters::Write(uint64 offset, uint64 value) {
  StdMutexLock lock(&mutex_);
  if (fd_ == -1) {
    return util::FailedPreconditionError("Device not open.");
  }
  if (read_only_) {
    return util::FailedPreconditionError("Read only, cannot write.");
  }
  if (offset % sizeof(uint64) != 0) {
    return util::FailedPreconditionError(StringPrintf(
        "Offset (0x%016llx) not aligned to 8B",
        static_cast<unsigned long long>(offset)));  // NOLINT(runtime/int)
  }

  ASSIGN_OR_RETURN(auto mmap_register, GetMappedOffset(offset, sizeof(uint64)));
  *reinterpret_cast<uint64*>(mmap_register) = value;
  VLOG(5) << StringPrintf(
      "Write: offset = 0x%016llx, value = 0x%016llx",
      static_cast<unsigned long long>(offset),  // NOLINT(runtime/int)
      static_cast<unsigned long long>(value));  // NOLINT(runtime/int)

  return util::Status();  // OK
}

util::StatusOr<uint64> KernelRegisters::Read(uint64 offset) {
  StdMutexLock lock(&mutex_);
  if (fd_ == -1) {
    return util::FailedPreconditionError("Device not open.");
  }
  if (offset % sizeof(uint64) != 0) {
    return util::FailedPreconditionError(StringPrintf(
        "Offset (0x%016llx) not aligned to 8B",
        static_cast<unsigned long long>(offset)));  // NOLINT(runtime/int)
  }

  ASSIGN_OR_RETURN(auto mmap_register, GetMappedOffset(offset, sizeof(uint64)));
  uint64 value = *reinterpret_cast<uint64*>(mmap_register);
  VLOG(5) << StringPrintf(
      "Read: offset = 0x%016llx, value: = 0x%016llx",
      static_cast<unsigned long long>(offset),  // NOLINT(runtime/int)
      static_cast<unsigned long long>(value));  // NOLINT(runtime/int)

  return value;
}

util::Status KernelRegisters::Write32(uint64 offset, uint32 value) {
  StdMutexLock lock(&mutex_);
  if (fd_ == -1) {
    return util::FailedPreconditionError("Device not open.");
  }
  if (read_only_) {
    return util::FailedPreconditionError("Read only, cannot write.");
  }
  if (offset % sizeof(uint32) != 0) {
    return util::FailedPreconditionError(StringPrintf(
        "Offset (0x%016llx) not aligned to 8B",
        static_cast<unsigned long long>(offset)));  // NOLINT(runtime/int)
  }

  ASSIGN_OR_RETURN(auto mmap_register, GetMappedOffset(offset, sizeof(uint32)));
  *reinterpret_cast<uint32*>(mmap_register) = value;
  VLOG(5) << StringPrintf(
      "Write: offset = 0x%016llx, value = 0x%08x",
      static_cast<unsigned long long>(offset),  // NOLINT(runtime/int)
      value);

  return util::Status();  // OK
}

util::StatusOr<uint32> KernelRegisters::Read32(uint64 offset) {
  StdMutexLock lock(&mutex_);
  if (fd_ == -1) {
    return util::FailedPreconditionError("Device not open.");
  }
  if (offset % sizeof(uint32) != 0) {
    return util::FailedPreconditionError(StringPrintf(
        "Offset (0x%016llx) not aligned to 8B",
        static_cast<unsigned long long>(offset)));  // NOLINT(runtime/int)
  }

  ASSIGN_OR_RETURN(auto mmap_register, GetMappedOffset(offset, sizeof(uint32)));
  uint32 value = *reinterpret_cast<uint32*>(mmap_register);
  VLOG(5) << StringPrintf(
      "Read: offset = 0x%016llx, value: = 0x%08x",
      static_cast<unsigned long long>(offset),  // NOLINT(runtime/int)
      value);

  return value;
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
