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

#ifndef DARWINN_DRIVER_MEMORY_DMA_DIRECTION_H_
#define DARWINN_DRIVER_MEMORY_DMA_DIRECTION_H_

namespace platforms {
namespace darwinn {
namespace driver {

// Mimics the Linux kernel dma_data_direction enum in the DMA API.
// This indicates the direction which data moves during a DMA transfer, and is a
// useful hint to pass to the kernel when mapping buffers.
enum class DmaDirection {
  // DMA_TO_DEVICE: CPU caches are flushed at mapping time.
  kToDevice = 1,
  // DMA_FROM_DEVICE: CPU caches are invalidated at unmapping time.
  kFromDevice = 2,
  // DMA_BIDIRECTIONAL: Both of the above.
  kBidirectional = 0,
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_MEMORY_DMA_DIRECTION_H_
