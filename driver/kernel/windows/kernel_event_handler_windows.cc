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

#include "driver/kernel/windows/kernel_event_handler_windows.h"

#include <string.h>

#include <string>

#include "driver/kernel/gasket_ioctl.h"
#include "driver/kernel/windows/kernel_event_windows.h"
#include "port/errors.h"
#include "port/ptr_util.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace {

std::wstring EventName(const std::string& device_path, int event_id) {
  size_t index = device_path.find(APEX_DEVICE_NAME_BASE);
  if (index == std::string::npos) {
    LOG(ERROR) << "Unexpected device name " << device_path;
    return std::wstring();
  }
  std::string name =
      StringPrintf("%sEvent%d", &device_path.c_str()[index], event_id);
  return std::wstring(name.begin(), name.end());
}

}  // namespace

KernelEventHandlerWindows::KernelEventHandlerWindows(
    const std::string& device_path, int num_events)
    : KernelEventHandler(device_path, num_events) {}

Status KernelEventHandlerWindows::SetEventFd(FileDescriptor fd,
                                             FileDescriptor event_fd,
                                             int event_id) const {
  gasket_set_event_ioctl gasket_set_event;
  gasket_set_event.int_num = event_id;
  std::wstring event_name = EventName(GetDevicePath(), event_id);
  wcscpy(gasket_set_event.event_name, event_name.c_str());

  if (!DeviceIoControl(fd, GASKET_IOCTL_SET_EVENTFD, &gasket_set_event,
                       sizeof(gasket_set_event), NULL, 0, NULL, NULL)) {
    return InternalError(
        StringPrintf("Setting Interrupt event failed: event_id:%d gle=%d",
                     event_id, GetLastError()));
  }

  VLOG(5) << StringPrintf("Set event fd : event_id:%d -> event_fd:%p, ",
                          event_id, event_fd);

  return OkStatus();
}

FileDescriptor KernelEventHandlerWindows::InitializeEventFd(
    int event_id) const {
  std::wstring wc_event_name = EventName(GetDevicePath(), event_id);
  FileDescriptor event_fd =
      CreateEventW(NULL, TRUE, FALSE, wc_event_name.c_str());
  if (event_fd == INVALID_FD_VALUE) {
    LOG(ERROR) << "Create event failed: gle=" << GetLastError();
  }
  return event_fd;
}

Status KernelEventHandlerWindows::ReleaseEventFd(FileDescriptor fd,
                                                 FileDescriptor event_fd,
                                                 int event_id) const {
  if (fd == INVALID_FD_VALUE) {
    return FailedPreconditionError("Device not open.");
  }

  gasket_set_event_ioctl gasket_set_event;
  gasket_set_event.int_num = event_id;
  std::wstring event_name = EventName(GetDevicePath(), event_id);
  wcscpy(gasket_set_event.event_name, event_name.c_str());

  if (!DeviceIoControl(fd, GASKET_IOCTL_CLEAR_EVENTFD, &gasket_set_event,
                       sizeof(gasket_set_event), NULL, 0, NULL, NULL)) {
    return InternalError(
        StringPrintf("Clearing Interrupt event failed: event_id:%d gle=%d",
                     event_id, GetLastError()));
  }

  BOOL result = CloseHandle(event_fd);
  if (!result) {
    return InternalError(
        StringPrintf("Close Int Event failed: event_id:%d gle=%d", event_id,
                     GetLastError()));
  }

  return OkStatus();
}

std::unique_ptr<KernelEvent> KernelEventHandlerWindows::CreateKernelEvent(
    FileDescriptor event_fd, KernelEvent::Handler handler) {
  return gtl::MakeUnique<KernelEventWindows>(event_fd, std::move(handler));
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
