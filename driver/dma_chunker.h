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

#ifndef DARWINN_DRIVER_DMA_CHUNKER_H_
#define DARWINN_DRIVER_DMA_CHUNKER_H_

#include <stddef.h>

#include "driver/device_buffer.h"

namespace platforms {
namespace darwinn {
namespace driver {

// A class to chunk DMAs into smaller DMAs given hardware constraints.
//
// Hardware can be:
// (1) HardwareProcessing::kCommitted
// Chunk given out from this class will be always processed in full, so
// DmaChunker will give out next chunk from previously given out chunk.
//
// (2) HardwareProcessing::kBestEffort
// Chunk given out from this class will be processed best-effort, and may
// partially fulfilled. Until DmaChunker is notified a completion of transfer
// (could be partial number of bytes), DmaChunker will give out the same chunk.
class DmaChunker {
 public:
  // Indicates how DMA will be processed in HW.
  enum class HardwareProcessing {
    // Chunked DMA will be always processed in full by HW.
    kCommitted,

    // Chunked DMA will be processed best-effort, and HW may partially perform
    // DMA.
    kBestEffort,
  };

  DmaChunker(HardwareProcessing processing, const DeviceBuffer& buffer)
      : processing_(processing), buffer_(buffer) {}

  // Returns true if there is next DMA chunk.
  bool HasNextChunk() const {
    return GetNextChunkOffset() < buffer_.size_bytes();
  }

  // Returns next DMA chunk to perform in full.
  DeviceBuffer GetNextChunk();

  // Returns next DMA chunk to perform upto "num_bytes".
  DeviceBuffer GetNextChunk(int num_bytes);

  // Notifies that "transferred_bytes" amount of data has been transferred.
  void NotifyTransfer(int transferred_bytes);

  // Returns true if transfer is active/completed.
  bool IsActive() const { return active_bytes_ > 0; }
  bool IsCompleted() const {
    return buffer_.size_bytes() == transferred_bytes_;
  }

  // Returns total DMA buffer.
  const DeviceBuffer& buffer() const { return buffer_; }

  // Returns how many active transfers are out, where each transfer is "bytes".
  int GetActiveCounts(int bytes) const {
    // Want to calculate CeilOfRatio(active_btyes_, bytes)
    const int floor = active_bytes_ / bytes;
    return floor + ((floor * bytes) < active_bytes_);
  }

 private:
  // Returns next chunk offset to transfer.
  int GetNextChunkOffset() const;

  // Marks "num_bytes" as actively transferred.
  void MarkActive(int num_bytes);

  // Hardware constraints.
  const HardwareProcessing processing_;

  // DeviceBuffer underlying DMA.
  const DeviceBuffer buffer_;

  // Number of actively transferring bytes.
  size_t active_bytes_{0};

  // Number of transferred bytes.
  size_t transferred_bytes_{0};
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_DMA_CHUNKER_H_
