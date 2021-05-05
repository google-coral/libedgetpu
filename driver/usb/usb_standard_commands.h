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

#ifndef DARWINN_DRIVER_USB_USB_STANDARD_COMMANDS_H_
#define DARWINN_DRIVER_USB_USB_STANDARD_COMMANDS_H_

#include "driver/usb/usb_device_interface.h"

namespace platforms {
namespace darwinn {
namespace driver {

// TODO provide a mechanism like locked/unlocked versions of functions
// and a busy state to prevent any interruption in the middle of a long sequence
// like firmware update.
class UsbStandardCommands {
 public:
  using ConstBuffer = UsbDeviceInterface::ConstBuffer;
  using MutableBuffer = UsbDeviceInterface::MutableBuffer;
  using DataInDone = UsbDeviceInterface::DataInDone;
  using DataOutDone = UsbDeviceInterface::DataOutDone;
  using CloseAction = UsbDeviceInterface::CloseAction;
  using TimeoutMillis = UsbDeviceInterface::TimeoutMillis;
  using SetupPacket = UsbDeviceInterface::SetupPacket;
  using CommandDataDir = UsbDeviceInterface::CommandDataDir;
  using CommandType = UsbDeviceInterface::CommandType;
  using CommandRecipient = UsbDeviceInterface::CommandRecipient;
  using DeviceSpeed = UsbDeviceInterface::DeviceSpeed;

  // Device descriptor can be retrieved from device by GetDeviceDescriptor.
  // Detailed meaning of each field can be found in USB spec.
  // Data in this structure determines how host would identify this device.
  struct DeviceDescriptor {
    // USB spec release number in BCD.
    uint16 usb_version_bcd;

    // Class of this device.
    UsbDeviceInterface::DeviceClass device_class;

    // Sub class of this device
    uint8 device_subclass;

    // Protocol this device speaks.
    uint8 bDeviceProtocol;

    // Packet size for endpoint 0
    uint8 max_packet_size_0;

    // Vendor ID.
    uint16 vendor_id;

    // Product ID.
    uint16 product_id;

    // Device release number in BCD.
    uint16 device_version_bcd;

    // Name of the manufacturer, as an index into string descriptors.
    uint8_t manufacturer_name_index;

    // Name of the product, as an index into string descriptors.
    uint8_t product_name_index;

    // Serial number in string, as an index into string descriptors.
    uint8_t serial_number_index;

    // Number of supported configurations.
    uint8 num_configurations;
  };

  // Configuration descriptor can be retrieved from device by
  // GetConfigurationDescriptor.
  // Detailed meaning of each field can be found in USB spec.
  struct ConfigurationDescriptor {
    // Number of interfaces supported by this configuration.
    uint8_t num_interfaces;

    // ID of this configuration, to be used in Set configuration.
    uint8_t configuration_value;

    // Name of this configuration, as an index into string descriptors.
    uint8_t configuration_name_index;

    // True if this device is self-powered, and hence doesn't need host to
    // provide any power.
    bool is_self_powered;

    // True if this device supports remote wake up feature.
    bool supports_remote_wakeup;

    // Max current could be drawn from host by this device. Note the encoding is
    // speed-specific.
    uint8_t encoded_max_power;

    // All descriptors for this configurations are returned, if allowed by
    // buffer size. Further parsing has to be done on this extra data to learn
    // about interfaces, endpoints, and other descriptors under this
    // configuration.
    std::vector<uint8_t> raw_data;
  };

  // Interface descriptor can be retrieved from device as by-product in
  // GetConfigurationDescriptor.
  // Detailed meaning of each field can be found in USB spec.
  struct InterfaceDescriptor {
    // ID of this interface to be used in set interface.
    uint8_t interface_number;

    // ID of alternate setting among several very similuar and mutural exclusing
    // interfaces.
    uint8_t alternate_setting;

    // Number of endpoints, other than the control endpoint, in this interface.
    uint8_t num_endpoints;

    // Classe code is defined by USB IF.
    uint8_t interface_class;

    // Sub-class code is defined by USB IF.
    uint8_t interface_subclass;

    // Protocol code is defined by USB IF.
    uint8_t interface_protocol;

    // Name of the interface, as an index into string descriptors.
    uint8_t interface_name_index;
  };

  UsbStandardCommands(std::unique_ptr<UsbDeviceInterface> device,
                      TimeoutMillis default_timeout_msec);

  // This class is neither copyable nor movable.
  UsbStandardCommands(const UsbStandardCommands&) = delete;
  UsbStandardCommands& operator=(const UsbStandardCommands&) = delete;

  virtual ~UsbStandardCommands();

  Status Close(CloseAction action) { return device_->Close(action); }

  Status SetConfiguration(int configuration) {
    return device_->SetConfiguration(configuration);
  }

  Status ClaimInterface(int interface_number) {
    return device_->ClaimInterface(interface_number);
  }

  Status ReleaseInterface(int interface_number) {
    return device_->ReleaseInterface(interface_number);
  }

  Status GetDescriptor(UsbDeviceInterface::DescriptorType desc_type,
                       uint8_t desc_index, MutableBuffer data_in,
                       size_t* num_bytes_transferred, const char* context) {
    // TODO add warnings/limitations on what can be queried, according
    // to USB 3 spec. Only device, config, string, and BOS types can be queried.
    // Only config and string types can have non-zero index specified.
    // Some devices do respond to more types, but this is device-specific.
    return device_->GetDescriptor(desc_type, desc_index, data_in,
                                  num_bytes_transferred, context);
  }

  DeviceSpeed GetDeviceSpeed() const { return device_->GetDeviceSpeed(); }

  Status SendControlCommand(const SetupPacket& command, const char* context) {
    return device_->SendControlCommand(command, default_timeout_msec_, context);
  }

  Status SendControlCommandWithDataOut(const SetupPacket& command,
                                       ConstBuffer data_out,
                                       const char* context) {
    return device_->SendControlCommandWithDataOut(
        command, data_out, default_timeout_msec_, context);
  }

  Status SendControlCommandWithDataIn(const SetupPacket& command,
                                      MutableBuffer data_in,
                                      size_t* num_bytes_transferred,
                                      const char* context) {
    return device_->SendControlCommandWithDataIn(
        command, data_in, num_bytes_transferred, default_timeout_msec_,
        context);
  }

  Status BulkOutTransfer(uint8_t endpoint, ConstBuffer data_out,
                         const char* context) {
    return device_->BulkOutTransfer(endpoint, data_out, default_timeout_msec_,
                                    context);
  }

  Status BulkInTransfer(uint8_t endpoint, MutableBuffer data_in,
                        size_t* num_bytes_transferred, const char* context) {
    return device_->BulkInTransfer(endpoint, data_in, num_bytes_transferred,
                                   default_timeout_msec_, context);
  }

  Status InterruptInTransfer(uint8_t endpoint, MutableBuffer data_in,
                             size_t* num_bytes_transferred,
                             const char* context) {
    return device_->InterruptInTransfer(endpoint, data_in,
                                        num_bytes_transferred,
                                        default_timeout_msec_, context);
  }

  Status AsyncBulkOutTransfer(uint8_t endpoint, ConstBuffer data_out,
                              DataOutDone callback, const char* context) {
    return device_->AsyncBulkOutTransfer(endpoint, data_out,
                                         default_timeout_msec_,
                                         std::move(callback), context);
  }

  Status AsyncBulkInTransfer(uint8_t endpoint, MutableBuffer data_in,
                             DataInDone callback, const char* context) {
    return device_->AsyncBulkInTransfer(
        endpoint, data_in, default_timeout_msec_, std::move(callback), context);
  }

  Status AsyncInterruptInTransfer(uint8_t endpoint, MutableBuffer data_in,
                                  DataInDone callback, const char* context) {
    return device_->AsyncInterruptInTransfer(
        endpoint, data_in, default_timeout_msec_, std::move(callback), context);
  }

  void TryCancelAllTransfers() { device_->TryCancelAllTransfers(); }

  StatusOr<MutableBuffer> AllocateTransferBuffer(size_t buffer_size) {
    return device_->AllocateTransferBuffer(buffer_size);
  }

  Status ReleaseTransferBuffer(MutableBuffer buffer) {
    return device_->ReleaseTransferBuffer(buffer);
  }

  uint8_t ComposeUsbRequestType(CommandDataDir dir, CommandType type,
                                CommandRecipient recipient) {
    return device_->ComposeUsbRequestType(dir, type, recipient);
  }

  // Retrieves device descriptor from device. In some implementations, a cached
  // one could be returned.
  StatusOr<DeviceDescriptor> GetDeviceDescriptor();

  // Retrieves configuration descriptor from device. In some implementations, a
  // cached one could be returned.
  StatusOr<ConfigurationDescriptor> GetConfigurationDescriptor(
      uint8_t index, size_t max_extra_data_length);

  TimeoutMillis GetDefaultTimeoutMillis() const {
    return default_timeout_msec_;
  }

 private:
  std::unique_ptr<UsbDeviceInterface> device_;
  const TimeoutMillis default_timeout_msec_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_USB_USB_STANDARD_COMMANDS_H_
