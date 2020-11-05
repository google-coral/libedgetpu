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

#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <memory>
#include <utility>
#include <vector>

#if !defined(DARWINN_PORT_ANDROID_SYSTEM) && \
    !defined(DARWINN_PORT_ANDROID_EMULATOR)
#include "absl/strings/numbers.h"
#endif

#include "api/chip.h"
#include "api/driver.h"
#include "api/driver_options_generated.h"
#include "driver/aligned_allocator.h"
#include "driver/beagle/beagle_top_level_handler.h"
#include "driver/beagle/beagle_top_level_interrupt_manager.h"
#include "driver/config/beagle/beagle_chip_config.h"
#include "driver/driver_factory.h"
#include "driver/interrupt/grouped_interrupt_controller.h"
#include "driver/interrupt/interrupt_controller.h"
#include "driver/interrupt/interrupt_controller_interface.h"
#include "driver/memory/null_dram_allocator.h"
#include "driver/package_registry.h"
#include "driver/package_verifier.h"
#include "driver/run_controller.h"
#include "driver/scalar_core_controller.h"
#include "driver/usb/local_usb_device.h"
#include "driver/usb/usb_device_interface.h"
#include "driver/usb/usb_driver.h"
#include "driver/usb/usb_ml_commands.h"
#include "driver/usb/usb_registers.h"
#include "driver_shared/time_stamper/driver_time_stamper.h"
#include "port/gflags.h"
#include "port/ptr_util.h"
#include "port/tracing.h"

namespace {
bool GetEnv(const char* env_var, bool default_value) {
#if !defined(DARWINN_PORT_ANDROID_SYSTEM) && \
    !defined(DARWINN_PORT_ANDROID_EMULATOR)
  bool value;
  const char* value_str = std::getenv(env_var);
  if (value_str != nullptr && absl::SimpleAtob(value_str, &value)) return value;
#endif
  return default_value;
}

int GetEnv(const char* env_var, int default_value) {
#if !defined(DARWINN_PORT_ANDROID_SYSTEM) && \
    !defined(DARWINN_PORT_ANDROID_EMULATOR)
  int value;
  const char* value_str = std::getenv(env_var);
  if (value_str != nullptr && absl::SimpleAtoi(value_str, &value)) return value;
#endif
  return default_value;
}

#if defined(__APPLE__)
constexpr int kDefaultUsbMaxNumAsyncTransfers = 1;
#else
constexpr int kDefaultUsbMaxNumAsyncTransfers = 3;
#endif
}  // namespace

/*
 * There are only 3 modes of operation regarding
 * usb_enable_bulk_out_descriptors_from_device and
 * usb_enable_processing_of_hints:
 *
 * 1) both true, we follow the hints, and
 * use descriptors sent from device as validation. This mode doesn't work if
 * the device sends a lot of bulk-out or bulk-in descriptors out which could
 * clog the descriptor/bulk-in pipeline.
 *
 * 2) disable descriptors but enable hints. We blindly follow the hints and
 * send data to device as fast as we can. The mode is similar to the
 * previous one, but could be slightly faster.
 *
 * 3) enable descriptors but disable the hints. we use descriptors from
 * device and pretend there is no hint from code gen, except for first one
 * (for instructions). This mode doesn't work with multiple instruction
 * chunks, as device is not capable of generating descriptors for
 * instructions.
 *
 */
ABSL_FLAG(bool, usb_enable_bulk_descriptors_from_device,
          GetEnv("USB_ENABLE_BULK_DESCRIPTORS_FROM_DEVICE", false),
          "USB set to true if bulk in/out descriptors from device are needed.");
ABSL_FLAG(bool, usb_enable_processing_of_hints,
          GetEnv("USB_ENABLE_PROCESSING_OF_HINTS", true),
          "USB set to true for driver to proactively send data to device.");
ABSL_FLAG(int, usb_timeout_millis, GetEnv("USB_TIMEOUT_MILLIS", 6000),
          "USB timeout in milliseconds");
ABSL_FLAG(bool, usb_reset_back_to_dfu_mode,
          GetEnv("USB_RESET_BACK_TO_DFU_MODE", false),
          "USB find device in app mode, reset back to DFU mode, and terminate");
ABSL_FLAG(
    int, usb_software_credits_low_limit,
    GetEnv("USB_SOFTWARE_CREDITS_LOW_LIMIT", 8192),
    "USB lower bound of bulk out transfer size in bytes, when used in mode 1");
ABSL_FLAG(int, usb_operating_mode, GetEnv("USB_OPERATING_MODE", 2),
          "USB driver operating mode: 0:Multiple-Ep w/ HW, 1:Multiple-Ep w/ "
          "SW, 2:Single-Ep");
ABSL_FLAG(int, usb_max_bulk_out_transfer,
          GetEnv("USB_MAX_BULK_OUT_TRANSFER", 1024 * 1024),
          "USB max bulk out transfer size in bytes");
ABSL_FLAG(int, usb_max_num_async_transfers,
          GetEnv("USB_MAX_NUM_ASYNC_TRANSFERS",
                 kDefaultUsbMaxNumAsyncTransfers),
          "USB max number of pending async bulk out transfer");
ABSL_FLAG(
    bool, usb_force_largest_bulk_in_chunk_size,
    GetEnv("USB_FORCE_LARGEST_BULK_IN_CHUNK_SIZE", false),
    "If true, bulk-in data is transmitted in largest chunks possible. Setting "
    "this to true increase performance on USB2.");
ABSL_FLAG(bool, usb_enable_overlapping_requests,
          GetEnv("USB_ENABLE_OVERLAPPING_REQUESTS", true),
          "Allows the next queued request to be partially overlapped with "
          "the current one.");
ABSL_FLAG(bool, usb_enable_overlapping_bulk_in_and_out,
          GetEnv("USB_ENABLE_OVERLAPPING_BULK_IN_AND_OUT", true),
          "Allows bulk-in trasnfer to be submitted before previous bulk-out "
          "requests complete.");
ABSL_FLAG(bool, usb_enable_queued_bulk_in_requests,
          GetEnv("USB_ENABLE_QUEUED_BULK_IN_REQUESTS", true),
          "Allows bulk-in transfers to be queued to improve performance.");
ABSL_FLAG(
    bool, usb_fail_if_slower_than_superspeed,
    GetEnv("USB_FAIL_IF_SLOWER_THAN_SUPERSPEED", false),
    "USB driver open would fail if the connection is slower than superspeed.");
ABSL_FLAG(int, usb_bulk_in_queue_capacity,
          GetEnv("USB_BULK_IN_QUEUE_CAPACITY", 32),
          "Max number of USB bulk-in requests that can be queued. This "
          "option is only effective when it is positive.");

namespace platforms {
namespace darwinn {
namespace driver {
namespace {

using platforms::darwinn::api::Chip;
using platforms::darwinn::api::Device;

constexpr uint16_t kTargetAppVendorId = 0x18D1;
constexpr uint16_t kTargetAppProductId = 0x9302;

constexpr uint16_t kTargetDfuVendorId = 0x1A6E;
constexpr uint16_t kTargetDfuProductId = 0x089A;

// TODO add proper error handling to this function.
// Convenience function to read a file to a vector.
std::vector<uint8_t> ReadToVector(const std::string& file_name) {
  VLOG(10) << __func__ << file_name;

  // TODO directly read into the vector instead of transcopying through
  // a string.
  std::ifstream ifs(file_name);
  std::string content_string((std::istreambuf_iterator<char>(ifs)),
                             (std::istreambuf_iterator<char>()));

  std::vector<uint8_t> result;
  auto data = reinterpret_cast<const uint8_t*>(content_string.c_str());
  result.insert(result.end(), data, data + content_string.size());
  return result;
}
}  // namespace

class BeagleUsbDriverProvider : public DriverProvider {
 public:
  static std::unique_ptr<DriverProvider> CreateDriverProvider() {
    return gtl::WrapUnique<DriverProvider>(new BeagleUsbDriverProvider());
  }

  ~BeagleUsbDriverProvider() override = default;

  std::vector<Device> Enumerate() override;
  bool CanCreate(const Device& device) override;
  util::StatusOr<std::unique_ptr<api::Driver>> CreateDriver(
      const Device& device, const api::DriverOptions& options) override;

 private:
  BeagleUsbDriverProvider() = default;
};

REGISTER_DRIVER_PROVIDER(BeagleUsbDriverProvider);

std::vector<Device> BeagleUsbDriverProvider::Enumerate() {
  TRACE_SCOPE("BeagleUsbDriverProvider::Enumerate");

  LocalUsbDeviceFactory usb_device_factory;
  std::vector<Device> device_list;

  auto usb_dfu_device_list_or_error = usb_device_factory.EnumerateDevices(
      kTargetDfuVendorId, kTargetDfuProductId);

  auto usb_app_device_list_or_error = usb_device_factory.EnumerateDevices(
      kTargetAppVendorId, kTargetAppProductId);

  if (usb_dfu_device_list_or_error.ok()) {
    for (const auto& path : usb_dfu_device_list_or_error.ValueOrDie()) {
      device_list.push_back({Chip::kBeagle, Device::Type::USB, path});
      VLOG(10) << StringPrintf("%s: adding path [%s]", __func__, path.c_str());
    }
  }

  if (usb_app_device_list_or_error.ok()) {
    for (const auto& path : usb_app_device_list_or_error.ValueOrDie()) {
      device_list.push_back({Chip::kBeagle, Device::Type::USB, path});
      VLOG(10) << StringPrintf("%s: adding path [%s]", __func__, path.c_str());
    }
  }

  return device_list;
}

bool BeagleUsbDriverProvider::CanCreate(const Device& device) {
  return device.type == Device::Type::USB && device.chip == Chip::kBeagle;
}

util::StatusOr<std::unique_ptr<api::Driver>>
BeagleUsbDriverProvider::CreateDriver(
    const Device& device, const api::DriverOptions& driver_options) {
  TRACE_SCOPE("BeagleUsbDriverProvider::CreateDriver");

  if (!CanCreate(device)) {
    return util::NotFoundError("Unsupported device.");
  }

  auto config = gtl::MakeUnique<config::BeagleChipConfig>();

  UsbDriver::UsbDriverOptions options;
  options.usb_force_largest_bulk_in_chunk_size =
      absl::GetFlag(FLAGS_usb_force_largest_bulk_in_chunk_size);
  options.usb_enable_bulk_descriptors_from_device =
      absl::GetFlag(FLAGS_usb_enable_bulk_descriptors_from_device);
  options.usb_enable_processing_of_hints =
      absl::GetFlag(FLAGS_usb_enable_processing_of_hints);
  options.usb_max_num_async_transfers =
      absl::GetFlag(FLAGS_usb_max_num_async_transfers);
  options.mode = static_cast<UsbDriver::OperatingMode>(
      absl::GetFlag(FLAGS_usb_operating_mode));
  options.max_bulk_out_transfer_size_in_bytes =
      absl::GetFlag(FLAGS_usb_max_bulk_out_transfer);
  options.software_credits_lower_limit_in_bytes =
      absl::GetFlag(FLAGS_usb_software_credits_low_limit);
  options.usb_enable_overlapping_requests =
      absl::GetFlag(FLAGS_usb_enable_overlapping_requests);
  options.usb_enable_overlapping_bulk_in_and_out =
      absl::GetFlag(FLAGS_usb_enable_overlapping_bulk_in_and_out);
  options.usb_fail_if_slower_than_superspeed =
      absl::GetFlag(FLAGS_usb_fail_if_slower_than_superspeed);
  options.usb_enable_queued_bulk_in_requests =
      absl::GetFlag(FLAGS_usb_enable_queued_bulk_in_requests);
  options.usb_bulk_in_queue_capacity =
      absl::GetFlag(FLAGS_usb_bulk_in_queue_capacity);

  auto usb_registers = gtl::MakeUnique<UsbRegisters>();
  std::vector<std::unique_ptr<InterruptControllerInterface>>
      top_level_interrupt_controllers;
  top_level_interrupt_controllers.push_back(
      gtl::MakeUnique<InterruptController>(
          config->GetUsbTopLevel0InterruptCsrOffsets(), usb_registers.get()));
  top_level_interrupt_controllers.push_back(
      gtl::MakeUnique<InterruptController>(
          config->GetUsbTopLevel1InterruptCsrOffsets(), usb_registers.get()));
  top_level_interrupt_controllers.push_back(
      gtl::MakeUnique<InterruptController>(
          config->GetUsbTopLevel2InterruptCsrOffsets(), usb_registers.get()));
  top_level_interrupt_controllers.push_back(
      gtl::MakeUnique<InterruptController>(
          config->GetUsbTopLevel3InterruptCsrOffsets(), usb_registers.get()));
  auto top_level_interrupt_controller =
      gtl::MakeUnique<GroupedInterruptController>(
          &top_level_interrupt_controllers);

  auto top_level_interrupt_manager =
      gtl::MakeUnique<BeagleTopLevelInterruptManager>(
          std::move(top_level_interrupt_controller), *config,
          usb_registers.get());

  auto fatal_error_interrupt_controller = gtl::MakeUnique<InterruptController>(
      config->GetUsbFatalErrorInterruptCsrOffsets(), usb_registers.get());

  auto top_level_handler = gtl::MakeUnique<BeagleTopLevelHandler>(
      *config, usb_registers.get(),
      /*use_usb=*/true, driver_options.performance_expectation());

  const api::DriverUsbOptions* usb_options = driver_options.usb();
  if (usb_options != nullptr) {
    if (usb_options->dfu_firmware() != nullptr) {
      auto provided_dfu_path = usb_options->dfu_firmware()->str();
      if (!provided_dfu_path.empty()) {
        // try loading firmware into memory.
        options.usb_firmware_image = ReadToVector(provided_dfu_path);
      }
    }
    options.usb_always_dfu = usb_options->always_dfu();

    // Override command line options if driver options are set.
    // Command line options are easier to use for command line tools, but
    // most other use cases should set the driver option.

    if (usb_options->has_fail_if_slower_than_superspeed()) {
      options.usb_fail_if_slower_than_superspeed =
          usb_options->fail_if_slower_than_superspeed();
    }

    if (usb_options->has_force_largest_bulk_in_chunk_size()) {
      options.usb_force_largest_bulk_in_chunk_size =
          usb_options->force_largest_bulk_in_chunk_size();
    }

    if (usb_options->has_enable_overlapping_bulk_in_and_out()) {
      options.usb_enable_overlapping_bulk_in_and_out =
          usb_options->enable_overlapping_bulk_in_and_out();
    }

    if (usb_options->has_enable_queued_bulk_in_requests()) {
      options.usb_enable_queued_bulk_in_requests =
          usb_options->enable_queued_bulk_in_requests();
    }

    if (usb_options->has_bulk_in_queue_capacity()) {
      options.usb_bulk_in_queue_capacity =
          usb_options->bulk_in_queue_capacity();
    }
  }

  auto dram_allocator = gtl::MakeUnique<NullDramAllocator>();

  std::string path(device.path);
  ASSIGN_OR_RETURN(auto verifier, MakeExecutableVerifier(flatbuffers::GetString(
                                      driver_options.public_key())));
  auto executable_registry = gtl::MakeUnique<PackageRegistry>(
      device.chip, std::move(verifier), dram_allocator.get());

  auto time_stamper = gtl::MakeUnique<driver_shared::DriverTimeStamper>();

  // Note that although driver_options is passed into constructor of UsbDriver,
  // it's USB portion is not used by the driver directly, due to historical
  // reasons.
  return {gtl::MakeUnique<UsbDriver>(
      driver_options, std::move(config),
      [path] {
        LocalUsbDeviceFactory usb_device_factory;

        return usb_device_factory.OpenDevice(
            path, absl::GetFlag(FLAGS_usb_timeout_millis));
      },
      std::move(usb_registers), std::move(top_level_interrupt_manager),
      std::move(fatal_error_interrupt_controller), std::move(top_level_handler),
      std::move(dram_allocator), std::move(executable_registry), options,
      std::move(time_stamper))};
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
