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

#ifndef DARWINN_DRIVER_DMA_INFO_EXTRACTOR_H_
#define DARWINN_DRIVER_DMA_INFO_EXTRACTOR_H_

#include <list>

#include "driver/device_buffer_mapper.h"
#include "driver/dma_info.h"
#include "driver/package_registry.h"
#include "executable/executable_generated.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Extracts DMAs to be performed by driver.
class DmaInfoExtractor {
 public:
  // Determines how to extract DMA infos for the executable.
  enum class ExtractorType {
    // Extracts only instruction DMAs for baseline PCIe usecase.
    kInstructionDma = 0,

    // Extracts through DMA hints for USB usecase.
    kDmaHints = 1,

    // Extracts only first instruction DMA for USB usecase.
    kFirstInstruction = 2,
  };

  explicit DmaInfoExtractor(ExtractorType type)
      : DmaInfoExtractor(type, true) {}
  DmaInfoExtractor(ExtractorType type, bool overlap_requests);
  virtual ~DmaInfoExtractor() = default;

  // Extracts a list of DMAs to be performed.
  virtual std::list<DmaInfo> ExtractDmaInfos(
      const ExecutableReference& executable_reference,
      const DeviceBufferMapper& buffers) const;

 private:
  // Extracts instruction DMAs.
  std::list<DmaInfo> ExtractInstructionDmaInfos(
      const DeviceBufferMapper& buffers) const;

  // Extracts DMA hints.
  std::list<DmaInfo> ExtractDmaHints(
      const ExecutableReference& executable_reference,
      const DeviceBufferMapper& buffers) const;

  // Extracts first instruction DMA.
  std::list<DmaInfo> ExtractFirstInstruction(
      const DeviceBufferMapper& buffers) const;

  // Extractor type.
  const ExtractorType type_;

  // True if requests can be overlapped. Should be set to false just for
  // debugging.
  const bool overlap_requests_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_DMA_INFO_EXTRACTOR_H_
