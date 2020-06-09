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

#ifndef DARWINN_DRIVER_INSTRUCTION_BUFFERS_H_
#define DARWINN_DRIVER_INSTRUCTION_BUFFERS_H_

#include <memory>
#include <vector>

#include "api/buffer.h"
#include "driver/allocator.h"
#include "driver/device_buffer_mapper.h"
#include "executable/executable_generated.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Wrapper class for handling instruction buffers.
class InstructionBuffers {
 public:
  // Constructs the instruction buffers by allocating and copying instruction
  // stream to host memory.
  InstructionBuffers(
      platforms::darwinn::driver::Allocator *allocator,
      const flatbuffers::Vector<flatbuffers::Offset<InstructionBitstream>>
          &instruction_bitstreams);
  ~InstructionBuffers();

  // Links scratch address, parameters, input, and output.
  void LinkInstructionBuffers(
      const DeviceBuffer &parameter_device_buffer,
      DeviceBufferMapper *device_buffer_mapper,
      const flatbuffers::Vector<flatbuffers::Offset<InstructionBitstream>>
          &instruction_bitstreams);

  // Returns the reference to the buffer vector.
  const std::vector<Buffer> &GetBuffers() const { return buffers_; }

 private:
  // The actual buffers which holds the instruction stream.
  std::vector<Buffer> buffers_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_INSTRUCTION_BUFFERS_H_
