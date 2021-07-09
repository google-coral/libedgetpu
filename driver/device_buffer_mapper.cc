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

#include "driver/device_buffer_mapper.h"

#include <algorithm>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "api/buffer.h"
#include "driver/hardware_structures.h"
#include "driver/memory/address_utilities.h"
#include "driver/memory/dma_direction.h"
#include "port/cleanup.h"
#include "port/logging.h"
#include "port/status_macros.h"
#include "port/stringprintf.h"
#include "port/tracing.h"

namespace platforms {
namespace darwinn {
namespace driver {

DeviceBufferMapper::DeviceBufferMapper(AddressSpace* address_space)
    : address_space_(address_space) {
  CHECK(address_space != nullptr);
}

Status DeviceBufferMapper::UnmapAll() {
  TRACE_SCOPE("DeviceBufferMapper::UnmapAll");

  RETURN_IF_ERROR(UnmapMultiple(instruction_mappings_));
  RETURN_IF_ERROR(Unmap(std::move(scratch_)));
  RETURN_IF_ERROR(UnmapMultiple(input_mappings_));
  RETURN_IF_ERROR(UnmapMultiple(output_mappings_));

  inputs_.clear();
  input_mappings_.clear();
  outputs_.clear();
  output_mappings_.clear();
  instructions_.clear();
  instruction_mappings_.clear();
  return Status();  // OK
}

Status DeviceBufferMapper::MapInputs(const Buffer::NamedMap& buffers) {
  TRACE_SCOPE("DeviceBufferMapper::MapInputs");
  return MapMultiple(buffers, DmaDirection::kToDevice, inputs_,
                     input_mappings_);
}

Status DeviceBufferMapper::MapOutputs(const Buffer::NamedMap& buffers) {
  TRACE_SCOPE("DeviceBufferMapper::MapOutputs");
  return MapMultiple(buffers, DmaDirection::kFromDevice, outputs_,
                     output_mappings_);
}

Status DeviceBufferMapper::MapScratch(const Buffer& buffer) {
  TRACE_SCOPE("DeviceBufferMapper::MapScratch");
  DCHECK(!scratch_.IsValid());
  ASSIGN_OR_RETURN(scratch_, Map(buffer, DmaDirection::kBidirectional));

  VLOG(3) << StringPrintf(
      "Mapped scratch : %s -> 0x%016llx, %zu bytes.", buffer.ToString().c_str(),
      static_cast<unsigned long long>(  // NOLINT(runtime/int)
          scratch_.device_address()),
      scratch_.size_bytes());

  return Status();  // OK
}

Status DeviceBufferMapper::MapInstructions(const std::vector<Buffer>& buffers) {
  TRACE_SCOPE("DeviceBufferMapper::MapInstructions");
  if (!instruction_mappings_.empty()) {
    return InvalidArgumentError("Instructions are already mapped.");
  }

  static const std::string kInstructions = "instructions";

  // For convenience, place the instructions in a NamedMap just like inputs or
  // outputs.
  Buffer::NamedMap map;
  map[kInstructions] = buffers;

  DeviceBuffer::NamedMap device_map;
  const Status ret = MapMultiple(map, DmaDirection::kToDevice, device_map,
                                 instruction_mappings_);
  instructions_ = std::move(device_map[kInstructions]);
  return ret;
}

StatusOr<DeviceBuffer> DeviceBufferMapper::Map(const Buffer& buffer,
                                               DmaDirection direction) {
  TRACE_SCOPE("DeviceBufferMapper::Map");
  if (buffer.IsValid()) {
    return address_space_->MapMemory(buffer, direction, MappingTypeHint::kAny);
  }
  return DeviceBuffer();  // Invalid buffer.
}

Status DeviceBufferMapper::Unmap(DeviceBuffer buffer) {
  TRACE_SCOPE("DeviceBufferMapper::Unmap");
  if (buffer.IsValid()) {
    return address_space_->UnmapMemory(std::move(buffer));
  }
  return Status();  // OK
}

Status DeviceBufferMapper::MapMultiple(
    const Buffer::NamedMap& buffers, DmaDirection direction,
    /*out*/ DeviceBuffer::NamedMap& user_buffers,
    /*out*/ std::vector<DeviceBuffer>& mapped_buffers) {
  if (!user_buffers.empty() || !mapped_buffers.empty()) {
    return InvalidArgumentError("Device buffer is already mapped.");
  }

  auto cleaner = MakeCleanup(
      [this, &mapped_buffers] { CHECK_OK(UnmapMultiple(mapped_buffers)); });

  // Separate the buffers into ptr- and non-ptr types.
  std::vector<Buffer> ptr_buffers;
  for (const auto& name_and_buffer : buffers) {
    for (const auto& buffer : name_and_buffer.second) {
      if (buffer.IsPtrType()) {
        ptr_buffers.push_back(buffer);
      }
    }
  }

  // Coalesce adjacent buffers. Since the underlying implementation can only map
  // whole pages, any buffers on the same page or adjacent pages can be merged
  // into a single underlying Map call. The basic algorithm is as follows:
  //
  // 1. Create a vector containing all start and end points, keeping a tag
  //    on each element indicating whether it was a start or end.
  // 2. Sort the vector, and if a start and end point have the same address, the
  //    start point should be first in sorted order.
  // 3. Iterate over the vector. Keep a running count of #start-#end points
  //    seen. Whenever this counter hits zero, that's the end of a merged
  //    interval.
  //
  // Because all the addresses are page-aligned, we can use the low bit to
  // distinguish between the start and end points.

  constexpr uint64 kEndOfMappingBit = 1;

  std::vector<uint64> addresses;
  addresses.reserve(ptr_buffers.size() * 2);

  // merged_intervals contains the start address of each merged interval.
  // Pre-allocate space assuming that no merging will happen.
  std::vector<uint8*> merged_intervals;
  merged_intervals.reserve(ptr_buffers.size());

  for (const auto& buffer : ptr_buffers) {
    uint64 start = GetPageAddress(reinterpret_cast<uintptr_t>(buffer.ptr()));
    uint64 end =
        start +
        GetNumberPages(buffer.ptr(), buffer.size_bytes()) * kHostPageSize +
        kEndOfMappingBit;
    addresses.push_back(start);
    addresses.push_back(end);
  }

  std::sort(addresses.begin(), addresses.end());

  int count = 0;
  for (uint64 address : addresses) {
    if (address & kEndOfMappingBit) {
      --count;
      CHECK_GE(count, 0);
      if (count == 0) {
        uint8* start = merged_intervals.back();
        uint8* end = reinterpret_cast<uint8*>(address - kEndOfMappingBit);
        Buffer merged_buffer(start, end - start);
        ASSIGN_OR_RETURN(auto device_buffer, Map(merged_buffer, direction));
        mapped_buffers.push_back(device_buffer);
      }
    } else {
      if (count == 0) {
        merged_intervals.push_back(reinterpret_cast<uint8*>(address));
      }
      ++count;
    }
  }

  // Figure out where the user's device buffers are within the merged buffers.
  for (const auto& name_and_buffer : buffers) {
    for (const auto& buffer : name_and_buffer.second) {
      DeviceBuffer device_buffer;
      if (buffer.IsPtrType()) {
        // Find the index of the corresponding merged buffer. In C++, there is
        // no way to directly binary search for an element that's less than a
        // given value, so instead we look for the closest one that's strictly
        // greater and subtract one from the index.
        const auto next = std::upper_bound(
            merged_intervals.begin(), merged_intervals.end(), buffer.ptr());
        int index = next - merged_intervals.begin() - 1;
        const auto merged = reinterpret_cast<uint8*>(merged_intervals[index]);
        const auto& mapped = mapped_buffers[index];
        device_buffer =
            DeviceBuffer(mapped.device_address() +
                         static_cast<uint64>(buffer.ptr() - merged),
                         buffer.size_bytes());
      } else {
        ASSIGN_OR_RETURN(device_buffer, Map(buffer, direction));
        mapped_buffers.push_back(device_buffer);
      }

      VLOG(3) << StringPrintf(
          "Mapped \"%s\" : %s -> 0x%016llx, %zu bytes. Direction=%d",
          name_and_buffer.first.c_str(), buffer.ToString().c_str(),
          static_cast<unsigned long long>(  // NOLINT(runtime/int)
              device_buffer.device_address()),
          device_buffer.size_bytes(),
          static_cast<std::underlying_type<DmaDirection>::type>(direction));

      user_buffers[name_and_buffer.first].push_back(std::move(device_buffer));
    }
  }

  cleaner.release();
  return OkStatus();
}

Status DeviceBufferMapper::UnmapMultiple(
    std::vector<DeviceBuffer>& device_buffers) {
  Status status;
  for (auto& device_buffer : device_buffers) {
    status.Update(Unmap(std::move(device_buffer)));
  }
  return status;
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
