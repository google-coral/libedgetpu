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

#include "driver/beagle/beagle_pci_driver_provider.h"
#include "driver/kernel/windows/kernel_coherent_allocator_windows.h"
#include "driver/kernel/windows/kernel_event_handler_windows.h"
#include "driver/kernel/windows/kernel_registers_windows.h"
#include "port/integral_types.h"
#include "port/ptr_util.h"

namespace platforms {
namespace darwinn {
namespace driver {

class BeaglePciDriverProviderWindows : public BeaglePciDriverProvider {
 public:
  static std::unique_ptr<DriverProvider> CreateDriverProvider() {
    return gtl::WrapUnique<DriverProvider>(
        new BeaglePciDriverProviderWindows());
  }

 protected:
  std::unique_ptr<KernelCoherentAllocator> CreateKernelCoherentAllocator(
      const std::string& device_path, int alignment_bytes,
      size_t size_bytes) override {
    return gtl::MakeUnique<KernelCoherentAllocatorWindows>(
        device_path, alignment_bytes, size_bytes);
  }

  std::unique_ptr<KernelRegisters> CreateKernelRegisters(
      const std::string& device_path,
      const std::vector<KernelRegisters::MmapRegion>& mmap_region,
      bool read_only) override {
    return gtl::MakeUnique<KernelRegistersWindows>(device_path, mmap_region,
                                                   read_only);
  }

  std::unique_ptr<KernelInterruptHandler> CreateKernelInterruptHandler(
      const std::string& device_path) override {
    auto event_handler = gtl::MakeUnique<KernelEventHandlerWindows>(
        device_path, DW_INTERRUPT_COUNT);
    return gtl::MakeUnique<KernelInterruptHandler>(std::move(event_handler));
  }

 private:
  BeaglePciDriverProviderWindows() = default;
};

REGISTER_DRIVER_PROVIDER(BeaglePciDriverProviderWindows);

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
