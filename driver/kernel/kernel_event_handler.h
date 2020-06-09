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

#ifndef DARWINN_DRIVER_KERNEL_KERNEL_EVENT_HANDLER_H_
#define DARWINN_DRIVER_KERNEL_KERNEL_EVENT_HANDLER_H_

#include <functional>
#include <memory>
#include <mutex>   // NOLINT
#include <string>
#include <thread>  // NOLINT
#include <vector>

#include "driver/kernel/kernel_event.h"
#include "port/status.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Implements a mechanism for processing kernel events.
class KernelEventHandler {
 public:
  KernelEventHandler(const std::string& device_path, int num_events);
  ~KernelEventHandler() = default;

  util::Status Open() LOCKS_EXCLUDED(mutex_);
  util::Status Close() LOCKS_EXCLUDED(mutex_);

  // Registers and enables the specified event.
  util::Status RegisterEvent(int event_id, KernelEvent::Handler handler)
      LOCKS_EXCLUDED(mutex_);

 private:
  // Maps the specified event number with the specified id.
  util::Status SetEventFd(int event_fd, int event_id) const
      SHARED_LOCKS_REQUIRED(mutex_);

  // Device path.
  const std::string device_path_;

  // Number of events.
  const int num_events_;

  // Mutex that guards fd_, event_fd_, interrupts_;
  mutable std::mutex mutex_;

  // File descriptor of the opened device.
  int fd_ GUARDED_BY(mutex_){-1};

  // Event FD list.
  std::vector<int> event_fds_ GUARDED_BY(mutex_);

  // Registered events.
  std::vector<std::unique_ptr<KernelEvent>> events_ GUARDED_BY(mutex_);
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_KERNEL_KERNEL_EVENT_HANDLER_H_
