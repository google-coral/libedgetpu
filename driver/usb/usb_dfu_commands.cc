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

#include "driver/usb/usb_dfu_commands.h"

#include <algorithm>
#include <utility>

#include "driver/usb/usb_device_interface.h"
#include "driver/usb/usb_standard_commands.h"
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

UsbDfuCommands::UsbDfuCommands(std::unique_ptr<UsbDeviceInterface> device,
                               TimeoutMillis default_timeout_msec)
    : UsbStandardCommands(std::move(device), default_timeout_msec) {
  VLOG(10) << __func__;
}

UsbDfuCommands::~UsbDfuCommands() { VLOG(10) << __func__; }

util::Status UsbDfuCommands::DfuDetach(uint16_t timeout_msec) {
  VLOG(10) << __func__;
  StdMutexLock lock(&mutex_);
  SetupPacket command{
      // Request type (00100001b).
      ComposeUsbRequestType(CommandDataDir::kHostToDevice, CommandType::kClass,
                            CommandRecipient::kInterface),

      // Request id.
      static_cast<uint8_t>(RequestId::kDfuDetach),

      // Timeout in milliseconds.
      timeout_msec,

      // Interface number.
      dfu_interface_number_,

      // Data length.
      0};

  return SendControlCommand(command, __func__);
}

void UsbDfuCommands::SetDfuInterface(int interface_number) {
  StdMutexLock lock(&mutex_);
  dfu_interface_number_ = static_cast<uint16_t>(interface_number);
  VLOG(5) << StringPrintf("%s set to %u", __func__, dfu_interface_number_);
}

util::StatusOr<std::pair<std::list<UsbStandardCommands::InterfaceDescriptor>,
                         UsbDfuCommands::DfuFunctionalDescriptor>>
UsbDfuCommands::FindDfuInterfaces(
    const std::vector<uint8_t>& raw_configuration_descriptor) {
  constexpr size_t kConfigDescriptorRawByteSize = 9;
  constexpr size_t kInterfaceDescriptorRawByteSize = 9;
  if (raw_configuration_descriptor.size() < kConfigDescriptorRawByteSize) {
    return util::InvalidArgumentError("Raw data is way too short");
  }

  const uint8_t reported_config_type = raw_configuration_descriptor[1];
  if (reported_config_type !=
      static_cast<uint8_t>(UsbDeviceInterface::DescriptorType::kConfig)) {
    return util::InvalidArgumentError("Not reported as config descriptor");
  }

  const uint8_t reported_config_length = raw_configuration_descriptor[0];
  const uint8_t reported_total_data_length = raw_configuration_descriptor[2];
  if (reported_total_data_length > raw_configuration_descriptor.size()) {
    return util::InvalidArgumentError("Incomplete config descriptor");
  }
  // Every configuration must has at least one interface.
  if (reported_total_data_length <
      kConfigDescriptorRawByteSize + kInterfaceDescriptorRawByteSize) {
    return util::InvalidArgumentError("Reported total data is way too short");
  }

  bool found_dfu_functional_descriptor = false;
  std::list<InterfaceDescriptor> dfu_interfaces;
  DfuFunctionalDescriptor dfu_functional_descriptor;
  size_t cursor = reported_config_length;
  do {
    VLOG(10) << StringPrintf("%s cursor %u", __func__,
                             static_cast<uint32_t>(cursor));
    if ((cursor + 1) >= raw_configuration_descriptor.size()) break;
    const uint8_t length = raw_configuration_descriptor[cursor];
    const uint8_t type = raw_configuration_descriptor[cursor + 1];
    VLOG(10) << StringPrintf("%s type 0x%x, length %u", __func__, type, length);

    if (length == 0) {
      return util::FailedPreconditionError(
          "Length of functional descriptor must not be 0");
    }

    if (type ==
        static_cast<uint8_t>(UsbDeviceInterface::DescriptorType::kInterface)) {
      // Treat this as an interface descriptor.
      // Make sure we have valid access to the whole descriptor.
      if ((cursor + kInterfaceDescriptorRawByteSize - 1) >=
          raw_configuration_descriptor.size()) {
        break;
      }

      // TODO consider changing numbers to named constants.
      InterfaceDescriptor interface;
      interface.interface_number = raw_configuration_descriptor[cursor + 2];
      interface.alternate_setting = raw_configuration_descriptor[cursor + 3];
      interface.num_endpoints = raw_configuration_descriptor[cursor + 4];
      interface.interface_class = raw_configuration_descriptor[cursor + 5];
      interface.interface_subclass = raw_configuration_descriptor[cursor + 6];
      interface.interface_protocol = raw_configuration_descriptor[cursor + 7];
      interface.interface_name_index = raw_configuration_descriptor[cursor + 8];

      VLOG(10) << StringPrintf(
          "%s interface %d, alternate settings %u, num of extra endpoints %u, "
          "class 0x%x, subclass 0x%x",
          __func__, interface.interface_number, interface.alternate_setting,
          interface.num_endpoints, interface.interface_class,
          interface.interface_subclass);

      if ((interface.num_endpoints == 0) &&
          (interface.interface_class ==
           /*Application specific, see DFU spec 1.1*/ 0xFE) &&
          (interface.interface_subclass == /*DFU, see DFU spec 1.1*/ 1)) {
        dfu_interfaces.push_back(interface);
      }

    } else if (type ==
               static_cast<uint8_t>(
                   UsbDeviceInterface::DescriptorType::kDfuFunctional)) {
      // Make sure we have valid access to the whole descriptor.
      constexpr size_t kDfuFunctionalDescriptorRawByteSize = 9;
      if ((cursor + kDfuFunctionalDescriptorRawByteSize - 1) >=
          raw_configuration_descriptor.size())
        break;

      found_dfu_functional_descriptor = true;

      // TODO consider adding named constants for numbers used in
      // parsing.
      // Fill fields from raw bytes to more accessible data structure.
      // memcpy is used for all multi-byte data fields to avoid any potential
      // alignment issues.
      const uint8_t attributes = raw_configuration_descriptor[cursor + 2];
      dfu_functional_descriptor.will_detach =
          static_cast<bool>(attributes & 0x8);
      dfu_functional_descriptor.manifestation_tolerant =
          static_cast<bool>(attributes & 0x4);
      dfu_functional_descriptor.can_upload =
          static_cast<bool>(attributes & 0x2);
      dfu_functional_descriptor.can_download =
          static_cast<bool>(attributes & 0x1);
      memcpy(&dfu_functional_descriptor.detach_timeout_msec,
             &raw_configuration_descriptor[cursor + 3], 2);
      memcpy(&dfu_functional_descriptor.transfer_size,
             &raw_configuration_descriptor[cursor + 5], 2);
      memcpy(&dfu_functional_descriptor.dfu_version_bcd,
             &raw_configuration_descriptor[cursor + 7], 2);

      VLOG(7) << StringPrintf("Will detach: %d, manifestation tolerant: %d",
                              dfu_functional_descriptor.will_detach,
                              dfu_functional_descriptor.manifestation_tolerant);
      VLOG(7) << StringPrintf("Can upload: %d, can download: %d",
                              dfu_functional_descriptor.can_upload,
                              dfu_functional_descriptor.can_download);
      VLOG(7) << StringPrintf("Transfer Size: 0x%x",
                              dfu_functional_descriptor.transfer_size);
      VLOG(7) << StringPrintf("Detach Timeout: 0x%x",
                              dfu_functional_descriptor.detach_timeout_msec);
      VLOG(7) << StringPrintf("DFU version in BCD: 0x%x",
                              dfu_functional_descriptor.dfu_version_bcd);
    } else {
      // Skip unrecognized entries.
    }
    cursor += length;
  } while (true);

  if ((!dfu_interfaces.empty()) && found_dfu_functional_descriptor) {
    return std::make_pair(std::move(dfu_interfaces), dfu_functional_descriptor);
  }

  return util::NotFoundError(__func__);
}

util::StatusOr<UsbDfuCommands::DfuStatus> UsbDfuCommands::DfuGetStatus() {
  VLOG(10) << __func__;
  StdMutexLock lock(&mutex_);
  constexpr size_t kGetStatusRawByteSize = 6;
  uint8_t buffer[kGetStatusRawByteSize] = {0};
  SetupPacket command{
      // Request type (10100001b).
      ComposeUsbRequestType(CommandDataDir::kDeviceToHost, CommandType::kClass,
                            CommandRecipient::kInterface),

      // Request id.
      static_cast<uint8_t>(RequestId::kDfuGetStatus),

      // Value is not used.
      0,

      // Interface number.
      dfu_interface_number_,

      // Data length.
      sizeof(buffer)};

  size_t num_bytes_transferred = 0;
  RETURN_IF_ERROR(SendControlCommandWithDataIn(
      command, MutableBuffer(buffer, sizeof(buffer)), &num_bytes_transferred,
      __func__));

  if (num_bytes_transferred != sizeof(buffer)) {
    return util::UnknownError("Invalid DFU status data");
  }

  DfuStatus dfu_status;

  // Initialize all fields to 0. The common case {0} doesn't work here, as
  // scoped enum doesn't have implicit conversion.
  memset(&dfu_status, 0, sizeof(dfu_status));

  // Fill fields from raw bytes to more accessible data structure.
  // memcpy is used for all multi-byte data fields to avoid any potential
  // alignment issues.
  dfu_status.previous_result = static_cast<Error>(buffer[0]);

  // TODO consider adding named constants for numbers used.
  // Note this field is only 3-byte-long. Since we assume everything is little
  // endian, only the first 3 bytes on the target are overwritten.
  memcpy(&dfu_status.poll_timeout_msec, &buffer[1], 3);

  dfu_status.state = static_cast<State>(buffer[4]);
  dfu_status.status_string_index = buffer[5];

  VLOG(7) << StringPrintf("Previous result: %d",
                          static_cast<int>(dfu_status.previous_result));
  VLOG(7) << StringPrintf("Poll timeout: %d", dfu_status.poll_timeout_msec);
  VLOG(7) << StringPrintf("State: %d", static_cast<int>(dfu_status.state));
  VLOG(7) << StringPrintf("Status string index: %d",
                          dfu_status.status_string_index);

  return dfu_status;
}

util::Status UsbDfuCommands::DfuClearStatus() {
  VLOG(10) << __func__;
  StdMutexLock lock(&mutex_);
  SetupPacket command{
      // Request type (00100001b).
      ComposeUsbRequestType(CommandDataDir::kHostToDevice, CommandType::kClass,
                            CommandRecipient::kInterface),

      // Request id.
      static_cast<uint8_t>(RequestId::kDfuClearStatus),

      // Value is not used.
      0,

      // Interface number.
      dfu_interface_number_,

      // Data length.
      0};

  return SendControlCommand(command, __func__);
}

util::Status UsbDfuCommands::DfuAbort() {
  VLOG(10) << __func__;
  StdMutexLock lock(&mutex_);
  SetupPacket command{
      // Request type (00100001b).
      ComposeUsbRequestType(CommandDataDir::kHostToDevice, CommandType::kClass,
                            CommandRecipient::kInterface),

      // Request id.
      static_cast<uint8_t>(RequestId::kDfuAbort),

      // Value is not used.
      0,

      // Interface number.
      dfu_interface_number_,

      // Data length.
      0};

  return SendControlCommand(command, __func__);
}

util::StatusOr<UsbDfuCommands::State> UsbDfuCommands::DfuGetState() {
  VLOG(10) << __func__;
  StdMutexLock lock(&mutex_);
  constexpr size_t kGetStateRawByteSize = 1;
  uint8_t buffer[kGetStateRawByteSize] = {0};
  SetupPacket command{
      // Request type (10100001b).
      ComposeUsbRequestType(CommandDataDir::kDeviceToHost, CommandType::kClass,
                            CommandRecipient::kInterface),

      // Request id.
      static_cast<uint8_t>(RequestId::kDfuGetState),

      // Value is not used.
      0,

      // Interface number.
      dfu_interface_number_,

      // Data length.
      sizeof(buffer)};

  size_t num_bytes_transferred = 0;
  RETURN_IF_ERROR(SendControlCommandWithDataIn(
      command, MutableBuffer(buffer, sizeof(buffer)), &num_bytes_transferred,
      __func__));

  if (num_bytes_transferred != sizeof(buffer)) {
    return util::UnknownError("Invalid DFU state data");
  }

  State dfu_state = static_cast<State>(buffer[0]);

  VLOG(7) << StringPrintf("State: %d", static_cast<int>(dfu_state));

  return dfu_state;
}

util::Status UsbDfuCommands::DfuDownloadBlock(uint16_t block_number,
                                              ConstBuffer block_buffer) {
  VLOG(10) << StringPrintf("%s block %u, request size %u", __func__,
                           block_number,
                           static_cast<uint32_t>(block_buffer.size()));

  StdMutexLock lock(&mutex_);

  SetupPacket command{
      // Request type (10100001b).
      ComposeUsbRequestType(CommandDataDir::kHostToDevice, CommandType::kClass,
                            CommandRecipient::kInterface),

      // Request id.
      static_cast<uint8_t>(RequestId::kDfuDownload),

      // Block number.
      block_number,

      // Interface number.
      dfu_interface_number_,

      // Data length.
      static_cast<uint16_t>(block_buffer.size())};

  return SendControlCommandWithDataOut(command, block_buffer, __func__);
}

util::Status UsbDfuCommands::DfuUploadBlock(uint16_t block_number,
                                            MutableBuffer block_buffer,
                                            size_t* num_bytes_transferred) {
  VLOG(10) << StringPrintf("%s block %u, request size %u", __func__,
                           block_number,
                           static_cast<uint32_t>(block_buffer.size()));

  StdMutexLock lock(&mutex_);

  SetupPacket command{
      // Request type (10100001b).
      ComposeUsbRequestType(CommandDataDir::kDeviceToHost, CommandType::kClass,
                            CommandRecipient::kInterface),

      // Request id.
      static_cast<uint8_t>(RequestId::kDfuUpload),

      // Block number.
      block_number,

      // Interface number.
      dfu_interface_number_,

      // Data length.
      static_cast<uint16_t>(block_buffer.size())};

  // command.length = static_cast<uint16_t>(block_buffer.size());

  return SendControlCommandWithDataIn(command, block_buffer,
                                      num_bytes_transferred, __func__);
}

util::Status UsbDfuCommands::UpdateFirmware(
    const DfuFunctionalDescriptor& descriptor, ConstBuffer firmware_image) {
  VLOG(7) << StringPrintf("%s Downloading firmware", __func__);

  if (firmware_image.empty()) {
    return util::InvalidArgumentError("Invalid DFU image file");
  }

  VLOG(7) << StringPrintf("%s Firmware image size %zu bytes", __func__,
                          firmware_image.size());

  // TODO: try DFU abort or clear status to clear the stage, if we're
  // not in DFU idle state.

  uint16 block_number = 0;
  size_t num_bytes_transferred = 0;
  bool last_packet_sent = false;
  while (num_bytes_transferred <= firmware_image.size()) {
    const uint16 transfer_size = static_cast<uint16>(
        std::min(static_cast<size_t>(descriptor.transfer_size),
                 firmware_image.size() - num_bytes_transferred));

    if (transfer_size == 0) {
      VLOG(8) << StringPrintf("%s Sending the final zero-length packet",
                              __func__);
    } else {
      VLOG(8) << StringPrintf(
          "%s Transfer size %u bytes, already transferred %zu bytes", __func__,
          transfer_size, num_bytes_transferred);
    }

    RETURN_IF_ERROR(DfuDownloadBlock(
        block_number,
        ConstBuffer(firmware_image, num_bytes_transferred, transfer_size)));

    auto dfu_status_query_result = DfuGetStatus();
    RETURN_IF_ERROR(dfu_status_query_result.status());
    auto dfu_status = dfu_status_query_result.ValueOrDie();

    VLOG(8) << StringPrintf("%s: block %d status:%d, state:%d", __func__,
                            block_number,
                            static_cast<int>(dfu_status.previous_result),
                            static_cast<int>(dfu_status.state));

    if ((Error::kOK == dfu_status.previous_result) &&
        (State::kDownloadIdle == dfu_status.state)) {
      // keep track of accumulated data
      num_bytes_transferred += transfer_size;
    } else if ((0 == transfer_size) &&
               (Error::kOK == dfu_status.previous_result) &&
               (State::kDfuIdle == dfu_status.state)) {
      // The last packet sent has zero length. Downloading is done.
      last_packet_sent = true;
      break;
    } else {
      VLOG(8) << StringPrintf("%s: download failed", __func__);
      break;
    }
    // block number could wrap around
    ++block_number;
  }

  VLOG(7) << StringPrintf("%s, transferred image size: %zu, EOF: %d", __func__,
                          num_bytes_transferred,
                          (firmware_image.size() == num_bytes_transferred));

  if (last_packet_sent) {
    return util::Status();  // OK.
  }

  return util::DataLossError("Firmware downloading failed");
}

util::Status UsbDfuCommands::ValidateFirmware(
    const DfuFunctionalDescriptor& descriptor, ConstBuffer firmware_image) {
  VLOG(7) << StringPrintf("%s Validating firmware", __func__);

  uint16_t block_number = 0;
  bool short_packet_received = false;
  std::vector<uint8_t> upload_image;
  upload_image.reserve(firmware_image.size());
  std::vector<uint8_t> chunk_buffer(descriptor.transfer_size);
  while (true) {
    // Always asks for max transfer size.
    const uint16 transfer_size = static_cast<uint16>(descriptor.transfer_size);

    VLOG(10) << StringPrintf("%s Reading firmware block %d", __func__,
                             block_number);

    size_t chunk_bytes_transferred = 0;
    RETURN_IF_ERROR(DfuUploadBlock(
        block_number, MutableBuffer(chunk_buffer.data(), chunk_buffer.size()),
        &chunk_bytes_transferred));

    upload_image.insert(upload_image.end(), chunk_buffer.begin(),
                        chunk_buffer.begin() + chunk_bytes_transferred);

    if (chunk_bytes_transferred < transfer_size) {
      // A short packet! Upload is done.
      short_packet_received = true;
      break;
    }
    // block number could wrap around.
    ++block_number;
  }

  VLOG(7) << StringPrintf("%s, Uploaded image size: %zu", __func__,
                          upload_image.size());

  if (upload_image.size() < firmware_image.size()) {
    VLOG(1) << StringPrintf("%s, Uploaded image is shorter than expected",
                            __func__);
    return util::DataLossError(__func__);
  }

  // Only compares the first part of uploaded image, for it's possible for
  // the uploaded images to be longer than the reference image.
  if (0 == memcmp(upload_image.data(), firmware_image.data(),
                  firmware_image.size())) {
    return util::Status();  // OK.
  }

  VLOG(1) << StringPrintf("%s, Uploaded image is different from expected",
                          __func__);

  return util::DataLossError(__func__);
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
