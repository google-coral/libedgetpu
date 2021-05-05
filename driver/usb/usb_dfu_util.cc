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

#include "driver/usb/usb_dfu_util.h"

#include <fstream>
#include <vector>

#include "port/errors.h"
#include "port/logging.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/stringprintf.h"
#include "port/time.h"
#include "port/tracing.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace {

// TODO: Consider an absl::Duration instead
// Sleep time after a reset for DFU.
// TODO: revisit this setting after we finalize PHY tuning.
constexpr int kSleepTimeSecondsAfterReset = 4;

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

Status UsbUpdateDfuDevice(UsbDfuCommands* dfu_device,
                          UsbDeviceInterface::ConstBuffer firmware_image,
                          bool skip_verify) {
  TRACE_SCOPE("UsbUpdateDfuDevice");

  VLOG(10) << StringPrintf("%s Loading descriptor for the first configuration",
                           __func__);

  constexpr size_t kMaxConfigDescriptorAllowed = 512;

  ASSIGN_OR_RETURN(UsbDfuCommands::ConfigurationDescriptor config_descriptor,
                   dfu_device->GetConfigurationDescriptor(
                       UsbDeviceInterface::kFirstDeviceConfiguration,
                       kMaxConfigDescriptorAllowed));

  ASSIGN_OR_RETURN(auto dfu_interfaces,
                   dfu_device->FindDfuInterfaces(config_descriptor.raw_data));

  const int dfu_interface = dfu_interfaces.first.begin()->interface_number;

  VLOG(10) << StringPrintf(
      "%s Num of DFU interfaces %zu, claiming interface %d", __func__,
      dfu_interfaces.first.size(), dfu_interface);

  RETURN_IF_ERROR(dfu_device->ClaimInterface(dfu_interface));

  dfu_device->SetDfuInterface(dfu_interface);

  RETURN_IF_ERROR(
      dfu_device->UpdateFirmware(dfu_interfaces.second, firmware_image));

  if (skip_verify) {
    return Status();  // OK.
  } else {
    return dfu_device->ValidateFirmware(dfu_interfaces.second, firmware_image);
  }
}

Status UsbUpdateAllDfuDevices(UsbManager* usb_manager, uint16_t vendor_id,
                              uint16_t product_id,
                              const std::string& firmware_filename,
                              bool skip_verify) {
  VLOG(7) << StringPrintf("%s Downloading firmware file:%s", __func__,
                          firmware_filename.c_str());

  auto firmware_image = ReadToVector(firmware_filename);
  if (firmware_image.empty()) {
    return InvalidArgumentError("Invalid DFU image file");
  }

  // Perform DFU on all devices that have the right product ID.
  bool is_dfu_attempted = false;
  constexpr int kMaxNumDfuRun = 10;
  for (int dfu_count = 0; dfu_count < kMaxNumDfuRun; ++dfu_count) {
    auto dfu_target =
        usb_manager->OpenDevice(vendor_id, product_id, UsbManager::kDoNotRetry);
    if (!dfu_target.ok()) {
      // Leave this step if we couldn't find any device for DFU.
      VLOG(7) << StringPrintf("%s No more device is in need for DFU", __func__);
      break;
    }

    is_dfu_attempted = true;
    VLOG(7) << StringPrintf("%s Performing DFU on device %d", __func__,
                            dfu_count);

    UsbDfuCommands dfu_commands(std::move(dfu_target.ValueOrDie()),
                                UsbDeviceInterface::kTimeoutOneSecond);
    // Return error if we encounter any error in the DFU process.
    // Returning here avoid trying DFU on the same faulty
    // device indefinitely.
    RETURN_IF_ERROR(
        UsbUpdateDfuDevice(&dfu_commands, firmware_image, skip_verify));
    RETURN_IF_ERROR(
        dfu_commands.Close(UsbDfuCommands::CloseAction::kGracefulPortReset));
  }

  if (is_dfu_attempted) {
    // Wait for short period of time so the devices could come back after reset.
    VLOG(7) << StringPrintf(
        "%s DFU completed. Waiting for devices to come back", __func__);
    Sleep(kSleepTimeSecondsAfterReset);
  }

  return Status();  // OK.
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
