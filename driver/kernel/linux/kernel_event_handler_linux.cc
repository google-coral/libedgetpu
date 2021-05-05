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

#include "driver/kernel/linux/kernel_event_handler_linux.h"

#include <sys/eventfd.h>

#include "driver/kernel/gasket_ioctl.h"
#include "driver/kernel/linux/kernel_event_linux.h"
#include "port/errors.h"
#include "port/ptr_util.h"
#include "port/status.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

KernelEventHandlerLinux::KernelEventHandlerLinux(const std::string& device_path,
                                                 int num_events)
    : KernelEventHandler(device_path, num_events) {}

Status KernelEventHandlerLinux::SetEventFd(FileDescriptor fd,
                                           FileDescriptor event_fd,
                                           int event_id) const {
  gasket_interrupt_eventfd interrupt;
  interrupt.interrupt = event_id;
  interrupt.event_fd = event_fd;
  if (ioctl(fd, GASKET_IOCTL_SET_EVENTFD, &interrupt) != 0) {
    return FailedPreconditionError(
        StringPrintf("Setting Event Fd Failed : %d (%s)", fd, strerror(errno)));
  }

  VLOG(5) << StringPrintf("Set event fd : event_id:%d -> event_fd:%d, ",
                          event_id, event_fd);

  return OkStatus();
}

FileDescriptor KernelEventHandlerLinux::InitializeEventFd(int event_id) const {
  return eventfd(0, EFD_CLOEXEC);
}

Status KernelEventHandlerLinux::ReleaseEventFd(FileDescriptor fd,
                                               FileDescriptor event_fd,
                                               int event_id) const {
  close(event_fd);
  return OkStatus();
}

std::unique_ptr<KernelEvent> KernelEventHandlerLinux::CreateKernelEvent(
    FileDescriptor event_fd, KernelEvent::Handler handler) {
  return gtl::MakeUnique<KernelEventLinux>(event_fd, std::move(handler));
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
