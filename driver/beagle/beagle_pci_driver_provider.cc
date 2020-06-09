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

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "api/chip.h"
#include "api/driver.h"
#include "driver/aligned_allocator.h"
#include "driver/beagle/beagle_kernel_top_level_handler.h"
#include "driver/config/beagle/beagle_chip_config.h"
#include "driver/config/chip_structures.h"
#include "driver/driver_factory.h"
#include "driver/hardware_structures.h"
#include "driver/interrupt/dummy_interrupt_controller.h"
#include "driver/interrupt/grouped_interrupt_controller.h"
#include "driver/interrupt/interrupt_handler.h"
#include "driver/interrupt/top_level_interrupt_manager.h"
#include "driver/kernel/kernel_coherent_allocator.h"
#include "driver/kernel/kernel_interrupt_handler.h"
#include "driver/kernel/kernel_mmu_mapper.h"
#include "driver/kernel/kernel_registers.h"
#include "driver/kernel/kernel_wire_interrupt_handler.h"
#include "driver/memory/dual_address_space.h"
#include "driver/memory/null_dram_allocator.h"
#include "driver/mmio/host_queue.h"
#include "driver/mmio_driver.h"
#include "driver/package_registry.h"
#include "driver/package_verifier.h"
#include "driver/run_controller.h"
#include "driver/scalar_core_controller.h"
#include "driver/time_stamper/driver_time_stamper.h"
#include "port/integral_types.h"
#include "port/ptr_util.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace {

using platforms::darwinn::api::Chip;
using platforms::darwinn::api::Device;

}  // namespace

class BeaglePciDriverProvider : public DriverProvider {
 public:
  static std::unique_ptr<DriverProvider> CreateDriverProvider() {
    return gtl::WrapUnique<DriverProvider>(new BeaglePciDriverProvider());
  }

  ~BeaglePciDriverProvider() override = default;

  std::vector<Device> Enumerate() override;
  bool CanCreate(const Device& device) override;
  util::StatusOr<std::unique_ptr<api::Driver>> CreateDriver(
      const Device& device, const api::DriverOptions& options) override;

 private:
  BeaglePciDriverProvider() = default;
};

REGISTER_DRIVER_PROVIDER(BeaglePciDriverProvider);

std::vector<Device> BeaglePciDriverProvider::Enumerate() {
  return EnumerateSysfs("apex", Chip::kBeagle, Device::Type::PCI);
}

bool BeaglePciDriverProvider::CanCreate(const Device& device) {
  return device.type == Device::Type::PCI && device.chip == Chip::kBeagle;
}

util::StatusOr<std::unique_ptr<api::Driver>>
BeaglePciDriverProvider::CreateDriver(const Device& device,
                                      const api::DriverOptions& options) {
  if (!CanCreate(device)) {
    return util::NotFoundError("Unsupported device.");
  }

  // TODO: Following queue size could come from a config.
  constexpr int kInstructionQueueSize = 256;

  // Coherent memory block granted to the Host Queue
  constexpr int kCoherentAllocatorMaxSizeByte = 0x4000;

  auto config = gtl::MakeUnique<config::BeagleChipConfig>();

  // Offsets are embedded in the CSR spec.
  constexpr uint64 kTileConfig0Offset = 0x40000;
  constexpr uint64 kScalarCoreOffset = 0x44000;
  constexpr uint64 kUserHibOffset = 0x48000;

  // Memory mapping must be aligned with page size. Assuming 4KB page size.
  constexpr uint64 kSectionSize = 0x1000;

  const std::vector<KernelRegisters::MmapRegion> regions = {
      {kTileConfig0Offset, kSectionSize},
      {kScalarCoreOffset, kSectionSize},
      {kUserHibOffset, kSectionSize},
  };
  auto registers = gtl::MakeUnique<KernelRegisters>(device.path, regions,
                                                    /*read_only=*/false);

  auto interrupt_handler = gtl::MakeUnique<KernelInterruptHandler>(device.path);
  auto top_level_handler = gtl::MakeUnique<BeagleKernelTopLevelHandler>(
      device.path, options.performance_expectation());
  auto mmu_mapper = gtl::MakeUnique<KernelMmuMapper>(device.path);
  auto address_space = gtl::MakeUnique<DualAddressSpace>(
      config->GetChipStructures(), mmu_mapper.get());
  int allocation_alignment_bytes =
      config->GetChipStructures().allocation_alignment_bytes;
  auto allocator =
      gtl::MakeUnique<AlignedAllocator>(allocation_alignment_bytes);
  auto coherent_allocator = gtl::MakeUnique<KernelCoherentAllocator>(
      device.path, allocation_alignment_bytes, kCoherentAllocatorMaxSizeByte);
  auto host_queue =
      gtl::MakeUnique<HostQueue<HostQueueDescriptor, HostQueueStatusBlock>>(
          config->GetInstructionQueueCsrOffsets(), config->GetChipStructures(),
          registers.get(), std::move(coherent_allocator),
          kInstructionQueueSize, /*single_descriptor_mode=*/false);

  // Keeping the number of interrupt so MmioDriver would still register for four
  // interrupt handlers.
  constexpr int kNumTopLevelInterrupts = 4;
  auto top_level_interrupt_controller =
      gtl::MakeUnique<DummyInterruptController>(kNumTopLevelInterrupts);

  // TODO Bridge top level interrupts to higher level logic.
  // TopLevelInterruptManager initialized with DummyInterruptController leaves
  // top level interrupts not really handled. We will have to further
  // extend TopLevelInterruptManager to bridge top level interrupt to
  // application/driver logic.
  auto top_level_interrupt_manager = gtl::MakeUnique<TopLevelInterruptManager>(
      std::move(top_level_interrupt_controller));

  auto fatal_error_interrupt_controller = gtl::MakeUnique<InterruptController>(
      config->GetFatalErrorInterruptCsrOffsets(), registers.get());
  auto scalar_core_controller =
      gtl::MakeUnique<ScalarCoreController>(*config, registers.get());
  auto run_controller =
      gtl::MakeUnique<RunController>(*config, registers.get());

  auto dram_allocator = gtl::MakeUnique<NullDramAllocator>();

  ASSIGN_OR_RETURN(
      auto verifier,
      MakeExecutableVerifier(flatbuffers::GetString(options.public_key())));
  auto executable_registry = gtl::MakeUnique<PackageRegistry>(
      device.chip, std::move(verifier), dram_allocator.get());
  auto time_stamper = gtl::MakeUnique<DriverTimeStamper>();

  return {gtl::MakeUnique<MmioDriver>(
      options, std::move(config), std::move(registers),
      std::move(dram_allocator), std::move(mmu_mapper),
      std::move(address_space), std::move(allocator), std::move(host_queue),
      std::move(interrupt_handler), std::move(top_level_interrupt_manager),
      std::move(fatal_error_interrupt_controller),
      std::move(scalar_core_controller), std::move(run_controller),
      std::move(top_level_handler), std::move(executable_registry),
      std::move(time_stamper))};
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
