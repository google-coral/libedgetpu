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

#ifndef DARWINN_PORT_DEFAULT_DMA_MANAGER_H_
#define DARWINN_PORT_DEFAULT_DMA_MANAGER_H_

#include "firmware/common/errors.h"
#include "firmware/common/iomem.h"
#include "firmware/common/status.h"
#include "firmware/driver/dma/dma330_interface.h"
#include "firmware/driver/dma/dma_manager_interface.h"

namespace platforms {
namespace darwinn {

class DmaManager : public firmware::driver::dma::DmaManagerInterface {
 public:
  DmaManager(firmware::common::IomemInterface* iomem) : iomem_(*iomem) {}

  firmware::common::Status IssueTransfer(
      const firmware::common::Buffer& source,
      const firmware::common::Buffer& destination,
      firmware::driver::dma::Dma330Interface::Channel channel,
      firmware::driver::dma::DmaDoneHandler handler) override {
    if (source.size_bytes() != destination.size_bytes()) {
      return firmware::common::InvalidArgumentError();
    }
    iomem_.IoMemcpy(reinterpret_cast<void*>(destination.address()),
                    reinterpret_cast<const void*>(source.address()),
                    source.size_bytes());
    handler(firmware::common::Status::OK());
    return firmware::common::Status::OK();
  }

  firmware::common::Status CancelTransfer(
      firmware::driver::dma::Dma330Interface::Channel channel) override {
    return firmware::common::Status::OK();
  }

  firmware::common::StatusOr<firmware::driver::dma::Dma330Interface::Channel>
  AllocateChannel() override {
    return firmware::driver::dma::Dma330Interface::Channel::k0;
  }

  void FreeChannel(
      firmware::driver::dma::Dma330Interface::Channel channel) override{};

  static bool DmaAccessible(const firmware::common::Buffer& buffer) {
    return true;
  }

 private:
  firmware::common::IomemInterface& iomem_;
};

}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_PORT_DEFAULT_DMA_MANAGER_H_
