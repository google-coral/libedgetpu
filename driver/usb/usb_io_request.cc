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

#include "driver/usb/usb_io_request.h"

#include "port/logging.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/stringprintf.h"
#include "port/unreachable.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace {

// Returns IO type for given DMA.
UsbIoRequest::Type ConvertToIoType(const DmaInfo& info) {
  switch (info.type()) {
    case DmaDescriptorType::kInstruction:
    case DmaDescriptorType::kInputActivation:
    case DmaDescriptorType::kParameter:
      return UsbIoRequest::Type::kBulkOut;

    case DmaDescriptorType::kOutputActivation:
      return UsbIoRequest::Type::kBulkIn;

    case DmaDescriptorType::kScalarCoreInterrupt0:
    case DmaDescriptorType::kScalarCoreInterrupt1:
    case DmaDescriptorType::kScalarCoreInterrupt2:
    case DmaDescriptorType::kScalarCoreInterrupt3:
      return UsbIoRequest::Type::kScHostInterrupt;

    default:
      LOG(FATAL) << "Cannot be converted";
      unreachable();  // NOLINT
  }
}

}  // namespace

UsbIoRequest::UsbIoRequest(int id, UsbMlCommands::DescriptorTag tag)
    : id_(id),
      source_and_match_status_(
          UsbIoRequest::SourceAndMatchStatus::kSubmittedByDevice),
      type_(Type::kScHostInterrupt),
      tag_(tag),
      chunker_(DmaChunker::HardwareProcessing::kCommitted, DeviceBuffer()) {}

UsbIoRequest::UsbIoRequest(int id, UsbIoRequest::Type type,
                                 UsbMlCommands::DescriptorTag tag,
                                 const DeviceBuffer& buffer)
    : id_(id),
      source_and_match_status_(
          UsbIoRequest::SourceAndMatchStatus::kSubmittedByDevice),
      type_(type),
      tag_(tag),
      // Bytes transferred for BulkIn is determined by device, and may not match
      // the descriptor. In such case, BulkIn will be delivered in chunks.
      chunker_((type == Type::kBulkIn)
                   ? DmaChunker::HardwareProcessing::kBestEffort
                   : DmaChunker::HardwareProcessing::kCommitted,
               buffer) {}

UsbIoRequest::UsbIoRequest(DmaInfo* dma_info)
    : id_([dma_info]() {
        CHECK(dma_info != nullptr);
        return dma_info->id();
      }()),
      source_and_match_status_(SourceAndMatchStatus::kHintNotYetMatched),
      type_(ConvertToIoType(*dma_info)),
      tag_(static_cast<UsbMlCommands::DescriptorTag>(dma_info->type())),
      // Bytes transferred for BulkIn is determined by device, and may not match
      // the descriptor. In such case, BulkIn will be delivered in chunks.
      chunker_((type_ == Type::kBulkIn)
                   ? DmaChunker::HardwareProcessing::kBestEffort
                   : DmaChunker::HardwareProcessing::kCommitted,
               dma_info->buffer()),
      dma_info_(dma_info) {}

void UsbIoRequest::SetMatched() {
  CHECK(dma_info_ != nullptr);
  VLOG(9) << StringPrintf("DMA[%d] hint matched with descriptor",
                          dma_info_->id());
  source_and_match_status_ = SourceAndMatchStatus::kHintAlreadyMatched;
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
