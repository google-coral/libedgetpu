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

#ifndef DARWINN_DRIVER_USB_USB_DFU_COMMANDS_H_
#define DARWINN_DRIVER_USB_USB_DFU_COMMANDS_H_

#include <list>
#include <mutex>  // NOLINT

#include "driver/usb/usb_device_interface.h"
#include "driver/usb/usb_standard_commands.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Thread-safe implementation of USB Device Firmare Update protocol.
// Note thread-safety doesn't mean much here, as the device cannot respond
// to interferences properly during DFU process.
// TODO provide a mechanism like locked/unlocked versions of functions
// and a busy state to prevent any interruption in the middle of a long sequence
// like firmware update.
class UsbDfuCommands : public UsbStandardCommands {
 public:
  // Detailed definition of these request IDs can be found in DFU spec.
  enum class RequestId {
    // Pushes a device into app detached state.
    kDfuDetach = 0,

    // Sends one chunk of firmware to device.
    kDfuDownload = 1,

    // Retrieves one chunk of firmware from device.
    kDfuUpload = 2,

    // Retrieves DFU status from device.
    kDfuGetStatus = 3,

    // Clears error status in DFU mode.
    kDfuClearStatus = 4,

    // Retrieves DFU state without affecting the state.
    kDfuGetState = 5,

    // Aborts current DFU operation.
    kDfuAbort = 6,
  };

  // Detail definition of these states can be found in DFU spec v1.1.
  enum class State {
    // Normal/idle in application mode.
    kAppIdle = 0,

    // Detached in application mode, waiting for USB reset to enter DFU mode.
    kAppDetach = 1,

    // Normal/idle in DFU mode.
    kDfuIdle = 2,

    // Downloading in DFU mode, waiting for GetStatus.
    kDownloadSync = 3,

    // Downloading in DFU mode, blocking further GetStatus.
    kDownloadBusy = 4,

    // Downloading in DFU mode, waiting for the next packet.
    kDownloadIdle = 5,

    // Programming in DFU mode, waiting for the last GetStatus to begin
    // manifest phase.
    kManifestSync = 6,

    // Programming in DFU mode.
    kManifest = 7,

    // Programming in DFU mode, waiting for the USB reset to leave DFU mode.
    kManifestWaitReset = 8,

    // Uploading in DFU mode, waiting for the next DfuUpload.
    kUploadIdle = 9,

    // Error state in DFU mode, waiting for ClearStatus.
    kError = 10,
  };

  // Detail definition of these error codes can be found in DFU spec v1.1.
  enum class Error {
    // No error.
    kOK = 0,

    // File is not targeted for this device.
    kWrongTarget = 1,

    // Vendor-specific verification failed.
    kFileVerifyFailed = 2,

    // Write memory failed.
    kWriteFailed = 3,

    // Erase memory failed.
    kEraseFailed = 4,

    // Check failed for erasing memory.
    kEraseCheckFailed = 5,

    // Program memory failed.
    kProgramFailed = 6,

    // Check failed for programming memory.
    kProgramVerifyFailed = 7,

    // Program failed because of address is invalid.
    kInvalidAddress = 8,

    // The downloaded firmware image doesn't seem to be long enough.
    kInsufficientData = 9,

    // The firmware is corrupted.
    kFirmwareIsCorrupt = 10,

    // Vendor-specific error.
    kVendorSpecificError = 11,

    // Unexpected USB reset detected.
    kUnexpectedUsbResetDetected = 12,

    // Unexpected POR detected.
    kUnexpectedPowerOnResetDetected = 13,

    // Unknown error.
    kUnknownError = 14,

    // Unexpected request.
    kUnexpectedRequestStalled = 15,
  };

  // Current device status returned by DFU GetStatus command.
  // Detailed definition of these fields can be found in DFU spec v1.1.
  struct DfuStatus {
    // Status before executing this GetStatus command.
    Error previous_result;

    // Minimum time the host should wait before a subsequent GetStatus command.
    // Valid range is 0-0xFFFFFF only
    uint32_t poll_timeout_msec;

    // State after executing this GetStatus command.
    State state;

    // Index of status description in the string table.
    uint8_t status_string_index;
  };

  // Functional descriptor for DFU funtction.
  // Detailed definition of these fields can be found in DFU spec v1.1.
  struct DfuFunctionalDescriptor {
    // True if host must not send USB reset after DFU_DETACH command.
    bool will_detach;

    // True if device goes back to DFU Idle after manifestation.
    bool manifestation_tolerant;

    // True if device can upload firmware image to host.
    bool can_upload;

    // True if device can download firmware image from host.
    bool can_download;

    // Max time, in msec, before device returns to App Idle mode from App
    // Detach.
    uint16_t detach_timeout_msec;

    // Max number of bytes in each control read/write request.
    // This number should be larger than max packet size for ep 0.
    uint16_t transfer_size;

    // DFU version supported in BCD. This field must be at least 0100h.
    uint16_t dfu_version_bcd;
  };

  // Constructs a new object from pointer to an USB device.
  UsbDfuCommands(std::unique_ptr<UsbDeviceInterface> device,
                 TimeoutMillis default_timeout_msec);

  // This class is neither copyable but movable.
  UsbDfuCommands(const UsbDfuCommands&) = delete;
  UsbDfuCommands& operator=(const UsbDfuCommands&) = delete;

  ~UsbDfuCommands() override;

  // Gets DFU functional descriptor from device.
  util::StatusOr<
      std::pair<std::list<InterfaceDescriptor>, DfuFunctionalDescriptor>>
  FindDfuInterfaces(const std::vector<uint8_t>& raw_configuration_descriptor);

  // Sets the target interface number for DFU interface-specific commands,
  // including DfuGetStatus, DfuClearStatus, DfuAbort,DfuGetState,
  // DfuDownloadBlock, and DfuUploadBlock.
  // Only the low 16-bit is used, as per USB spec.
  void SetDfuInterface(int interface_number) LOCKS_EXCLUDED(mutex_);

  // Detaches from application mode.
  util::Status DfuDetach(uint16_t timeout_msec);

  // Retrieves DFU status from device.
  util::StatusOr<DfuStatus> DfuGetStatus() LOCKS_EXCLUDED(mutex_);

  // Clears error status in DFU mode.
  util::Status DfuClearStatus() LOCKS_EXCLUDED(mutex_);

  // Aborts current DFU operation.
  util::Status DfuAbort() LOCKS_EXCLUDED(mutex_);

  // Retrieves DFU state from device without affecting the virtual state.
  util::StatusOr<State> DfuGetState() LOCKS_EXCLUDED(mutex_);

  // Downloads a block of firmware from host to device.
  util::Status DfuDownloadBlock(uint16_t block_number, ConstBuffer block_buffer)
      LOCKS_EXCLUDED(mutex_);

  // Uploads a block of firmware frmo device to host.
  util::Status DfuUploadBlock(uint16_t block_number, MutableBuffer block_buffer,
                              size_t* num_bytes_transferred)
      LOCKS_EXCLUDED(mutex_);

  util::Status UpdateFirmware(const DfuFunctionalDescriptor& descriptor,
                              ConstBuffer firmware_image)
      LOCKS_EXCLUDED(mutex_);

  util::Status ValidateFirmware(const DfuFunctionalDescriptor& descriptor,
                                ConstBuffer firmware_image)
      LOCKS_EXCLUDED(mutex_);

 private:
  // Serializes access to this interface and hence shared data.
  mutable std::mutex mutex_;

  uint16_t dfu_interface_number_ GUARDED_BY(mutex_){0};
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_USB_USB_DFU_COMMANDS_H_
