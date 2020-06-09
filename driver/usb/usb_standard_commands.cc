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

#include "driver/usb/usb_standard_commands.h"

#include <utility>

#include "driver/usb/usb_device_interface.h"
#include "port/errors.h"
#include "port/logging.h"
#include "port/ptr_util.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

UsbStandardCommands::UsbStandardCommands(
    std::unique_ptr<UsbDeviceInterface> device,
    TimeoutMillis default_timeout_msec)
    : device_(std::move(device)), default_timeout_msec_(default_timeout_msec) {
  VLOG(10) << __func__;
}

UsbStandardCommands::~UsbStandardCommands() { VLOG(10) << __func__; }

util::StatusOr<UsbStandardCommands::DeviceDescriptor>
UsbStandardCommands::GetDeviceDescriptor() {
  VLOG(10) << __func__;
  // The raw size of standard USB device descriptor.
  constexpr size_t kDeviceDescriptorRawByteSize = 18;
  uint8 descriptor_buffer[kDeviceDescriptorRawByteSize];
  size_t num_bytes_transferred = 0;

  RETURN_IF_ERROR(
      GetDescriptor(UsbDeviceInterface::DescriptorType::kDevice, 0,
                    gtl::MutableArraySlice<uint8>(descriptor_buffer,
                                                  sizeof(descriptor_buffer)),
                    &num_bytes_transferred, __func__));

  if (num_bytes_transferred < kDeviceDescriptorRawByteSize) {
    return util::UnknownError("Device descriptor is too short");
  }

  DeviceDescriptor descriptor = {0};

  // Fill fields from raw bytes to more accessible data structure.
  // memcpy is used for all multi-byte data fields to avoid any potential
  // alignment issues.
  memcpy(&descriptor.usb_version_bcd, descriptor_buffer + 2, 2);
  descriptor.device_class =
      static_cast<UsbDeviceInterface::DeviceClass>(descriptor_buffer[4]);
  descriptor.device_subclass = descriptor_buffer[5];
  descriptor.bDeviceProtocol = descriptor_buffer[6];
  descriptor.max_packet_size_0 = descriptor_buffer[7];
  memcpy(&descriptor.vendor_id, descriptor_buffer + 8, 2);
  memcpy(&descriptor.product_id, descriptor_buffer + 10, 2);
  memcpy(&descriptor.device_version_bcd, descriptor_buffer + 12, 2);
  descriptor.manufacturer_name_index = descriptor_buffer[14];
  descriptor.product_name_index = descriptor_buffer[15];
  descriptor.serial_number_index = descriptor_buffer[16];
  descriptor.num_configurations = descriptor_buffer[17];

  VLOG(7) << StringPrintf("Vender ID: 0x%x", descriptor.vendor_id);
  VLOG(7) << StringPrintf("Product ID: 0x%x", descriptor.product_id);

  return descriptor;
}

util::StatusOr<UsbStandardCommands::ConfigurationDescriptor>
UsbStandardCommands::GetConfigurationDescriptor(uint8_t index,
                                                size_t max_extra_data_length) {
  VLOG(10) << StringPrintf("%s index %d", __func__, index);
  // The raw size of standard USB device descriptor.
  constexpr size_t kConfigDescriptorRawByteSize = 9;
  const size_t total_data_length =
      kConfigDescriptorRawByteSize + max_extra_data_length;
  size_t num_bytes_transferred = 0;
  ConfigurationDescriptor descriptor;
  descriptor.raw_data.resize(total_data_length);

  RETURN_IF_ERROR(
      GetDescriptor(UsbDeviceInterface::DescriptorType::kConfig, 0,
                    gtl::MutableArraySlice<uint8>(descriptor.raw_data.data(),
                                                  descriptor.raw_data.size()),
                    &num_bytes_transferred, __func__));

  if (num_bytes_transferred < kConfigDescriptorRawByteSize) {
    return util::UnknownError("Device descriptor is too short");
  }

  descriptor.raw_data.resize(num_bytes_transferred);

  descriptor.num_interfaces = descriptor.raw_data[4];
  descriptor.configuration_value = descriptor.raw_data[5];
  descriptor.configuration_name_index = descriptor.raw_data[6];
  const uint8_t attributes = descriptor.raw_data[7];
  descriptor.is_self_powered = (attributes >> 6) & 1;
  descriptor.supports_remote_wakeup = (attributes >> 5) & 1;
  descriptor.encoded_max_power = descriptor.raw_data[8];

  VLOG(7) << StringPrintf("Configuration requested: %d", index);
  VLOG(7) << StringPrintf("Configuration reported: %d",
                          descriptor.configuration_value);
  VLOG(7) << StringPrintf("Number of interfaces: %u",
                          descriptor.num_interfaces);
  VLOG(7) << StringPrintf("Is self powered: %d", descriptor.is_self_powered);
  VLOG(7) << StringPrintf("Supports remote wakeup: %d",
                          descriptor.supports_remote_wakeup);
  VLOG(7) << StringPrintf("Encoded max power: 0x%x",
                          descriptor.is_self_powered);
  VLOG(7) << StringPrintf("Raw data size: %d",
                          static_cast<uint32_t>(descriptor.raw_data.size()));

  return descriptor;
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
