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

#ifndef DARWINN_API_DRIVER_H_
#define DARWINN_API_DRIVER_H_

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "api/buffer.h"
#include "api/package_reference.h"
#include "api/request.h"
#include "api/telemeter_interface.h"
#include "api/timing.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {
namespace api {

// DarwiNN driver. Thread-safe, but not all functions can be called in
// callback context.
//
// Typical usage:
// Driver driver = driverFactory.get();
//
// m1 = driver.RegisterExecutable(<path to executable file>)
// m2 = driver.RegisterExecutable(<path to executable file>)
//
// driver.Open();
// r1 = driver.CreateRequest(m1, done_callback);
// r2 = driver.CreateRequest(m1, done_callback);
// driver.Submit(r1);
// driver.Submit(r2).
// driver.Close();
//...
// After some time, application can try to re-open the driver again.
// driver.Open();
// ...
// driver.Close();
class Driver {
 public:
  // Callback for thermal warnings. Set with SetThermalWarningCallback().
  using ThermalWarningCallback = std::function<void()>;

  // Callback for fatal, unrecoverable failure. Set with
  // SetFatalErrorCallback().
  using FatalErrorCallback = std::function<void(const Status&)>;

  // Driver options. Opaque pointer to an options::Options FB object.
  using Options = std::vector<uint8_t>;

  // Current driver option version. Should match the version in
  // driver_options.fbs.
  static constexpr int kOptionsVersion = 1;

  // Specifies how driver should be closed.
  enum class ClosingMode {
    // Lets the active requests (the ones that have started DMA) finish and
    // cancels pending requests. This may take a few milliseconds.
    kGraceful = 0,

    // Cancels all active and pending requests. This is the fastest way we can
    // close the driver without risk of crashing.
    kAsap = 1,
  };

  // Specifies the way a model is preferred to be ran in terms of power/
  // performance trade-off. This can mapped to equivalent settings in higher
  // level APIs (e.g. PreferenceCode in NNAPI). Please note that the enum
  // integer values may be different from those in NNAPI or other APIs. The
  // values here are defined in the order of priority when there are multiple
  // models requesting different preferences (e.g. sustained speed takes
  // priority over low power).
  enum class ExecutionPreference {
    // Run at the absolute maximum performance.
    kSingleFastAnswer = 0,

    // Ideal for cases in which we are trying to optimize for power.
    kLowPower = 1,

    // Run at the maximum performance but in a way that does not require power /
    // thermal throttling in the long run.
    kSustainedSpeed = 2,
  };

  // Encapsulates different TPU (and related components) operational settings
  // that can impact runtime behavior.
  struct OperationalSettings {
    // TPU clock-rate in hertz.
    int64 tpu_frequency_hz;

    // Data transfer bandwidth between host DRAM and TPU in bytes per second.
    int64 host_to_tpu_bps;
  };

  Driver() = default;
  virtual ~Driver() = default;

  // This class is neither copyable nor movable.
  Driver(const Driver&) = delete;
  Driver& operator=(const Driver&) = delete;

  // Returns true if the driver is open state.
  virtual bool IsOpen() const = 0;

  // Returns true if underlying hardware is in an error state.
  virtual bool IsError() const = 0;

  // Registers a file containing pre-compiled DarwiNN executable and returns a
  // reference to the registered executable. The reference can be used for
  // constructing requests later on.
  virtual StatusOr<const PackageReference*> RegisterExecutableFile(
      const std::string& executable_filename) = 0;

  // Registers a string with serialized contents of a pre-compiled DarwiNN
  // executable and returns a reference to the registered executable. The
  // reference can be used for constructing requests later on.
  virtual StatusOr<const PackageReference*> RegisterExecutableSerialized(
      const std::string& executable_content) = 0;
  virtual StatusOr<const PackageReference*> RegisterExecutableSerialized(
      const char* executable_content, size_t length) = 0;

  // Unregisters a previously registered model.
  virtual Status UnregisterExecutable(
      const PackageReference* executable_ref) = 0;

  // Opens and initializes the underlying hardware. If debug_mode is true,
  // the hardware is setup for use with a debugger. If context_lost is true
  // driver assumes all data on chip (e.g. on DRAM) a from previous open has
  // been lost.
  virtual Status Open(bool debug_mode = false, bool context_lost = false) = 0;

  // Creates a request object initialized with the given ExecutableReference.
  virtual StatusOr<std::shared_ptr<Request>> CreateRequest(
      const PackageReference* executable_ref) = 0;

  // Submits a request for asynchronous execution. On success, done_callback
  // will eventually be executed with the request status. The caller is expected
  // to exit the done_callback as soon as possible. It is acceptable to only
  // call #Submit() in the context of this callback.
  virtual Status Submit(std::shared_ptr<Request> request,
                        Request::Done done_callback) = 0;

  // Executes a request synchronously. Calling thread will block until execution
  // is complete.
  virtual Status Execute(std::shared_ptr<Request> request) = 0;

  // Executes a series of requests synchronously in the given order. Calling
  // thread will block until execution is complete.
  virtual Status Execute(
      const std::vector<std::shared_ptr<Request>>& request) = 0;

  // Attempts to cancel a request. This is best effort cancellation. As in,
  // requests already submitted to the hardware will be allowed to complete.
  // Other requests will be cancelled, and will invoke done_callback with
  // cancelled error.
  virtual Status Cancel(std::shared_ptr<Request> request) = 0;

  // Best effort cancellation of all submitted requests.
  virtual Status CancelAllRequests() = 0;

  // Closes and shutdowns underlying hardware possibly, switching it off.
  // Pending requests are cancelled or completed and callbacks issued. Once
  // closed, requests can no longer be submitted.
  virtual Status Close(ClosingMode mode) = 0;

  // Buffer allocation alignment and granularity.
  // Buffers allocated with this alignment may avoid additional copies within
  // the driver.
  virtual uint64_t allocation_alignment_bytes() const = 0;

  // Allocates size_bytes bytes and returns a Buffer for application use. The
  // allocated memory is tied to the lifecycle of the Buffer object which in
  // turn is tied to the life cycle of the driver instance.
  virtual Buffer MakeBuffer(size_t size_bytes) const = 0;

  // Sets the callback for fatal, unrecoverable failure. When a fatal
  // error is raised, the driver is pushed into an error state. All new
  // submitted requests will fail. Application can generate a bug report and
  // should close the driver, at which point all pending requests will fail and
  // their callbacks executed.
  virtual void SetFatalErrorCallback(FatalErrorCallback callback) = 0;

  // Sets the callback for thermal warnings. Application may be required to
  // to reduce performance level and/or throttle new requests.
  virtual void SetThermalWarningCallback(ThermalWarningCallback callback) = 0;

  // Enters/leaves real-time mode, if applicable. This is best effort as it
  // relies on user provided timing information, and the fact that current
  // generations of DarwiNN is not preemptable.
  virtual Status SetRealtimeMode(bool on) = 0;

  // Sets expected arrival rates and max execution time (in milliseconds) for an
  // package. Only used in real-time mode.
  virtual Status SetExecutableTiming(const api::PackageReference* executable,
                                     const api::Timing& timing) = 0;

  // Sets the provided execution preference for the provided package. Execution
  // preferences are hints to the driver for how to adjust its settings in
  // accordance with power/perf trade-off. Driver will try to keep all requested
  // preferences satisfied erring on the side of performance.
  virtual Status SetExecutionPreference(const api::PackageReference* package,
                                        ExecutionPreference preference) = 0;

  // Sets the perferred telemeter interface. This interface is platform
  // specific. By default, telemetry operations are NOPs. The
  // telemeter_interface is not owned by api::Driver, so the telemeter's
  // lifetime must remain valid as long as the driver object is valid.
  virtual void SetTelemeterInterface(
      api::TelemeterInterface* telemeter_interface) = 0;

  // Updates the operational settings in the driver. This method is to be called
  // when any of these settings change (e.g. due to thermal throttling).
  virtual void UpdateOperationalSettings(
      const OperationalSettings& settings) = 0;

  // TODO: Add function for dumping bugreport.
};

}  // namespace api
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_API_DRIVER_H_
