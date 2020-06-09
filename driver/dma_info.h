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

#ifndef DARWINN_DRIVER_DMA_INFO_H_
#define DARWINN_DRIVER_DMA_INFO_H_

#include <string>

#include "driver/device_buffer.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Possible DMA descriptor types.
enum class DmaDescriptorType {
  kInstruction = 0,
  kInputActivation = 1,
  kParameter = 2,
  kOutputActivation = 3,
  kScalarCoreInterrupt0 = 4,
  kScalarCoreInterrupt1 = 5,
  kScalarCoreInterrupt2 = 6,
  kScalarCoreInterrupt3 = 7,

  // Fence is not exposed to driver.
  // Used to synchronize DMAs local to a Request.
  kLocalFence = 8,

  // Used to synchronize DMAs across Requests.
  kGlobalFence = 9,
};

// Tracks DMA status.
enum class DmaState {
  // DMA has not started yet.
  kPending,

  // DMA is on-the-fly.
  kActive,

  // DMA has completed.
  kCompleted,

  // DMA had an error.
  kError,
};

// DMA information.
class DmaInfo {
 public:
  DmaInfo(int id, DmaDescriptorType type) : id_(id), type_(type) {}
  DmaInfo(int id, DmaDescriptorType type, const DeviceBuffer& buffer)
      : id_(id), type_(type), buffer_(buffer) {}

  // Accessors.
  int id() const { return id_; }
  DmaDescriptorType type() const { return type_; }
  const DeviceBuffer& buffer() const { return buffer_; }

  // Returns true if DMA is in given state.
  bool IsActive() const { return state_ == DmaState::kActive; }
  bool IsCompleted() const { return state_ == DmaState::kCompleted; }
  bool IsInError() const { return state_ == DmaState::kError; }

  // Sets to given state.
  void MarkActive() { state_ = DmaState::kActive; }
  void MarkCompleted() { state_ = DmaState::kCompleted; }

  // Returns debug-friendly information.
  std::string Dump() const;

 private:
  // ID.
  int id_;

  // Type of DMA.
  DmaDescriptorType type_;

  // DMA status.
  DmaState state_{DmaState::kPending};

  // Memory to DMA from the device point of view.
  DeviceBuffer buffer_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_DMA_INFO_H_
