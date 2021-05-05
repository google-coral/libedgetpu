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

#ifndef DARWINN_DRIVER_DEVICE_BUFFER_MAPPER_H_
#define DARWINN_DRIVER_DEVICE_BUFFER_MAPPER_H_

#include <functional>
#include <vector>

#include "api/buffer.h"
#include "driver/device_buffer.h"
#include "driver/memory/address_space.h"
#include "driver/memory/dma_direction.h"
#include "port/logging.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Thread-unsafe.
// Maps request-specific Buffers to DeviceBuffers, and keeps track of
// DeviceBuffers. These include: input, output, instruction and scratch.
// Note that parameters are mapped and owned by ExecutableReference.
class DeviceBufferMapper {
 public:
  explicit DeviceBufferMapper(AddressSpace* address_space);
  ~DeviceBufferMapper() = default;

  // This class is neither copyable nor movable.
  DeviceBufferMapper(const DeviceBufferMapper&) = delete;
  DeviceBufferMapper& operator=(const DeviceBufferMapper&) = delete;

  // Unmaps all per-request buffers. It is safe to call this method for cleanup
  // even if DeviceBuffers are partially mapped.
  Status UnmapAll();

  // Maps given buffers to DeviceBuffers.
  Status MapInputs(const Buffer::NamedMap& buffers);
  Status MapOutputs(const Buffer::NamedMap& buffers);
  Status MapScratch(const Buffer& buffer);
  Status MapInstructions(const std::vector<Buffer>& buffers);

  // Returns mapped DeviceBuffers.
  const DeviceBuffer::NamedMap& GetInputDeviceBuffers() const {
    return inputs_;
  }
  const DeviceBuffer::NamedMap& GetOutputDeviceBuffers() const {
    return outputs_;
  }
  const DeviceBuffer& GetScratchDeviceBuffer() const { return scratch_; }
  const std::vector<DeviceBuffer>& GetInstructionDeviceBuffers() const {
    return instructions_;
  }

  // Returns mapped DeviceBuffer for given argument.
  const DeviceBuffer& GetInputDeviceBuffer(const std::string& name,
                                           int batch) const {
    return inputs_.at(name)[batch];
  }
  const DeviceBuffer& GetOutputDeviceBuffer(const std::string& name,
                                            int batch) const {
    return outputs_.at(name)[batch];
  }
  const DeviceBuffer& GetInstructionDeviceBuffer(int chunk_id) const {
    DCHECK_LT(chunk_id, instructions_.size());
    return instructions_[chunk_id];
  }

 private:
  // Convenience function that wraps AddressSpace#Map() handling invalid
  // buffers.
  StatusOr<DeviceBuffer> Map(const Buffer& buffer, DmaDirection direction);

  // Convenience function that wraps AddressSpace#UnmapMemory() handling invalid
  // buffers.
  Status Unmap(DeviceBuffer buffer);

  // Helper function to map multiple buffers, merging adjacent buffers.
  // - Fills user_buffers with a map of device buffers that directly correspond
  // to the passed in buffers. Data parallel elements are represented as
  // separate entries, even if the memory is contiguous. These device buffers
  // are suitable for use in the instruction linking process.
  // - Fills mapped_buffers with the merged list of device buffers that actually
  // got mapped. These are the device buffers that need to be unmapped later.
  Status MapMultiple(const Buffer::NamedMap& buffers, DmaDirection direction,
                     /*out*/ DeviceBuffer::NamedMap& user_buffers,
                     /*out*/ std::vector<DeviceBuffer>& mapped_buffers);

  // Helper function to unmap multiple buffers. All passed in buffers will be
  // invalidated by this call.
  Status UnmapMultiple(std::vector<DeviceBuffer>& device_buffers);

  // Address space used for mapping.
  AddressSpace* const address_space_;

  // Scratch buffer. Could be invalid.
  DeviceBuffer scratch_;

  // Input/output buffers.
  // input/output[layer_name][batch_id] = DeviceBuffer
  DeviceBuffer::NamedMap inputs_;
  DeviceBuffer::NamedMap outputs_;

  // Actual mappings that were created, after coalescing adjacent buffers. These
  // are the mappings that need to be unmapped at the end of the request.
  std::vector<DeviceBuffer> input_mappings_;
  std::vector<DeviceBuffer> output_mappings_;

  // Instruction buffers.
  std::vector<DeviceBuffer> instructions_;

  // Actual mappings that were created for instructions, after coalescing
  // adjacent buffers.
  std::vector<DeviceBuffer> instruction_mappings_;
};

// Holds a mapped device buffer as well as a callback for unmapping.
class MappedDeviceBuffer {
 public:
  MappedDeviceBuffer() = default;
  MappedDeviceBuffer(const DeviceBuffer& device_buffer,
                     const std::function<Status(const DeviceBuffer&)>& unmapper)
      : device_buffer_(device_buffer),
        unmap_(std::bind(unmapper, device_buffer)) {}

  ~MappedDeviceBuffer() {
    // We should have unmapped the buffer at this moment.
    CHECK(!unmap_);
  }

  // This type is not copyable; we can't have the same device buffer unmapped
  // more than once.
  MappedDeviceBuffer(const MappedDeviceBuffer&) = delete;
  MappedDeviceBuffer& operator=(const MappedDeviceBuffer&) = delete;

  // This type is movable.
  MappedDeviceBuffer(MappedDeviceBuffer&& other) = default;
  MappedDeviceBuffer& operator=(MappedDeviceBuffer&& other) = default;

  const DeviceBuffer& device_buffer() const { return device_buffer_; }

  // Unmaps the associated DeviceBuffer using the given unmapper.
  Status Unmap() {
    if (unmap_) RETURN_IF_ERROR(unmap_());
    unmap_ = nullptr;
    return Status();  // OK.
  }

 private:
  DeviceBuffer device_buffer_;
  std::function<Status()> unmap_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_DEVICE_BUFFER_MAPPER_H_
