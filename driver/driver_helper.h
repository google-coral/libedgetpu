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

#ifndef DARWINN_DRIVER_DRIVER_HELPER_H_
#define DARWINN_DRIVER_DRIVER_HELPER_H_

#include <unistd.h>

#include <chrono>              // NOLINT
#include <condition_variable>  // NOLINT
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "api/buffer.h"
#include "api/chip.h"
#include "api/driver.h"
#include "api/package_reference.h"
#include "api/request.h"
#include "api/telemeter_interface.h"
#include "api/timing.h"
#include "driver/executable_util.h"
#include "driver/package_registry.h"
#include "driver/test_vector.h"
#include "executable/executable_generated.h"
#include "port/errors.h"
#include "port/logging.h"
#include "port/ptr_util.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Wraps a driver instance with additional functions that performs tests and
// verify results.
class DriverHelper : public api::Driver {
 public:
  DriverHelper(std::unique_ptr<api::Driver> driver, int max_pending_requests,
               bool prefill_output_tensors = false,
               size_t guard_area_size_bytes = 0);

  // Destructor. Waits for pending tasks to avoid Submit callbacks
  // acquiring otherwise-destructed mutex_. See b/111616778.
  ~DriverHelper() override {
    if (IsOpen()) CHECK_OK(Close(api::Driver::ClosingMode::kGraceful));
  }

  Status Open(bool debug_mode, bool context_lost = false) final
      LOCKS_EXCLUDED(mutex_);

  Status Close(api::Driver::ClosingMode mode) final LOCKS_EXCLUDED(mutex_);

  bool IsOpen() const final LOCKS_EXCLUDED(mutex_);

  bool IsError() const final;

  StatusOr<const api::PackageReference*> RegisterExecutableFile(
      const std::string& executable_filename) final;

  StatusOr<const api::PackageReference*> RegisterExecutableSerialized(
      const std::string& executable_content) final;
  StatusOr<const api::PackageReference*> RegisterExecutableSerialized(
      const char* executable_content, size_t length) final;

  Status UnregisterExecutable(
      const api::PackageReference* executable_ref) final;

  StatusOr<std::shared_ptr<api::Request>> CreateRequest(
      const api::PackageReference* executable_ref) final;

  Status Submit(std::shared_ptr<api::Request> request,
                api::Request::Done done_callback) final;

  Status Execute(std::shared_ptr<api::Request> request) final;

  Status Execute(
      const std::vector<std::shared_ptr<api::Request>>& requests) final;

  Status Cancel(std::shared_ptr<api::Request> request) final;

  Status CancelAllRequests() final;

  uint64_t allocation_alignment_bytes() const final;

  Buffer MakeBuffer(size_t size_bytes) const final;

  void SetFatalErrorCallback(FatalErrorCallback callback) final;

  void SetThermalWarningCallback(ThermalWarningCallback callback) final;

  Status SetExecutionPreference(const api::PackageReference* package,
                                ExecutionPreference preference) final {
    return OkStatus();
  }

  // Extensions to the Device interface to facilitate easier testing.

  // Submits an inference request with given test vector.
  Status Submit(const TestVector& test_vector, int batches)
      LOCKS_EXCLUDED(mutex_);

  // Submits an inference request and execute the specified callback on
  // completion. |tag| is a user friendly name for tracking this request
  // (typically the model name).
  Status Submit(const std::string& tag,
                const api::PackageReference* executable_ref,
                const Buffer::NamedMap& input, const Buffer::NamedMap& output,
                const Buffer::NamedMap& output_with_guard_areas,
                api::Request::Done request_done) LOCKS_EXCLUDED(mutex_);

  // Submits an inference request and verify output, with optional guard area
  // sorrounding the output buffers. Dumps the output upon mismatch, if
  // output_file_name is not empty.
  Status Submit(
      const std::string& tag, const api::PackageReference* executable_ref,
      const std::string& output_file_name, const Buffer::NamedMap& input,
      const Buffer::NamedMap& expected_output, const Buffer::NamedMap& output,
      const Buffer::NamedMap& output_with_guard_areas) LOCKS_EXCLUDED(mutex_);

  // Submits an inference request and verify output.
  Status Submit(const std::string& tag,
                const api::PackageReference* executable_ref,
                const Buffer::NamedMap& input,
                const Buffer::NamedMap& expected_output,
                const Buffer::NamedMap& output) LOCKS_EXCLUDED(mutex_);

  Status SetRealtimeMode(bool on) override;

  Status SetExecutableTiming(const api::PackageReference* executable,
                             const api::Timing& timing) override;

  void SetTelemeterInterface(
      api::TelemeterInterface* telemeter_interface) override {}

  void UpdateOperationalSettings(const OperationalSettings& settings) override {
    driver_->UpdateOperationalSettings(settings);
  }

 private:
  // Wrapped driver instance.
  std::unique_ptr<api::Driver> driver_;

  // Maximum number of pending requests.
  const int max_pending_requests_{1};

  // Current number of pending requests.
  int pending_requests_ GUARDED_BY(mutex_){0};

  // Total number of requests processed so far.
  int total_requests_ GUARDED_BY(mutex_){0};

  // Condition variable to synchronously wait for pending requests.
  std::condition_variable cv_ GUARDED_BY(mutex_);

  // Guards pending_requests_, cv_ and other internal states.
  mutable std::mutex mutex_;

  // Time at which first submit was called.
  std::chrono::steady_clock::time_point first_submit_;

  // A vector of roundtrip times for all requests in milliseconds. Roundtrip
  // time is measured from when driver::submit is called until the callback is
  // first received.
  std::vector<double> roundtrip_times_ms_;

  // A vector of verification times for all requests in milliseconds.
  // Verification time is measured from when the callback is first received
  // until the callback is completed.
  std::vector<double> verification_times_ms_;

  // If true, the output tensors are pre-filled with known data pattern.
  // This helps catch incomplete output activations, i.e. when any parts of the
  // output memory region are not overwritten.
  const bool prefill_output_tensors_;

  // If non-zero, leading and trailing guard areas would be allocated for every
  // output buffer, and filled with known data pattern. These guard areas would
  // then be checked when a request is completed, to detect data overflow.
  // The size should be page-aligned for PCIe use cases.
  // Note that in cases the driver always makes a copy of the output buffers,
  // this mechanism would only catch driver-caused overruns.
  const size_t guard_area_size_bytes_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_DRIVER_HELPER_H_
