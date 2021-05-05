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

#ifndef DARWINN_DRIVER_BEAGLE_BEAGLE_KERNEL_TOP_LEVEL_HANDLER_H_
#define DARWINN_DRIVER_BEAGLE_BEAGLE_KERNEL_TOP_LEVEL_HANDLER_H_

#include <mutex>  // NOLINT

#include "api/driver_options_generated.h"
#include "driver/top_level_handler.h"
#include "port/fileio.h"
#include "port/status.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Handles chip specific resets.
class BeagleKernelTopLevelHandler : public TopLevelHandler {
 public:
  BeagleKernelTopLevelHandler(const std::string &device_path,
                              api::PerformanceExpectation performance);
  ~BeagleKernelTopLevelHandler() override = default;

  // Implements ResetHandler interface.
  Status Open() override;
  Status Close() override;
  Status EnableSoftwareClockGate() override;
  Status DisableSoftwareClockGate() override;
  Status QuitReset() override;

 private:
  // Device path.
  const std::string device_path_;

  // File descriptor of the opened device.
  FileDescriptor fd_ GUARDED_BY(mutex_){INVALID_FD_VALUE};

  // Mutex that guards fd_.
  std::mutex mutex_;

  // Chip starts in clock gated state.
  bool clock_gated_{true};

  // Performance setting.
  const api::PerformanceExpectation performance_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_BEAGLE_BEAGLE_KERNEL_TOP_LEVEL_HANDLER_H_
