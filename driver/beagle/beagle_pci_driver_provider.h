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

#ifndef DARWINN_DRIVER_BEAGLE_BEAGLE_PCI_DRIVER_PROVIDER_H_
#define DARWINN_DRIVER_BEAGLE_BEAGLE_PCI_DRIVER_PROVIDER_H_

#include "api/chip.h"
#include "api/driver.h"
#include "driver/driver_factory.h"
#include "driver/kernel/kernel_coherent_allocator.h"
#include "driver/kernel/kernel_interrupt_handler.h"
#include "driver/kernel/kernel_registers.h"
#include "port/integral_types.h"
#include "port/ptr_util.h"

namespace platforms {
namespace darwinn {
namespace driver {

class BeaglePciDriverProvider : public DriverProvider {
 public:
  std::vector<api::Device> Enumerate() override;
  bool CanCreate(const api::Device& device) override;
  StatusOr<std::unique_ptr<api::Driver>> CreateDriver(
      const api::Device& device, const api::DriverOptions& options) override;

 protected:
  virtual std::unique_ptr<KernelCoherentAllocator>
  CreateKernelCoherentAllocator(const std::string& device_path,
                                int alignment_bytes, size_t size_bytes) = 0;
  virtual std::unique_ptr<KernelRegisters> CreateKernelRegisters(
      const std::string& device_path,
      const std::vector<KernelRegisters::MmapRegion>& mmap_region,
      bool read_only) = 0;
  virtual std::unique_ptr<KernelInterruptHandler> CreateKernelInterruptHandler(
      const std::string& device_path) = 0;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_BEAGLE_BEAGLE_PCI_DRIVER_PROVIDER_H_
