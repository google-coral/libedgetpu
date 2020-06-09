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

#ifndef DARWINN_DRIVER_USB_USB_DEVICE_INTERFACE_H_
#define DARWINN_DRIVER_USB_USB_DEVICE_INTERFACE_H_

#include "port/array_slice.h"
#include "port/integral_types.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {
namespace driver {

// This interface abstracts away access to a USB device.
// All operations listed here are primitive, and hence can be trust to be
// atomic/thread-safe in implementation. However, combinations of these
// primitive operations might have to be protected at higher level.
class UsbDeviceInterface {
 public:
  // Configuration number to be used in set configuration command.
  enum ConfigurationNumber {
    kFirstDeviceConfiguration = 1,
    kResetDeviceConfiguration = -1,
  };

  // Device class, as defined in USB spec.
  enum class DeviceClass {
    // Use class information in the Interface descriptors.
    kPerInterface = 0,
    // Vendor-specific class.
    kVendorSpecific = 0xff,
  };

  enum class DeviceSpeed {
    // The implementation, probably a remote USB device,
    // doesn't know the current USB speed.
    kUnknown = 0,

    // Low speed is 1.5 Mbit/sec, defined in USB 1.0
    kLow = 1,
    // Full speed is 11 Mbit/sec, defined in USB 1.0
    kFull = 2,

    // High speed is 480 Mbit/sec, defined in USB 2.0
    kHigh = 3,

    // Superspeed is 5 Gbit/sec, defined in USB 3.0.
    kSuper = 4,

    // Superspeed+ is 10 Gbit/sec, defined in USB 3.1.
    kSuperPlus = 5,
  };

  // Descriptor type, as defined in USB spec.
  enum class DescriptorType {
    // Device descriptor.
    kDevice = 1,
    // Configuration descriptor.
    kConfig = 2,
    // String descriptor.
    kString = 3,
    // Interface descriptor.
    kInterface = 4,
    // Endpoint descriptor.
    kEndpoint = 5,
    // Device qualifier.
    kDeviceQualifier = 6,
    // Other speed configuration.
    kOtherSpeedConfiguration = 7,
    // BOS descriptor.
    kBos = 0xf,
    // Device capability descriptor.
    kDeviceCapability = 0x10,
    // DFU functional descriptor.
    kDfuFunctional = 0x21,
    // Super speed endpoint companion descriptor.
    kSuperSpeedEndpointCompanion = 0x30,
  };

  // Detailed definition of this enum can be found in USB spec.
  // Used in specifying the request type in setup packet.
  enum class CommandDataDir {
    // Data, if present in this command, flows from host to device
    kHostToDevice = 0,
    // Data, if present in this command, flows from device to host
    kDeviceToHost = 1,
  };

  // Detailed definition of this enum can be found in USB spec.
  // Used in specifying the request type in setup packet.
  enum class CommandType {
    // This is part of the standard command set every USB device has to support.
    kStandard = 0,
    // This is a class-specific command.
    kClass = 1,
    // This is a vendor-specific command.
    kVendor = 2,
  };

  // Detailed definition of this enum can be found in USB spec.
  // Used in specifying the request type in setup packet.
  enum class CommandRecipient {
    // The recipient of this command is the whole device.
    kDevice = 0,
    // The recipient of this command is the specified interface.
    kInterface = 1,
    // The recipient of this command is the specified endpoint.
    kEndpoint = 2,
    // The recipient of this command is none of the above.
    kOther = 3,
  };

  // Setup packet is used in all commands sent over control endpoint 0.
  // Detailed definition can be found in USB spec. Definition of most fields are
  // command-specific, so the comments here only provide the general concept.
  struct SetupPacket {
    // Type, direction, and recipient of this command.
    uint8_t request_type;
    // The actual request ID.
    uint8_t request;
    // General field used to carry parameter in this command.
    uint16_t value;
    // Usually used to specify the interface number for this command.
    uint16_t index;
    // The amount of data, in number of bytes, for data phase associated with
    // this command.
    uint16_t length;
  };

  // Options available when closing the device.
  enum class CloseAction {
    // Closes the device. The same device can be opened right away.
    kNoReset = 0,

    // Performs USB port reset before closing the device. USB bus re-enumeration
    // could take some time, before the same, or a different device can be
    // opened again.
    kGracefulPortReset,

    // Perform emergency USB port reset without first releasing all interfaces,
    // and then closes the device. Some resource leak or incompatibility
    // with underlying OS could arise.
    kForcefulPortReset,

    // Performs chip reset before closing the device. USB bus re-enumeration
    // could take some time, before the same, or a different device can be
    // opened again. Locally connected devices perform kPortReset instead.
    kGracefulChipReset,

    // Perform emergency whole chip reset without first releasing all
    // interfaces, and then closes the device. Some resource leak or
    // incompatibility with underlying OS could arise.
    kForcefulChipReset,
  };

  // Completion callback made when data in has been completed.
  // Note that short data transfer is not considered an error, so application
  // must check the amount received.
  // This callback receives two arguments, the number of bytes transferred, and
  // resulting status of the data in request.
  using DataInDone = std::function<void(util::Status, size_t)>;

  // Completion callback made when data out has been completed.
  // Note that short data transfer is considered an error.
  using DataOutDone = std::function<void(util::Status)>;

  // Used to specify timeout, in number of milliseconds.
  using TimeoutMillis = int;

  // Constant buffer, which is used to send data to device (USB OUT).
  using ConstBuffer = gtl::ArraySlice<uint8_t>;

  // Mutable buffer, which is used for receiving data from device (USB IN).
  using MutableBuffer = gtl::MutableArraySlice<uint8_t>;

  // Timeout, in milliseconds, to be used in USB operations.
  enum TimeoutSpec : TimeoutMillis {
    kDoNotRetry = 0,
    kTimeoutOneSecond = 1000,
  };

  UsbDeviceInterface() = default;

  // This class is neither copyable nor movable.
  UsbDeviceInterface(const UsbDeviceInterface&) = delete;
  UsbDeviceInterface& operator=(const UsbDeviceInterface&) = delete;

  virtual ~UsbDeviceInterface() = default;

  // Closes the device and releases all associated resources.
  virtual util::Status Close(CloseAction action) = 0;

  // Sets the active configuration. Application must not assume a default
  // configuration is already being set to active. Valid configuration starts
  // with 1. Setting configuration to -1 would set the device into unconfigured
  // state. All claimed interfaces must be released before one can change or
  // reset configuration.
  virtual util::Status SetConfiguration(int configuration) = 0;

  // Notifies underlying OS that this application intends to use this interface
  // in current configuration.
  virtual util::Status ClaimInterface(int interface_number) = 0;

  // Releases ownership of this interface in current configuration.
  virtual util::Status ReleaseInterface(int interface_number) = 0;

  // Retrieves the specified descriptor from device.
  virtual util::Status GetDescriptor(DescriptorType desc_type,
                                     uint8_t desc_index, MutableBuffer data_in,
                                     size_t* num_bytes_transferred,
                                     const char* context) = 0;

  virtual DeviceSpeed GetDeviceSpeed() const { return DeviceSpeed::kUnknown; }

  // Composes request type in setup packet for USB commands.
  // This is an utility function for subclasses to compose setup packets.
  static uint8_t ComposeUsbRequestType(CommandDataDir dir, CommandType type,
                                       CommandRecipient recipient) {
    constexpr int DataDirBitShift = 7;
    constexpr int TypeBitShift = 5;
    return (static_cast<uint8_t>(dir) << DataDirBitShift) |
           (static_cast<uint8_t>(type) << TypeBitShift) |
           static_cast<uint8_t>(recipient);
  }

  // Sets control command over endpoint 0, with no data phase
  virtual util::Status SendControlCommand(const SetupPacket& command,
                                          TimeoutMillis timeout_msec,
                                          const char* context) = 0;

  // Sets control command over endpoint 0, with data out.
  virtual util::Status SendControlCommandWithDataOut(const SetupPacket& command,
                                                     ConstBuffer data_out,
                                                     TimeoutMillis timeout_msec,
                                                     const char* context) = 0;

  // Sets control command over endpoint 0, with data in.
  virtual util::Status SendControlCommandWithDataIn(
      const SetupPacket& command, MutableBuffer data_in,
      size_t* num_bytes_transferred, TimeoutMillis timeout_msec,
      const char* context) = 0;

  // Transfers data on the specified bulk out endpoint.
  // This function returns after the bulk out has been done. Short transfer
  // is considered as an error.
  virtual util::Status BulkOutTransfer(uint8_t endpoint, ConstBuffer data_out,
                                       TimeoutMillis timeout_msec,
                                       const char* context) = 0;

  // Transfers data on the specified bulk in endpoint.
  // This function returns after the bulk in has been done. Short transfer
  // is expected and the number of bytes transferred is returned through
  // *num_bytes_transferred.
  virtual util::Status BulkInTransfer(uint8_t endpoint, MutableBuffer data_in,
                                      size_t* num_bytes_transferred,
                                      TimeoutMillis timeout_msec,
                                      const char* context) = 0;

  // Transfers data on the specified interrupt in endpoint.
  // This function returns after the interrupt in has been done. Short transfer
  // is expected and the number of bytes transferred is returned through
  // *num_bytes_transferred.
  virtual util::Status InterruptInTransfer(uint8_t endpoint,
                                           MutableBuffer data_in,
                                           size_t* num_bytes_transferred,
                                           TimeoutMillis timeout_msec,
                                           const char* context) = 0;

  // Transfers data on the specified bulk out endpoint.
  // This function returns immediately after the data buffer is submitted into
  // lower layer. A callback will be made, most probably from another thread,
  // after the actual transfer is done.
  virtual util::Status AsyncBulkOutTransfer(uint8_t endpoint,
                                            ConstBuffer data_out,
                                            TimeoutMillis timeout_msec,
                                            DataOutDone callback,
                                            const char* context) = 0;

  // Transfers data on the specified bulk in endpoint.
  // This function returns immediately after the data buffer is submitted into
  // lower layer. A callback will be made, most probably from another thread,
  // after the actual transfer is done.
  virtual util::Status AsyncBulkInTransfer(uint8_t endpoint,
                                           MutableBuffer data_in,
                                           TimeoutMillis timeout_msec,
                                           DataInDone callback,
                                           const char* context) = 0;

  // Transfers data on the specified interrupt in endpoint.
  // This function returns immediately after the data buffer is submitted into
  // lower layer. A callback will be made, most probably from another thread,
  // after the actual transfer is done.
  virtual util::Status AsyncInterruptInTransfer(uint8_t endpoint,
                                                MutableBuffer data_in,
                                                TimeoutMillis timeout_msec,
                                                DataInDone callback,
                                                const char* context) = 0;

  // Cancels all current transfers. This is a best-effort request.
  virtual void TryCancelAllTransfers() = 0;

  // Allocates transfer buffer for subsequent data transfer.
  // This is only useful in locally connected cases, and only if the underlying
  // libusb and OS both support zero-copy on USB data transfer.
  // If supported the amount of memory available could be limited by USB driver
  // in kernel space.
  // If not supported, the allocation would still be emulated in user space,
  // and data might have to be copied between user and kernel space.
  virtual util::StatusOr<MutableBuffer> AllocateTransferBuffer(
      size_t buffer_size) = 0;

  // Releases transfer buffer previously allocated.
  // CloseDevice automatically releases all transfer buffers associated with the
  // device.
  virtual util::Status ReleaseTransferBuffer(MutableBuffer buffer) = 0;
};

// This interface abstracts the enumeration for connected USB devices.
//
// It is possible to connect to more than one devices at the same time, which
// could be interesting in some use cases. Extensions have to be
// made to the OpenDevice family of API to open multiple devices with the same
// vendor and product IDs.
class UsbManager {
 public:
  // Used to specify timeout, in number of milliseconds.
  using TimeoutMillis = int;

  UsbManager() = default;

  // This class is neither copyable nor movable.
  UsbManager(const UsbManager&) = delete;
  UsbManager& operator=(const UsbManager&) = delete;

  virtual ~UsbManager() = default;

  // Opens a new device and returns an instance of UsbDeviceInterface for that
  // discovered device. If there are multiple of them connected, only the first
  // one is opened. Automatic retry could be made within timeout limit.
  // timeout_msec could be 0, which means no retry is allowed.
  // Negative timeout_msec, if supported, means unlimited/very long timeout and
  // retry.
  virtual util::StatusOr<std::unique_ptr<UsbDeviceInterface>> OpenDevice(
      uint16_t vendor_id, uint16_t product_id, TimeoutMillis timeout_msec) = 0;

  // Equivalent to OpenDevice() above, but without product_id specifier. The
  // first device found with the specified vendor ID is opened, regardless of
  // the product ID. Order of bus enumeration through this API is unreliable,
  // and hence it's not guaranteed to open the same device everytime if more
  // than one devices of the same vendor ID are present.
  virtual util::StatusOr<std::unique_ptr<UsbDeviceInterface>> OpenDevice(
      uint16_t vendor_id, TimeoutMillis timeout_msec) = 0;

  static constexpr TimeoutMillis kDoNotRetry = 0;
};

// Factory class to produce path strings for devices connected to USB,
// and create device objects from the path strings. Thread-safe.
class UsbDeviceFactory {
 public:
  // Used to specify timeout, in number of milliseconds.
  using TimeoutMillis = int;

  UsbDeviceFactory() = default;

  virtual ~UsbDeviceFactory() = default;

  // This class is neither copyable nor movable.
  UsbDeviceFactory(const UsbDeviceFactory&) = delete;
  UsbDeviceFactory& operator=(const UsbDeviceFactory&) = delete;

  // On success, returns a vector of strings for all connected USB devices
  // matching the vendor and product ID specified. The strings are
  // system-specific, but not limited to a particular factory instance.
  virtual util::StatusOr<std::vector<std::string>> EnumerateDevices(
      uint16_t vendor_id, uint16_t product_id) = 0;

  // Creates object implementing UsbDeviceInterface from the specified path
  // string. The timeout is meant for the enumerating and opening operation.
  virtual util::StatusOr<std::unique_ptr<UsbDeviceInterface>> OpenDevice(
      const std::string& path, TimeoutMillis timeout_msec) = 0;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_USB_USB_DEVICE_INTERFACE_H_
