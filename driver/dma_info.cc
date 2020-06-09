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

#include "driver/dma_info.h"

#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace {

// Returns a debugging-friendly string.
std::string ToString(DmaState state) {
  switch (state) {
    case DmaState::kPending:
      return "pending";

    case DmaState::kActive:
      return "active";

    case DmaState::kCompleted:
      return "completed";

    case DmaState::kError:
      return "error";
  }
}

// Returns a debugging-friendly string.
std::string ToString(const DeviceBuffer& buffer) {
  return StringPrintf("device_address = 0x%llx, bytes = %zd",
                      static_cast<unsigned long long>(buffer.device_address()),
                      buffer.size_bytes());
}

}  // namespace

std::string DmaInfo::Dump() const {
  std::string prefix = StringPrintf("DMA[%d]: ", id_);
  switch (type_) {
    case DmaDescriptorType::kInstruction:
      return prefix + "Instruction: " + ToString(buffer_) + ", " +
             ToString(state_);
    case DmaDescriptorType::kInputActivation:
      return prefix + "Input activation: " + ToString(buffer_) + ", " +
             ToString(state_);
    case DmaDescriptorType::kParameter:
      return prefix + "Parameter: " + ToString(buffer_) + ", " +
             ToString(state_);
    case DmaDescriptorType::kOutputActivation:
      return prefix + "Output activation: " + ToString(buffer_) + ", " +
             ToString(state_);
    case DmaDescriptorType::kScalarCoreInterrupt0:
      return prefix + "SC interrupt 0";
    case DmaDescriptorType::kScalarCoreInterrupt1:
      return prefix + "SC interrupt 1";
    case DmaDescriptorType::kScalarCoreInterrupt2:
      return prefix + "SC interrupt 2";
    case DmaDescriptorType::kScalarCoreInterrupt3:
      return prefix + "SC interrupt 3";
    case DmaDescriptorType::kLocalFence:
      return prefix + "Local fence";
    case DmaDescriptorType::kGlobalFence:
      return prefix + "Global fence";
  }
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
