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

#ifndef DARWINN_DRIVER_KERNEL_KERNEL_EVENT_HANDLER_LINUX_H_
#define DARWINN_DRIVER_KERNEL_KERNEL_EVENT_HANDLER_LINUX_H_

#include "driver/kernel/kernel_event_handler.h"
#include "port/fileio.h"
#include "port/status.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Implements a mechanism for processing kernel events.
class KernelEventHandlerLinux : public KernelEventHandler {
 public:
  KernelEventHandlerLinux(const std::string& device_path, int num_events);

 private:
  // Maps the specified event number with the specified id.
  util::Status SetEventFd(FileDescriptor fd, FileDescriptor event_fd,
                          int event_id) const override;

  // Performs platform specific event object handle initialization
  FileDescriptor InitializeEventFd(int event_id) const override;

  // Releases platform specific resources associated with event object handle
  util::Status ReleaseEventFd(FileDescriptor fd, FileDescriptor event_fd,
                              int event_id) const override;

  // Creates platform specific KernelEvent backing object
  std::unique_ptr<KernelEvent> CreateKernelEvent(
      FileDescriptor event_fd, KernelEvent::Handler handler) override;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_KERNEL_KERNEL_EVENT_HANDLER_LINUX_H_
