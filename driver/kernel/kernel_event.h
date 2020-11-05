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

#ifndef DARWINN_DRIVER_KERNEL_KERNEL_EVENT_H_
#define DARWINN_DRIVER_KERNEL_KERNEL_EVENT_H_

#include <functional>

#include "port/fileio.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Monitors events generated through eventfd. The eventfd file
// descriptor passed through the constructor must already be open
// and associated with an event source. Monitoring starts
// on instance creation and stops on destroy.
class KernelEvent {
 public:
  using Handler = std::function<void()>;

  KernelEvent(FileDescriptor event_fd, Handler handler) {}
  virtual ~KernelEvent() = default;

  // This class is neither copyable nor movable.
  KernelEvent(const KernelEvent&) = delete;
  KernelEvent& operator=(const KernelEvent&) = delete;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_KERNEL_KERNEL_EVENT_H_
