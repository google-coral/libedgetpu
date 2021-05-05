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

#include "driver/usb/local_usb_device.h"

#include <utility>

#include "driver/usb/libusb_options.h"
#include "driver/usb/usb_device_interface.h"
#include "port/cleanup.h"
#include "port/errors.h"
#include "port/logging.h"
#include "port/ptr_util.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"
#include "port/time.h"
#include "port/tracing.h"

#define VLOG_IF_ERROR(L, S)                               \
  if (!(S).ok()) {                                        \
    VLOG((L)) << S << " " << __FILE__ << ":" << __LINE__; \
  }

namespace platforms {
namespace darwinn {
namespace driver {

namespace {

// Max depth for USB 3 is 7.
constexpr int kMaxUsbPathDepth = 7;
constexpr const char* kUsbPathPrefix = "/sys/bus/usb/devices/";

// Automatic retry for control commands, to reduce failure rates.
constexpr int kMaxNumRetriesForCommands = 5;

// Automatic retries for checking if device is available after Close.
constexpr int kMaxNumRetriesForClose = 3;

Status ConvertLibUsbError(int error, const char* context) {
  if (error >= 0) {
    return Status();  // OK.
  }

  std::string logline = StringPrintf("USB error %d [%s]", error, context);

  VLOG(1) << StringPrintf("%s: %s", __func__, logline.c_str());

  switch (error) {
    case LIBUSB_ERROR_INVALID_PARAM:
      return InvalidArgumentError(logline);
    case LIBUSB_ERROR_ACCESS:
      return PermissionDeniedError(logline);
    case LIBUSB_ERROR_NO_MEM:
      return ResourceExhaustedError(logline);
    case LIBUSB_ERROR_NO_DEVICE:
      return UnavailableError(logline);
    case LIBUSB_ERROR_NOT_FOUND:
      return NotFoundError(logline);
    case LIBUSB_ERROR_BUSY:
      return DeadlineExceededError(logline);
    case LIBUSB_ERROR_TIMEOUT:
      return DeadlineExceededError(logline);
    case LIBUSB_ERROR_OVERFLOW:
      return DataLossError(logline);
    case LIBUSB_ERROR_INTERRUPTED:
      return CancelledError(logline);
    case LIBUSB_ERROR_NOT_SUPPORTED:
      return UnimplementedError(logline);
    default:
      return UnknownError(logline);
  }
}

Status ConvertLibUsbTransferStatus(libusb_transfer_status status,
                                   const char* context) {
  if (status == LIBUSB_TRANSFER_COMPLETED) {
    return Status();  // OK.
  }

  std::string logline =
      StringPrintf("USB transfer error %d [%s]", status, context);

  VLOG(1) << StringPrintf("%s: %s", __func__, logline.c_str());

  switch (status) {
    case LIBUSB_TRANSFER_TIMED_OUT:
      return DeadlineExceededError(logline);
    case LIBUSB_TRANSFER_CANCELLED:
      return CancelledError(logline);
    case LIBUSB_TRANSFER_STALL:
      return InvalidArgumentError(logline);
    case LIBUSB_TRANSFER_NO_DEVICE:
      return NotFoundError(logline);
    case LIBUSB_TRANSFER_OVERFLOW:
      return DataLossError(logline);
    default:
      return UnknownError(logline);
  }
}

// Automatically retries a libusb command on error.
template <typename LibUsbCommand>
Status AutoRetryLibUsbCommand(const LibUsbCommand& func, const char* context,
                              int* command_result = nullptr) {
  int result = 0;
  for (int attempt_count = 0; attempt_count < kMaxNumRetriesForCommands;
       ++attempt_count) {
    result = func();
    if (result < 0) {
      (void)ConvertLibUsbError(result, context);
      VLOG(1) << StringPrintf("[%s] failed [%d].", context, attempt_count + 1);
    } else {
      break;
    }
  }

  if (command_result) {
    *command_result = result;
  }
  return ConvertLibUsbError(result, context);
}

// Find if a device exists at a given bus/port combination, with retries.
// Used to detect if a device has finished being released by the OS during
// Close.
Status FindDeviceByBusAndPortWithRetries(libusb_context* context,
                                         int bus_number, int port_number) {
  for (int attempt_count = 0; attempt_count < kMaxNumRetriesForClose;
       ++attempt_count) {
    libusb_device** device_list;
    ssize_t device_count = libusb_get_device_list(context, &device_list);
    // Clean up the device list when we leave this scope.
    auto device_list_cleaner =
        MakeCleanup([device_list] { libusb_free_device_list(device_list, 1); });
    bool found = false;
    for (int i = 0; i < device_count; i++) {
      libusb_device* device = device_list[i];
      int device_bus_number = libusb_get_bus_number(device);
      int device_port_number = libusb_get_port_number(device);
      if (device_port_number == port_number &&
          device_bus_number == bus_number) {
        found = true;
        break;
      }
    }
    if (found) {
      return Status();
    } else {
      Sleep(1);
    }
  }

  return NotFoundError(StringPrintf(
      "Could not find device on bus %d and port %d.", bus_number, port_number));
}

}  // namespace

LocalUsbDevice::LocalUsbDevice(libusb_device_handle* handle, bool use_zero_copy,
                               libusb_context* context)
    : use_zero_copy_(use_zero_copy),
      libusb_handle_(handle),
      libusb_context_(context) {
#if !LIBUSB_HAS_MEM_ALLOC
  (void)use_zero_copy_;
#endif  // LIBUSB_HAS_MEM_ALLOC

  CHECK(handle != nullptr);
  CHECK(context != nullptr);
  VLOG(10) << __func__;

  libusb_keep_running_ = true;
  libusb_event_thread_ = std::thread([this]() NO_THREAD_SAFETY_ANALYSIS {
    TRACE_START_THREAD("LocalUsbDeviceEventThread");
    while (libusb_keep_running_) {
      libusb_handle_events(libusb_context_);
    }
  });
}

LocalUsbDevice::~LocalUsbDevice() {
  VLOG(10) << __func__;
  (void)Close(CloseAction::kNoReset);
}

Status LocalUsbDevice::CheckForNullHandle(const char* context) const {
  if (libusb_handle_ == nullptr) {
    return FailedPreconditionError(context);
  }
  return Status();  // OK.
}

void LocalUsbDevice::TryCancelAllTransfers() {
  StdMutexLock lock(&mutex_);
  DoCancelAllTransfers();
}

void LocalUsbDevice::DoCancelAllTransfers() {
  {
    StdCondMutexLock cond_lock(&async_callback_mutex_);
    // Cancel all async transfer.
    VLOG(9) << StringPrintf("%s: cancelling %d async transfers", __func__,
                            static_cast<int>(async_transfers_.size()));
    for (auto transfer_control_block : async_transfers_) {
      VLOG_IF_ERROR(
          1, ConvertLibUsbError(libusb_cancel_transfer(transfer_control_block),
                                __func__));
    }

    VLOG(9) << StringPrintf("%s: waiting for all async transfers to complete",
                            __func__);

    // Wait for all async transfer to complete.
    // This could take some time, as cancel may or may not work.
    while (!async_transfers_.empty()) {
      cond_.wait(cond_lock);
    }
  }

  VLOG(9) << StringPrintf("%s: all async transfers have completed", __func__);
}

// TODO use status update to record the first failure.
Status LocalUsbDevice::Close(CloseAction action) {
  TRACE_SCOPE("LocalUsbDevice::Close");

  StdMutexLock lock(&mutex_);

  VLOG(6) << StringPrintf("%s: closing device %p ", __func__, libusb_handle_);

  RETURN_IF_ERROR(CheckForNullHandle(__func__));

  // Perform forceful reset if so specified.
  switch (action) {
    case CloseAction::kForcefulPortReset:
    case CloseAction::kForcefulChipReset: {
      TRACE_SCOPE("LocalUsbDevice::Close:forceful_reset");
      VLOG(1) << StringPrintf("%s: forcefully reset device %p", __func__,
                              libusb_handle_);
      VLOG_IF_ERROR(
          1, ConvertLibUsbError(libusb_reset_device(libusb_handle_), __func__));
      break;
    }
    default: {
      TRACE_SCOPE("LocalUsbDevice::Close:release_interface");
      // Release all interfaces claimed for this handle.
      // This is not needed if we reset the device upfront.
      for (auto interface_id : claimed_interfaces_) {
        VLOG(9) << StringPrintf("%s: releasing claimed interface %d", __func__,
                                interface_id);
        VLOG_IF_ERROR(1, ConvertLibUsbError(libusb_release_interface(
                                                libusb_handle_, interface_id),
                                            __func__));
      }
      break;
    }
  }

  DoCancelAllTransfers();

  // Release all transfer buffers allocated for this handle.
  VLOG(9) << StringPrintf("%s: releasing %d transfer buffers", __func__,
                          static_cast<int>(transfer_buffers_.size()));

  for (auto& record : transfer_buffers_) {
    VLOG_IF_ERROR(1, DoReleaseTransferBuffer(record.second));
  }
  transfer_buffers_.clear();

  // Perform graceful reset if so specified.
  switch (action) {
    case CloseAction::kGracefulPortReset:
    case CloseAction::kGracefulChipReset: {
      TRACE_SCOPE("LocalUsbDevice::Close:graceful_reset");
      VLOG(9) << StringPrintf("%s: performing graceful reset", __func__);
      VLOG_IF_ERROR(
          1, ConvertLibUsbError(libusb_reset_device(libusb_handle_), __func__));
      break;
    }
    default: {
      // Do nothing.
      break;
    }
  }

  libusb_keep_running_ = false;

  // Get the libusb bus/port number before closing.
  int this_bus_number, this_port_number;
  libusb_device* this_dev = libusb_get_device(libusb_handle_);
  this_bus_number = libusb_get_bus_number(this_dev);
  this_port_number = libusb_get_port_number(this_dev);

  // Close the libusb device handle. Event thread is awaken by this
  // action.
  libusb_close(libusb_handle_);
  libusb_handle_ = nullptr;
  libusb_event_thread_.join();

  // Block until the closed device reappears on the USB bus (or for
  // kMaxNumRetriesForClose checks).
  VLOG_IF_ERROR(1, FindDeviceByBusAndPortWithRetries(
                       libusb_context_, this_bus_number, this_port_number));

  libusb_exit(libusb_context_);
  libusb_context_ = nullptr;

  VLOG(9) << StringPrintf("%s: final clean up completed", __func__);

  return Status();  // OK.
}

Status LocalUsbDevice::SetConfiguration(int configuration) {
  VLOG(10) << __func__;
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(CheckForNullHandle(__func__));

  // Claimed interfaces for current configuration should already be released
  // before we change configuration.
  if (!claimed_interfaces_.empty()) {
    VLOG(1) << StringPrintf("%s Claimed interfaces have not been released",
                            __func__);
    claimed_interfaces_.clear();
  }

  // This alias is created to circumvent thread safety analysis.
  // Accessing to libusb_handle_ must be protected by a mutex.
  auto* libusb_handle_alias = libusb_handle_;
  return AutoRetryLibUsbCommand(
      [=] {
        return libusb_set_configuration(libusb_handle_alias, configuration);
      },
      __func__);
}

Status LocalUsbDevice::ClaimInterface(int interface_number) {
  TRACE_SCOPE("LocalUsbDevice::ClaimInterface");
  VLOG(10) << __func__;

  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(CheckForNullHandle(__func__));

  // This alias is created to circumvent thread safety analysis.
  // Accessing to libusb_handle_ must be protected by a mutex.
  auto* libusb_handle_alias = libusb_handle_;
  RETURN_IF_ERROR(AutoRetryLibUsbCommand(
      [=] {
        return libusb_claim_interface(libusb_handle_alias, interface_number);
      },
      __func__));

  claimed_interfaces_.insert(interface_number);
  return Status();  // OK.
}

Status LocalUsbDevice::ReleaseInterface(int interface_number) {
  TRACE_SCOPE("LocalUsbDevice::ReleaseInterface");
  VLOG(10) << __func__;

  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(CheckForNullHandle(__func__));
  auto iterator = claimed_interfaces_.find(interface_number);
  if (iterator != claimed_interfaces_.end()) {
    // This alias is created to circumvent thread safety analysis.
    // Accessing to libusb_handle_ must be protected by a mutex.
    auto* libusb_handle_alias = libusb_handle_;
    RETURN_IF_ERROR(AutoRetryLibUsbCommand(
        [=] {
          return libusb_release_interface(libusb_handle_alias,
                                          interface_number);
        },
        __func__));

    claimed_interfaces_.erase(iterator);
    return Status();  // OK.
  }
  return NotFoundError(__func__);
}

Status LocalUsbDevice::GetDescriptor(DescriptorType desc_type,
                                     uint8_t desc_index, MutableBuffer data_in,
                                     size_t* num_bytes_transferred,
                                     const char* context) {
  VLOG(10) << __func__;
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(CheckForNullHandle(__func__));

  int result = 0;

  // This alias is created to circumvent thread safety analysis.
  // Accessing to libusb_handle_ must be protected by a mutex.
  auto* libusb_handle_alias = libusb_handle_;
  RETURN_IF_ERROR(AutoRetryLibUsbCommand(
      [=] {
        // data_in is shallow copied into this lambda.
        return libusb_get_descriptor(
            libusb_handle_alias, static_cast<uint8_t>(desc_type), desc_index,
            data_in.data(), static_cast<int>(data_in.size()));
      },
      context, &result));

  *num_bytes_transferred = static_cast<size_t>(result);
  return Status();  // OK.
}

UsbDeviceInterface::DeviceSpeed LocalUsbDevice::GetDeviceSpeed() const {
  StdMutexLock lock(&mutex_);
  if (!CheckForNullHandle(__func__).ok()) {
    return UsbDeviceInterface::DeviceSpeed::kUnknown;
  }

  const int result = libusb_get_device_speed(libusb_get_device(libusb_handle_));

  switch (result) {
    case LIBUSB_SPEED_LOW:
      return UsbDeviceInterface::DeviceSpeed::kLow;
    case LIBUSB_SPEED_FULL:
      return UsbDeviceInterface::DeviceSpeed::kFull;
    case LIBUSB_SPEED_HIGH:
      return UsbDeviceInterface::DeviceSpeed::kHigh;
    case LIBUSB_SPEED_SUPER:
      return UsbDeviceInterface::DeviceSpeed::kSuper;
    case LIBUSB_SPEED_UNKNOWN:
    default:
      return UsbDeviceInterface::DeviceSpeed::kUnknown;
  }
}

Status LocalUsbDevice::SendControlCommand(const SetupPacket& command,
                                          TimeoutMillis timeout_msec,
                                          const char* context) {
  VLOG(10) << __func__;
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(CheckForNullHandle(__func__));

  // Length must be 0.
  if (command.length != 0) {
    return InvalidArgumentError("Length must be 0");
  }

  // This alias is created to circumvent thread safety analysis.
  // Accessing to libusb_handle_ must be protected by a mutex.
  auto* libusb_handle_alias = libusb_handle_;

  return AutoRetryLibUsbCommand(
      [=] {
        int result = libusb_control_transfer(
            libusb_handle_alias, command.request_type, command.request,
            command.value, command.index, nullptr, 0, timeout_msec);

        // Only 0 is the right answer here.
        return (result > 0) ? LIBUSB_ERROR_OVERFLOW : result;
      },
      __func__);
}

Status LocalUsbDevice::SendControlCommandWithDataOut(const SetupPacket& command,
                                                     ConstBuffer data_out,
                                                     TimeoutMillis timeout_msec,
                                                     const char* context) {
  VLOG(10) << __func__;
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(CheckForNullHandle(__func__));

  // Length must be less than or equal to  buffer size.
  CHECK_LE(command.length, data_out.length());

  VLOG(10) << "SYNC CTRL WITH DATA OUT begin";

  int result = 0;

  // This alias is created to circumvent thread safety analysis.
  // Accessing to libusb_handle_ must be protected by a mutex.
  auto* libusb_handle_alias = libusb_handle_;

  RETURN_IF_ERROR(AutoRetryLibUsbCommand(
      [=] {
        return libusb_control_transfer(
            libusb_handle_alias, command.request_type, command.request,
            command.value, command.index, const_cast<uint8_t*>(data_out.data()),
            command.length, timeout_msec);
      },
      context, &result));

  VLOG(10) << "SYNC CTRL WITH DATA OUT end";

  // Result must be less than or equal to specified data amount.
  CHECK_LE(result, command.length);

  if (result != command.length) {
    return DataLossError(__func__);
  }
  return Status();  // OK.
}

Status LocalUsbDevice::SendControlCommandWithDataIn(
    const SetupPacket& command, MutableBuffer data_in,
    size_t* num_bytes_transferred, TimeoutMillis timeout_msec,
    const char* context) {
  VLOG(10) << __func__;
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(CheckForNullHandle(__func__));

  // Length must be less than or equal to  buffer size.
  CHECK_LE(command.length, data_in.length());

  VLOG(10) << "SYNC CTRL WITH DATA IN begin";

  int result = 0;

  // This alias is created to circumvent thread safety analysis.
  // Accessing to libusb_handle_ must be protected by a mutex.
  auto* libusb_handle_alias = libusb_handle_;

  RETURN_IF_ERROR(AutoRetryLibUsbCommand(
      [=] {
        return libusb_control_transfer(
            libusb_handle_alias, command.request_type, command.request,
            command.value, command.index, data_in.data(), command.length,
            timeout_msec);
      },
      context, &result));

  VLOG(10) << "SYNC CTRL WITH DATA IN end";

  // Result must be less than or equal to specified data amount.
  CHECK_LE(result, command.length);

  *num_bytes_transferred = static_cast<size_t>(result);
  return Status();  // OK.
}

Status LocalUsbDevice::BulkOutTransfer(uint8_t endpoint, ConstBuffer data_out,
                                       TimeoutMillis timeout_msec,
                                       const char* context) {
  VLOG(10) << __func__;
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(CheckForNullHandle(__func__));

  int amount_transferred = 0;

  VLOG(10) << StringPrintf("SYNC OUT %d begin", endpoint);

  const int result = libusb_bulk_transfer(
      libusb_handle_, endpoint | LIBUSB_ENDPOINT_OUT,
      const_cast<uint8_t*>(data_out.data()), data_out.length(),
      &amount_transferred, timeout_msec);

  VLOG(10) << StringPrintf("SYNC OUT %d end", endpoint);

  if (result < 0) {
    return ConvertLibUsbError(result, __func__);
  } else {
    // Underrun is a fatal error.
    CHECK_LE(static_cast<size_t>(amount_transferred), data_out.length());

    if (static_cast<size_t>(amount_transferred) != data_out.length()) {
      return DataLossError(__func__);
    }
  }
  return Status();  // OK.
}

Status LocalUsbDevice::BulkInTransfer(uint8_t endpoint, MutableBuffer data_in,
                                      size_t* num_bytes_transferred,
                                      TimeoutMillis timeout_msec,
                                      const char* context) {
  VLOG(10) << __func__;
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(CheckForNullHandle(__func__));

  int amount_transferred = 0;
  *num_bytes_transferred = 0;

  VLOG(10) << StringPrintf("SYNC IN %d begin", endpoint);

  const int result = libusb_bulk_transfer(
      libusb_handle_, endpoint | LIBUSB_ENDPOINT_IN, data_in.data(),
      data_in.length(), &amount_transferred, timeout_msec);

  VLOG(10) << StringPrintf("SYNC IN %d end", endpoint);

  *num_bytes_transferred = static_cast<size_t>(amount_transferred);

  if (result < 0) {
    return ConvertLibUsbError(result, __func__);
  } else {
    // Overflow is a fatal error.
    CHECK_LE(*num_bytes_transferred, data_in.length());
  }

  return Status();  // OK.
}

Status LocalUsbDevice::InterruptInTransfer(uint8_t endpoint,
                                           MutableBuffer data_in,
                                           size_t* num_bytes_transferred,
                                           TimeoutMillis timeout_msec,
                                           const char* context) {
  VLOG(10) << __func__;
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(CheckForNullHandle(__func__));

  int amount_transferred = 0;
  *num_bytes_transferred = 0;

  VLOG(10) << StringPrintf("SYNC IN %d begin", endpoint);

  const int result = libusb_interrupt_transfer(
      libusb_handle_, endpoint | LIBUSB_ENDPOINT_IN, data_in.data(),
      data_in.length(), &amount_transferred, timeout_msec);

  VLOG(10) << StringPrintf("SYNC IN %d end", endpoint);

  *num_bytes_transferred = static_cast<size_t>(amount_transferred);

  if (result < 0) {
    return ConvertLibUsbError(result, __func__);
  } else {
    // Overflow is a fatal error.
    CHECK_LE(*num_bytes_transferred, data_in.length());
  }

  return Status();  // OK.
}

void LocalUsbDevice::UnregisterCompletedTransfer(libusb_transfer* transfer) {
  VLOG(10) << __func__;
  StdMutexLock lock(&async_callback_mutex_);

  // There must be exactly one element to be erased.
  CHECK_EQ(async_transfers_.erase(transfer), 1);

  // Notify all, mostly just the main thread which is trying to close this
  // device, that status has changed.
  cond_.notify_all();
}

void LocalUsbDevice::LibUsbDataOutCallback(libusb_transfer* transfer) {
  AsyncDataOutUserData* callback_obj =
      static_cast<AsyncDataOutUserData*>(transfer->user_data);

  VLOG(10) << StringPrintf("ASYNC OUT %d end", transfer->endpoint);

  // The callback function is delivered without locking the host interface.
  // This allows further calls to be made during callback.
  (callback_obj->callback)(
      ConvertLibUsbTransferStatus(transfer->status, __func__));

  callback_obj->device->UnregisterCompletedTransfer(transfer);
  delete callback_obj;
}

void LocalUsbDevice::LibUsbDataInCallback(libusb_transfer* transfer) {
  AsyncDataInUserData* callback_obj =
      reinterpret_cast<AsyncDataInUserData*>(transfer->user_data);

  VLOG(10) << StringPrintf("ASYNC IN %d end", transfer->endpoint & 0x7F);

  // The callback function is delivered without locking the host interface.
  // This allows further calls to be made during callback.
  (callback_obj->callback)(
      ConvertLibUsbTransferStatus(transfer->status, __func__),
      static_cast<size_t>(transfer->actual_length));

  callback_obj->device->UnregisterCompletedTransfer(transfer);
  delete callback_obj;
}

libusb_transfer* LocalUsbDevice::NewAsyncTransfer() {
  // Allocate transfer control block.
  libusb_transfer* transfer_control =
      libusb_alloc_transfer(kLibUsbTransferNoIsoPackets);
  CHECK(transfer_control != nullptr);

  StdMutexLock lock(&async_callback_mutex_);
  async_transfers_.insert(transfer_control);

  return transfer_control;
}

void LocalUsbDevice::DestroyFailedAsyncTransfer(
    libusb_transfer* transfer_control) {
  StdMutexLock lock(&async_callback_mutex_);
  async_transfers_.erase(transfer_control);
  libusb_free_transfer(transfer_control);
}

Status LocalUsbDevice::AsyncBulkOutTransfer(uint8_t endpoint,
                                            ConstBuffer data_out,
                                            TimeoutMillis timeout_msec,
                                            DataOutDone callback,
                                            const char* context) {
  VLOG(10) << __func__;
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(CheckForNullHandle(__func__));

  // Allocate transfer control block.
  libusb_transfer* transfer_control = NewAsyncTransfer();

  // Allocate user data for completion callback.
  // This object is passed to transfer completion callback and will be released
  // there.
  AsyncDataOutUserData* callback_obj =
      new AsyncDataOutUserData{this, std::move(callback)};
  CHECK(callback_obj != nullptr);

  VLOG(10) << StringPrintf("ASYNC OUT %d begin", endpoint);

  libusb_fill_bulk_transfer(
      transfer_control, libusb_handle_, endpoint | LIBUSB_ENDPOINT_OUT,
      const_cast<uint8_t*>(data_out.data()), data_out.length(),
      LibUsbDataOutCallback, callback_obj, timeout_msec);

  transfer_control->flags |=
      (LIBUSB_TRANSFER_SHORT_NOT_OK | LIBUSB_TRANSFER_FREE_TRANSFER);

  Status status =
      ConvertLibUsbError(libusb_submit_transfer(transfer_control), __func__);

  if (!status.ok()) {
    DestroyFailedAsyncTransfer(transfer_control);
    delete callback_obj;
  }

  return status;
}

Status LocalUsbDevice::AsyncBulkInTransfer(uint8_t endpoint,
                                           MutableBuffer data_in,
                                           TimeoutMillis timeout_msec,
                                           DataInDone callback,
                                           const char* context) {
  VLOG(10) << __func__;
  StdMutexLock lock(&mutex_);

  RETURN_IF_ERROR(CheckForNullHandle(__func__));

  // Allocate transfer control block.
  libusb_transfer* transfer_control = NewAsyncTransfer();

  // Allocate user data for completion callback.
  AsyncDataInUserData* callback_obj =
      new AsyncDataInUserData{this, std::move(callback)};
  CHECK(callback_obj != nullptr);

  VLOG(10) << StringPrintf("ASYNC IN %d begin", endpoint & 0x7F);

  libusb_fill_bulk_transfer(transfer_control, libusb_handle_,
                            endpoint | LIBUSB_ENDPOINT_IN, data_in.data(),
                            data_in.length(), LibUsbDataInCallback,
                            callback_obj, timeout_msec);

  transfer_control->flags |= LIBUSB_TRANSFER_FREE_TRANSFER;

  Status status =
      ConvertLibUsbError(libusb_submit_transfer(transfer_control), __func__);

  if (!status.ok()) {
    DestroyFailedAsyncTransfer(transfer_control);
    delete callback_obj;
  }
  return status;
}

Status LocalUsbDevice::AsyncInterruptInTransfer(uint8_t endpoint,
                                                MutableBuffer data_in,
                                                TimeoutMillis timeout_msec,
                                                DataInDone callback,
                                                const char* context) {
  VLOG(10) << __func__;
  StdMutexLock lock(&mutex_);

  RETURN_IF_ERROR(CheckForNullHandle(__func__));

  // Allocate transfer control block.
  libusb_transfer* transfer_control = NewAsyncTransfer();

  // Allocate user data for completion callback.
  AsyncDataInUserData* callback_obj =
      new AsyncDataInUserData{this, std::move(callback)};
  CHECK(callback_obj != nullptr);

  VLOG(10) << StringPrintf("ASYNC IN %d begin", endpoint & 0x7F);

  libusb_fill_interrupt_transfer(transfer_control, libusb_handle_,
                                 endpoint | LIBUSB_ENDPOINT_IN, data_in.data(),
                                 data_in.length(), LibUsbDataInCallback,
                                 callback_obj, timeout_msec);

  transfer_control->flags |= LIBUSB_TRANSFER_FREE_TRANSFER;

  Status status =
      ConvertLibUsbError(libusb_submit_transfer(transfer_control), __func__);

  if (!status.ok()) {
    DestroyFailedAsyncTransfer(transfer_control);
    delete callback_obj;
  }
  return status;
}

StatusOr<LocalUsbDevice::MutableBuffer> LocalUsbDevice::AllocateTransferBuffer(
    size_t buffer_size) {
  VLOG(10) << __func__;
  StdMutexLock lock(&mutex_);

  RETURN_IF_ERROR(CheckForNullHandle(__func__));

  uint8_t* ptr = DoAllocateTransferBuffer(buffer_size);
  if (ptr == nullptr) {
    return ResourceExhaustedError(__func__);
  }

  auto result = transfer_buffers_.insert(
      std::make_pair(ptr, MutableBuffer(ptr, buffer_size)));

  return result.first->second;
}

uint8_t* LocalUsbDevice::DoAllocateTransferBuffer(size_t buffer_size) {
#if LIBUSB_HAS_MEM_ALLOC
  if (use_zero_copy_) {
    // Release memory block through libusb and return from here.
    return libusb_dev_mem_alloc(libusb_handle_, buffer_size);
  }
#endif  // LIBUSB_HAS_MEM_ALLOC

  return new uint8_t[buffer_size];
}

Status LocalUsbDevice::ReleaseTransferBuffer(MutableBuffer buffer) {
  VLOG(10) << __func__;
  StdMutexLock lock(&mutex_);

  RETURN_IF_ERROR(CheckForNullHandle(__func__));

  auto block = transfer_buffers_.find(buffer.data());

  // Missing record of the memory buffer is a fatal error.
  CHECK(block != transfer_buffers_.end());

  // Remove record before we actually release the memory.
  // Iterator is invalidated after this line.
  transfer_buffers_.erase(block);

  return DoReleaseTransferBuffer(buffer);
}

Status LocalUsbDevice::DoReleaseTransferBuffer(MutableBuffer buffer) {
#if LIBUSB_HAS_MEM_ALLOC
  if (use_zero_copy_) {
    // Release memory block through libusb and return from here.
    return ConvertLibUsbError(
        libusb_dev_mem_free(libusb_handle_, buffer.data(), buffer.length()),
        __func__);
  }
#endif  // LIBUSB_HAS_MEM_ALLOC

  // Use plain old delete [] to release the memory block.
  delete[] buffer.data();

  return Status();  // OK.
}

LocalUsbDeviceFactory::LocalUsbDeviceFactory(bool use_zero_copy)
    : use_zero_copy_(use_zero_copy) {}

StatusOr<LocalUsbDeviceFactory::ParsedPath>
LocalUsbDeviceFactory::ParsePathString(const std::string& path) {
  ParsedPath result;
  unsigned int bus_number;
  const size_t prefix_length = strlen(kUsbPathPrefix);
  if (path.length() <= prefix_length) {
    return InvalidArgumentError("Path must be longer than the proper prefix");
  }

  std::stringstream path_stingstream(path.substr(prefix_length));

  path_stingstream >> bus_number;
  if (path_stingstream.fail()) {
    return InvalidArgumentError("Path must begin with bus number");
  }
  if (path_stingstream.peek() == '-') {
    path_stingstream.ignore();
  } else {
    return InvalidArgumentError("Missing separator after bus number");
  }
  // TODO: check for valid bus number range.
  result.bus_number = static_cast<uint8>(bus_number);

  unsigned int port;
  while (path_stingstream >> port) {
    if (path_stingstream.fail()) {
      return InvalidArgumentError("Path must contain port numbers");
    }

    // TODO: check for valid port number range.
    result.port_numbers.push_back(static_cast<uint8>(port));

    if (path_stingstream.peek() == '.') {
      path_stingstream.ignore();
    }
  }

  return result;
}

std::string LocalUsbDeviceFactory::ComposePathString(
    const LocalUsbDeviceFactory::ParsedPath& path) {
  std::stringstream result;
  result << kUsbPathPrefix;
  result << static_cast<int>(path.bus_number);
  bool is_first_port = true;
  for (uint8 port_number : path.port_numbers) {
    if (is_first_port) {
      result << '-';
      is_first_port = false;
    } else {
      result << '.';
    }
    result << static_cast<int>(port_number);
  }
  return result.str();
}

StatusOr<std::vector<std::string>> LocalUsbDeviceFactory::EnumerateDevices(
    uint16_t vendor_id, uint16_t product_id) {
  TRACE_SCOPE("LocalUsbDeviceFactory::EnumerateDevices");
  VLOG(6) << StringPrintf("%s: vendor:0x%x, product:0x%x", __func__, vendor_id,
                          product_id);

  libusb_context* context = nullptr;
  const int libusb_init_error = libusb_init(&context);
  if (libusb_init_error != 0) {
    return FailedPreconditionError("libusb initialization failed");
  }

  Status libusb_option_status =
      ConvertLibUsbError(SetLibUsbOptions(context), "SetLibUsbOptions");
  RETURN_IF_ERROR(libusb_option_status);

  auto context_cleaner = MakeCleanup([context] { libusb_exit(context); });

  // Find the specified devices
  libusb_device** device_list = nullptr;
  ssize_t num_device_or_error = libusb_get_device_list(context, &device_list);

  if (num_device_or_error < 0) {
    return ConvertLibUsbError(num_device_or_error, __func__);
  }
  auto device_list_cleaner = MakeCleanup([device_list] {
    // The device list must be freed in the end.
    // Remove one reference from all devices in the device list.
    libusb_free_device_list(device_list, 1);
  });

  std::vector<std::string> device_paths;

  for (ssize_t device_index = 0; device_index < num_device_or_error;
       ++device_index) {
    libusb_device* device = device_list[device_index];
    libusb_device_descriptor device_descriptor = {0};
    const uint8 bus_number = libusb_get_bus_number(device);
    VLOG(7) << StringPrintf("%s: checking bus[%d] port[%d]", __func__,
                            bus_number, libusb_get_port_number(device));
    if (libusb_get_device_descriptor(device, &device_descriptor) ==
        LIBUSB_SUCCESS) {
      if ((device_descriptor.idVendor == vendor_id) &&
          (device_descriptor.idProduct == product_id)) {
        // Generate path string for this device.
        uint8 port_numbers[kMaxUsbPathDepth] = {0};
        const int depth_or_error =
            libusb_get_port_numbers(device, port_numbers, kMaxUsbPathDepth);
        if (depth_or_error < 0) {
          VLOG(2) << StringPrintf("%s: get device port numbers failed:",
                                  __func__)
                  << ConvertLibUsbError(depth_or_error, __func__);
        } else {
          ParsedPath parsed_path = {
              bus_number,
              std::vector<uint8>(port_numbers, port_numbers + depth_or_error)};
          std::string path = ComposePathString(parsed_path);
          VLOG(2) << StringPrintf("%s: found [%s]", __func__, path.c_str());
          device_paths.push_back(path);
        }
      }
    } else {
      VLOG(2) << StringPrintf("%s: get device descriptor failed", __func__);
    }
  }

  return device_paths;
}

StatusOr<std::unique_ptr<UsbDeviceInterface>> LocalUsbDeviceFactory::OpenDevice(
    const std::string& path, TimeoutMillis) {
  TRACE_SCOPE("LocalUsbDeviceFactory::OpenDevice");
  VLOG(6) << StringPrintf("%s: [%s]", __func__, path.c_str());

  ASSIGN_OR_RETURN(auto parsed_path, ParsePathString(path));

  libusb_context* context = nullptr;
  const int libusb_init_error = libusb_init(&context);
  if (libusb_init_error != 0) {
    return FailedPreconditionError("libusb initialization failed");
  }

  Status libusb_option_status =
      ConvertLibUsbError(SetLibUsbOptions(context), "SetLibUsbOptions");
  if (!libusb_option_status.ok()) {
    return libusb_option_status;
  }

  auto context_cleaner = MakeCleanup([context] { libusb_exit(context); });

  // Find the specified devices
  libusb_device** device_list = nullptr;
  libusb_device* found_device = nullptr;
  ssize_t num_device_or_error = libusb_get_device_list(context, &device_list);

  if (num_device_or_error < 0) {
    return ConvertLibUsbError(num_device_or_error, __func__);
  }
  auto device_list_cleaner = MakeCleanup([device_list] {
    // The device list must be freed in the end.
    // Remove one reference from all devices in the device list.
    libusb_free_device_list(device_list, 1);
  });

  for (ssize_t device_index = 0; device_index < num_device_or_error;
       ++device_index) {
    libusb_device* device = device_list[device_index];

    const uint8 bus_number = libusb_get_bus_number(device);
    VLOG(7) << StringPrintf("%s: checking bus[%d] port[%d]", __func__,
                            bus_number, libusb_get_port_number(device));

    if (bus_number != parsed_path.bus_number) {
      continue;
    }

    // Generate path string for this device.
    uint8 port_numbers[kMaxUsbPathDepth] = {0};
    const int depth_or_error =
        libusb_get_port_numbers(device, port_numbers, kMaxUsbPathDepth);
    if (depth_or_error < 0) {
      VLOG(2) << StringPrintf("%s: get device port numbers failed:", __func__)
              << ConvertLibUsbError(depth_or_error, __func__);
    } else if (depth_or_error == parsed_path.port_numbers.size()) {
      if (memcmp(port_numbers, parsed_path.port_numbers.data(),
                 parsed_path.port_numbers.size()) == 0) {
        found_device = device;
        break;
      }
    }
  }

  libusb_device_handle* libusb_handle = nullptr;
  if (found_device) {
    RETURN_IF_ERROR(ConvertLibUsbError(
        libusb_open(found_device, &libusb_handle), __func__));
  } else {
    return NotFoundError(__func__);
  }

  VLOG(6) << StringPrintf("%s: device opened %p", __func__, libusb_handle);

  std::unique_ptr<UsbDeviceInterface> device = gtl::WrapUnique(
      new LocalUsbDevice(libusb_handle, use_zero_copy_, context));

  CHECK(device);

  // Ownership of the libusb context has been transferred to the new device.
  context_cleaner.release();

  // This statement explicitly constructs an unique_ptr, instead of relying on
  // implicit compiler-invoked conversion. Some C++ 11 compilers/verions do
  // not properly invoke the most suitable conversion.
  return {std::move(device)};
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
