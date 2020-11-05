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

#ifndef DARWINN_DRIVER_KERNEL_WINDOWS_KERNEL_EVENT_WINDOWS_H_
#define DARWINN_DRIVER_KERNEL_WINDOWS_KERNEL_EVENT_WINDOWS_H_

#include <mutex>   // NOLINT
#include <thread>  // NOLINT

#include "driver/kernel/kernel_event.h"
#include "port/fileio.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Monitors events generated through eventfd. The eventfd file
// descriptor passed through the constructor must already be open
// and associated with an event source. Monitoring starts
// on instance creation and stops on destroy.
class KernelEventWindows : public KernelEvent {
 public:
  KernelEventWindows(FileDescriptor event_fd, Handler handler);
  ~KernelEventWindows() override;

  // This class is neither copyable nor movable.
  KernelEventWindows(const KernelEvent&) = delete;
  KernelEventWindows& operator=(const KernelEvent&) = delete;

 private:
  // Monitors eventfd_. Runs on thread_.
  void Monitor(const Handler& handler);

  // Convenience function to read |enabled_| with locks held.
  bool IsEnabled() const LOCKS_EXCLUDED(mutex_);

  // Event fd.
  const FileDescriptor event_fd_;

  // Mutex that guards enabled_;
  mutable std::mutex mutex_;

  // Enabled if true.
  bool enabled_ GUARDED_BY(mutex_){true};

  // Thread for monitoring interrupts.
  std::thread thread_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_KERNEL_WINDOWS_KERNEL_EVENT_WINDOWS_H_
