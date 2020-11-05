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

#include "driver/beagle/beagle_kernel_top_level_handler.h"

#include "api/driver_options_generated.h"
#include "driver/beagle/beagle_ioctl.h"
#include "port/errors.h"
#include "port/logging.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

BeagleKernelTopLevelHandler::BeagleKernelTopLevelHandler(
    const std::string &device_path, api::PerformanceExpectation performance)
    : device_path_(device_path), performance_(performance) {}

util::Status BeagleKernelTopLevelHandler::DisableSoftwareClockGate() {
  StdMutexLock lock(&mutex_);

  if (!clock_gated_) {
    return util::Status();  // OK
  }

  apex_gate_clock_ioctl ioctl_buffer;
  memset(&ioctl_buffer, 0, sizeof(ioctl_buffer));
  ioctl_buffer.enable = 0;

  if (ioctl(fd_, APEX_IOCTL_GATE_CLOCK, &ioctl_buffer) != 0) {
    return util::FailedPreconditionError(StringPrintf(
        "Could not Disable Clock Gating : %d (%s)", fd_, strerror(errno)));
  }

  clock_gated_ = false;

  return util::Status();  // OK
}

util::Status BeagleKernelTopLevelHandler::EnableSoftwareClockGate() {
  StdMutexLock lock(&mutex_);

  if (clock_gated_) {
    return util::Status();  // OK
  }

  apex_gate_clock_ioctl ioctl_buffer;
  memset(&ioctl_buffer, 0, sizeof(ioctl_buffer));
  ioctl_buffer.enable = 1;

  if (ioctl(fd_, APEX_IOCTL_GATE_CLOCK, &ioctl_buffer) != 0) {
    return util::FailedPreconditionError(
        StringPrintf("Could not Clock Gate : %d (%s)", fd_, strerror(errno)));
  }

  clock_gated_ = true;

  return util::Status();  // OK
}

util::Status BeagleKernelTopLevelHandler::Open() {
  StdMutexLock lock(&mutex_);
  if (fd_ != INVALID_FD_VALUE) {
    return util::FailedPreconditionError("Device already open.");
  }

  fd_ = open(device_path_.c_str(), O_RDWR);
  if (fd_ < 0) {
    return util::FailedPreconditionError(
        StringPrintf("Device open failed : %d (%s)", fd_, strerror(errno)));
  }

  return util::Status();  // OK
}

util::Status BeagleKernelTopLevelHandler::Close() {
  StdMutexLock lock(&mutex_);
  if (fd_ == INVALID_FD_VALUE) {
    return util::FailedPreconditionError("Device not open.");
  }

  close(fd_);
  fd_ = INVALID_FD_VALUE;

  return util::Status();  // OK
}

util::Status BeagleKernelTopLevelHandler::QuitReset() {
  apex_performance_expectation_ioctl ioctl_buffer;
  memset(&ioctl_buffer, 0, sizeof(ioctl_buffer));

  switch (performance_) {
    case api::PerformanceExpectation_Low:
      ioctl_buffer.performance = APEX_PERFORMANCE_LOW;
      break;

    case api::PerformanceExpectation_Medium:
      ioctl_buffer.performance = APEX_PERFORMANCE_MED;
      break;

    case api::PerformanceExpectation_High:
      ioctl_buffer.performance = APEX_PERFORMANCE_HIGH;
      break;

    case api::PerformanceExpectation_Max:
      ioctl_buffer.performance = APEX_PERFORMANCE_MAX;
      break;

    default:
      return util::InvalidArgumentError(
          StringPrintf("Bad performance setting %d.", performance_));
  }

  StdMutexLock lock(&mutex_);
  if (ioctl(fd_, APEX_IOCTL_PERFORMANCE_EXPECTATION, &ioctl_buffer) != 0) {
    LOG(WARNING) << StringPrintf(
        "Could not set performance expectation : %d (%s)", fd_,
        strerror(errno));
  }

  return util::Status();  // OK
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
