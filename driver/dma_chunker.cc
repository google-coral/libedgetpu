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

#include "driver/dma_chunker.h"

#include <algorithm>
#include <string>

#include "port/logging.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

DeviceBuffer DmaChunker::GetNextChunk() {
  const auto curr_offset = GetNextChunkOffset();
  const int remaining_bytes = buffer_.size_bytes() - curr_offset;
  VLOG(10) << StringPrintf(
      "Completed %zd bytes; Outstanding %zd bytes; Processing next %d bytes",
      transferred_bytes_, active_bytes_, remaining_bytes);

  MarkActive(remaining_bytes);
  return buffer_.Slice(curr_offset, remaining_bytes);
}

DeviceBuffer DmaChunker::GetNextChunk(int num_bytes) {
  const auto curr_offset = GetNextChunkOffset();
  const int remaining_bytes = buffer_.size_bytes() - curr_offset;
  const int transfer_bytes = std::min(remaining_bytes, num_bytes);
  VLOG(10) << StringPrintf(
      "Completed %zd bytes; Outstanding %zd bytes; Processing next %d bytes",
      transferred_bytes_, active_bytes_, transfer_bytes);

  MarkActive(transfer_bytes);
  return buffer_.Slice(curr_offset, transfer_bytes);
}

void DmaChunker::NotifyTransfer(int transferred_bytes) {
  transferred_bytes_ += transferred_bytes;
  CHECK_GE(active_bytes_, transferred_bytes);
  switch (processing_) {
    case HardwareProcessing::kCommitted:
      active_bytes_ -= transferred_bytes;
      break;

    case HardwareProcessing::kBestEffort:
      // Active bytes may be partially dropped by HW. Re-chunk them.
      active_bytes_ = 0;
      break;
  }
  CHECK_LE(transferred_bytes_, buffer_.size_bytes());
}

int DmaChunker::GetNextChunkOffset() const {
  switch (processing_) {
    case HardwareProcessing::kCommitted:
      return transferred_bytes_ + active_bytes_;

    case HardwareProcessing::kBestEffort:
      return transferred_bytes_;
  }
}

void DmaChunker::MarkActive(int num_bytes) {
  switch (processing_) {
    case HardwareProcessing::kCommitted:
      active_bytes_ += num_bytes;
      return;

    case HardwareProcessing::kBestEffort:
      // Previous active bytes are irrelavant as best-effort can drop them.
      active_bytes_ = num_bytes;
      return;
  }
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
