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

#ifndef DARWINN_DRIVER_USB_LOCAL_USB_DEVICE_H_
#define DARWINN_DRIVER_USB_LOCAL_USB_DEVICE_H_

#include <atomic>              // NOLINT
#include <condition_variable>  // NOLINT
#include <map>
#include <mutex>               // NOLINT
#include <thread>              // NOLINT
#include <unordered_set>

#include "driver/usb/usb_device_interface.h"
#include "port/array_slice.h"
#include "port/defs.h"
#include "port/integral_types.h"
#include "port/statusor.h"
#include "port/thread_annotations.h"
#if DARWINN_PORT_USE_EXTERNAL
#include "libusb/libusb.h"
#else  // !DARWINN_PORT_USE_EXTERNAL
#include <libusb-1.0/libusb.h>
#endif  // DARWINN_PORT_USE_EXTERNAL

// TODO: upgrade this to allow the latest version of libusb to be used.
// We need at least 1.0.21 (LIBUSB_API_VERSION >= 0x01000105) for zero copy
// feature.

namespace platforms {
namespace darwinn {
namespace driver {

// Thread-safe implementation of UsbDeviceInterface on top of libusb.
//
// Async USB transfer functions are still async, but all other
// functions are serialized.
class LocalUsbDevice : public UsbDeviceInterface {
 public:
  // This class is neither copyable nor movable.
  LocalUsbDevice(const LocalUsbDevice&) = delete;
  LocalUsbDevice& operator=(const LocalUsbDevice&) = delete;

  // Destructor. Calls Close implicitly.
  ~LocalUsbDevice() override;

  util::Status Close(CloseAction action) override LOCKS_EXCLUDED(mutex_);

  util::Status SetConfiguration(int configuration) override
      LOCKS_EXCLUDED(mutex_);

  util::Status ClaimInterface(int interface_number) override
      LOCKS_EXCLUDED(mutex_);

  util::Status ReleaseInterface(int interface_number) override
      LOCKS_EXCLUDED(mutex_);

  util::Status GetDescriptor(DescriptorType desc_type, uint8_t desc_index,
                             MutableBuffer data_in,
                             size_t* num_bytes_transferred,
                             const char* context) override
      LOCKS_EXCLUDED(mutex_);

  DeviceSpeed GetDeviceSpeed() const override LOCKS_EXCLUDED(mutex_);

  util::Status SendControlCommand(const SetupPacket& command,
                                  TimeoutMillis timeout_msec,
                                  const char* context) override
      LOCKS_EXCLUDED(mutex_);

  util::Status SendControlCommandWithDataOut(const SetupPacket& command,
                                             ConstBuffer data_out,
                                             TimeoutMillis timeout_msec,
                                             const char* context) override
      LOCKS_EXCLUDED(mutex_);

  util::Status SendControlCommandWithDataIn(const SetupPacket& command,
                                            MutableBuffer data_in,
                                            size_t* num_bytes_transferred,
                                            TimeoutMillis timeout_msec,
                                            const char* context) override
      LOCKS_EXCLUDED(mutex_);

  util::Status BulkOutTransfer(uint8_t endpoint, ConstBuffer data_out,
                               TimeoutMillis timeout_msec,
                               const char* context) override
      LOCKS_EXCLUDED(mutex_);

  util::Status BulkInTransfer(uint8_t endpoint, MutableBuffer data_in,
                              size_t* num_bytes_transferred,
                              TimeoutMillis timeout_msec,
                              const char* context) override
      LOCKS_EXCLUDED(mutex_);

  util::Status InterruptInTransfer(uint8_t endpoint, MutableBuffer data_in,
                                   size_t* num_bytes_transferred,
                                   TimeoutMillis timeout_msec,
                                   const char* context) override
      LOCKS_EXCLUDED(mutex_);

  util::Status AsyncBulkOutTransfer(uint8_t endpoint, ConstBuffer data_out,
                                    TimeoutMillis timeout_msec,
                                    DataOutDone callback,
                                    const char* context) override
      LOCKS_EXCLUDED(mutex_);

  util::Status AsyncBulkInTransfer(uint8_t endpoint, MutableBuffer data_in,
                                   TimeoutMillis timeout_msec,
                                   DataInDone callback,
                                   const char* context) override
      LOCKS_EXCLUDED(mutex_);

  util::Status AsyncInterruptInTransfer(uint8_t endpoint, MutableBuffer data_in,
                                        TimeoutMillis timeout_msec,
                                        DataInDone callback,
                                        const char* context) override
      LOCKS_EXCLUDED(mutex_);

  void TryCancelAllTransfers() override LOCKS_EXCLUDED(mutex_);

  util::StatusOr<MutableBuffer> AllocateTransferBuffer(
      size_t buffer_size) override LOCKS_EXCLUDED(mutex_);

  util::Status ReleaseTransferBuffer(MutableBuffer buffer) override
      LOCKS_EXCLUDED(mutex_);

 private:
  friend class LocalUsbDeviceFactory;

  // User data carried in libusb transfer completion callback.
  struct AsyncDataOutUserData {
    // Pointer to device object.
    LocalUsbDevice* device;
    // Pointer to callback function object.
    DataOutDone callback;
  };

  // User data carried in libusb transfer completion callback.
  struct AsyncDataInUserData {
    // Pointer to device object.
    LocalUsbDevice* device;
    // Pointer to callback function object.
    DataInDone callback;
  };

  // Constructor. All instances of this class must be allocated through
  // LocalUsbManager.
  LocalUsbDevice(libusb_device_handle* handle, bool use_zero_copy,
                 libusb_context* context);

  // Callback function provided to libubs for data out completion callback.
  static void LibUsbDataOutCallback(libusb_transfer* transfer);

  // Callback function provided to libubs for data in completion callback.
  static void LibUsbDataInCallback(libusb_transfer* transfer);

  util::Status CheckForNullHandle(const char* context) const
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  void UnregisterCompletedTransfer(libusb_transfer* transfer)
      LOCKS_EXCLUDED(mutex_);

  // Allocates transfer buffer for this device.
  // Although this function doesn't explicitly modify any shared data, the
  // underlying data in a libusb device handle could be affected.
  uint8_t* DoAllocateTransferBuffer(size_t buffer_size)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Releases transfer buffer previous allocated. This function relies on caller
  // to perform bookkeeping of records. Although this function doesn't
  // explicitly modify any shared data, the underlying data in a libusb device
  // handle could be affected.
  util::Status DoReleaseTransferBuffer(MutableBuffer buffer)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  libusb_transfer* NewAsyncTransfer();
  void DestroyFailedAsyncTransfer(libusb_transfer* transfer_control);

  // Cancels all async transfers without explicitly locking the mutex.
  void DoCancelAllTransfers() EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  static constexpr int kLibUsbTransferNoIsoPackets = 0;

  // Serializes access to this interface and hence shared data.
  mutable std::mutex mutex_;

  // Wait till all async transfers complete.
  mutable std::condition_variable cond_;

  // True if the implementation should use libusb_dev_mem functions to allocate
  // memory.
  const bool use_zero_copy_;

  libusb_device_handle* libusb_handle_ GUARDED_BY(mutex_);

  // Interfaces, in current device configuration, have been claimed through
  // ClaimInterface.
  std::unordered_set<int> claimed_interfaces_ GUARDED_BY(mutex_);

  // Memory buffers allocated through libusb_dev_mem_alloc.
  std::map<uint8_t*, MutableBuffer> transfer_buffers_ GUARDED_BY(mutex_);

  // Serializes access to this interface and hence shared data.
  mutable std::mutex async_callback_mutex_;

  // Transfer control blocks allocated for async USB transfers.
  std::unordered_set<libusb_transfer*> async_transfers_
      GUARDED_BY(async_callback_mutex_);

  // Points to session context, which is allocated by libusb.
  libusb_context* libusb_context_{nullptr};

  // False if libusb event thread should stop running.
  std::atomic<bool> libusb_keep_running_{false};

  // Thread running the libusb event loop.
  std::thread libusb_event_thread_;
};

class LocalUsbDeviceFactory : public UsbDeviceFactory {
 public:
  // Holds components to a path string, pointing to a locally connected USB
  // device.
  struct ParsedPath {
    uint8 bus_number;
    std::vector<uint8> port_numbers;
  };

  LocalUsbDeviceFactory(bool use_zero_copy = false);

  ~LocalUsbDeviceFactory() override = default;

  // This class is neither copyable nor movable.
  LocalUsbDeviceFactory(const LocalUsbDeviceFactory&) = delete;
  LocalUsbDeviceFactory& operator=(const LocalUsbDeviceFactory&) = delete;

  util::StatusOr<std::vector<std::string>> EnumerateDevices(
      uint16_t vendor_id, uint16_t product_id) override;

  util::StatusOr<std::unique_ptr<UsbDeviceInterface>> OpenDevice(
      const std::string& path, TimeoutMillis timeout_msec) override;

  // Visible for testing.
  // Returns a path broken down to components.
  static util::StatusOr<ParsedPath> ParsePathString(const std::string& path);

  // Visible for testing.
  // Composes a path string from components.
  static std::string ComposePathString(const ParsedPath& path);

 private:
  // True if we should try to use memory allocation routine provided by libusb,
  // for zero copy support.
  const bool use_zero_copy_{false};
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_USB_LOCAL_USB_DEVICE_H_
