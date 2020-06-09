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

#include "driver/instruction_buffers.h"

#include <vector>

#include "driver/aligned_allocator.h"
#include "driver/device_buffer_mapper.h"
#include "driver/executable_util.h"
#include "executable/executable_generated.h"
#include "port/logging.h"
#include "port/tracing.h"

namespace platforms {
namespace darwinn {
namespace driver {

using ::flatbuffers::Offset;
using ::flatbuffers::Vector;
using ::flatbuffers::VectorLength;

InstructionBuffers::InstructionBuffers(
    Allocator* const allocator,
    const Vector<Offset<InstructionBitstream>>& instruction_bitstreams) {
  // Allocate and create an aligned copy of instruction bitstream.
  buffers_.reserve(VectorLength(&instruction_bitstreams));

  for (const auto& chunk : instruction_bitstreams) {
    auto buffer = allocator->MakeBuffer(chunk->bitstream()->Length());
    buffers_.push_back(std::move(buffer));
    memcpy(buffers_.back().ptr(), chunk->bitstream()->data(),
           chunk->bitstream()->Length());
  }
  VLOG(10) << "InstructionBuffers created.";
}

InstructionBuffers::~InstructionBuffers() {
  buffers_.clear();
  VLOG(10) << "InstructionBuffers destroyed.";
}

void InstructionBuffers::LinkInstructionBuffers(
    const DeviceBuffer& parameter_device_buffer,
    DeviceBufferMapper* device_buffer_mapper,
    const Vector<Offset<InstructionBitstream>>& instruction_bitstreams) {
  TRACE_SCOPE("InstructionBuffers::LinkInstructionBuffers");

  // Update the instruction stream to link the input, output and parameter
  // addresses.
  for (int i = 0; i < VectorLength(&instruction_bitstreams); ++i) {
    // Link scratch address if necessary.
    // Note: we may be able to optimize this scratch address linking like the
    // parameters below, so that we don't re-link it every time since we have
    // more control on the scratch memory and could keep it at the same address.
    // It's unclear how easy to make this change at this point though, and we
    // could revisit this later if needed (yuchicheng).
    if (device_buffer_mapper->GetScratchDeviceBuffer().IsValid()) {
      ExecutableUtil::LinkScratchAddress(
          device_buffer_mapper->GetScratchDeviceBuffer().device_address(),
          instruction_bitstreams.Get(i)->field_offsets(),
          gtl::MutableArraySlice<uint8>(
              buffers_[i].ptr(),
              VectorLength(instruction_bitstreams.Get(i)->bitstream())));
    }

    // Link parameters if necessary.
    if (parameter_device_buffer.IsValid()) {
      const uint64 linked_parameter_address =
          parameter_device_buffer.device_address();
      ExecutableUtil::LinkParameterAddress(
          linked_parameter_address,
          instruction_bitstreams.Get(i)->field_offsets(),
          gtl::MutableArraySlice<uint8>(
              buffers_[i].ptr(),
              VectorLength(instruction_bitstreams.Get(i)->bitstream())));
    }

    for (const auto& name_and_mapped_input :
         device_buffer_mapper->GetInputDeviceBuffers()) {
      std::vector<uint64> linked_input_addresses;
      for (const auto& mapped_input : name_and_mapped_input.second) {
        linked_input_addresses.push_back(mapped_input.device_address());
      }
      ExecutableUtil::LinkInputAddress(
          name_and_mapped_input.first, linked_input_addresses,
          instruction_bitstreams.Get(i)->field_offsets(),
          gtl::MutableArraySlice<uint8>(
              buffers_[i].ptr(),
              VectorLength(instruction_bitstreams.Get(i)->bitstream())));
    }

    for (const auto& name_and_mapped_output :
         device_buffer_mapper->GetOutputDeviceBuffers()) {
      std::vector<uint64> linked_output_addresses;
      for (const auto& mapped_output : name_and_mapped_output.second) {
        linked_output_addresses.push_back(mapped_output.device_address());
      }
      ExecutableUtil::LinkOutputAddress(
          name_and_mapped_output.first, linked_output_addresses,
          instruction_bitstreams.Get(i)->field_offsets(),
          gtl::MutableArraySlice<uint8>(
              buffers_[i].ptr(),
              VectorLength(instruction_bitstreams.Get(i)->bitstream())));
    }
  }
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
