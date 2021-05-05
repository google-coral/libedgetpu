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

#ifndef DARWINN_DRIVER_REQUEST_H_
#define DARWINN_DRIVER_REQUEST_H_

#include "api/request.h"
#include "driver/tpu_request.h"
#include "driver_shared/time_stamper/time_stamper.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {
namespace driver {

// This class represents a top level inference request that is created by the
// runtime user. It may have an arbitrary batch size. Its responsibility is to
// sanity check the request and populate TPU requests that can be sent to the
// device as well as tracking their completion.
//
// This is a stateful class. Here's the execution pattern.
//   1. Construction (state: kInitial)
//   2. AddInput() and AddOutput() (can be multiple times) from API interface.
//      (state: kInitial)
//   3. SetDone() in runtime (state: kInitial)
//   4. Prepare() in runtime (kInitial state changes to kPrepared).
//   5. PrepareTpuRequest() in runtime as many times as
//      RequiredTpuRequestCount() (State: kPrepared).
//   6. Done callback is called when request finishes (state changes from
//      // kPrepared to kDone).
class Request : public api::Request {
 public:
  // Constructs a request provided a unique ID and a reference to the package,
  // and an interface to get current timestamps in nanoseconds.
  Request(int id, const PackageReference& package_ref,
          const driver_shared::TimeStamper& timestamper);

  // This class is not copyable nor movable.
  Request(const Request&) = delete;
  Request& operator=(const Request&) = delete;

  // Adds an input buffer. Please refer to the API documentation for more info.
  Status AddInput(const std::string& name, const Buffer& input) override
      LOCKS_EXCLUDED(mutex_);

  // Adds an output buffer. Please refer to the API documentation for more info.
  Status AddOutput(const std::string& name, Buffer output) override
      LOCKS_EXCLUDED(mutex_);

  Status SetPriority(int priority) override LOCKS_EXCLUDED(mutex_);

  // Returns the unique ID of this request.
  int id() const override { return id_; }

  // Returns the timing information of this request. Please refer to the API
  // documentation for more info.
  StatusOr<Timing> GetTiming() const override LOCKS_EXCLUDED(mutex_);

  // Returns a reference to the executable this request belongs to.
  const ExecutableReference& MainExecutableReference() const {
    return main_executable_ref_;
  }

  const PackageReference& GetPackageReference() const { return package_ref_; }

  int GetPriority() const LOCKS_EXCLUDED(mutex_);

  // Sets the done callback function. This function is called the request has
  // finished execution.
  Status SetDone(Done done) LOCKS_EXCLUDED(mutex_);

  // Prepares the request to be broken down to TPU requests. This should be
  // called after we are through adding input/outputs, and have called the
  // SetDone() function.
  Status Prepare() LOCKS_EXCLUDED(mutex_);

  // Returns the number of TPU requests that are needed to be prepared and
  // submitted for this request to be fully carried out.
  StatusOr<int> RemainingTpuRequestCount() const LOCKS_EXCLUDED(mutex_);

  // Sets the input/output buffers and callback of the provided TPU request
  // based on the input/output buffers in this request. Can only be called after
  // Prepare(). It needs to be called as many times as RequiredTpuRequestCount()
  // to ensure that TPU requests for all batch elements are created.
  Status PrepareTpuRequest(std::shared_ptr<TpuRequest> tpu_request)
      LOCKS_EXCLUDED(mutex_);

  // Notifies the request that a part (or all) of it has been submitted to the
  // hardware.
  void NotifySubmission(TpuRequest::RequestType) LOCKS_EXCLUDED(mutex_);

  // Notifies the request that a part (or all) of it has completed execution on
  // the hardware.
  void NotifyCompletion(TpuRequest::RequestType) LOCKS_EXCLUDED(mutex_);

  // Number of estimated cycles it takes for a single TpuRequest of this request
  // to take in order to run on TPU (only applies to execution requests, and not
  // parameter caching).
  int64 EstimatedCyclesPerInference() const {
    return GetPackageReference().MainExecutableReference()->EstimatedCycles();
  }

  // Marks num_requests_done pending TpuRequests of this request as done with
  // the provided status. It executes the done callback if all TPU requests are
  // done at this point.
  Status HandleTpuRequestsDone(const Status& status, int num_requests_done)
      LOCKS_EXCLUDED(mutex_);

 private:
  // An enum to specify the state of a request.
  enum State {
    kInitial,   // Input and outputs are still being added.
    kPrepared,  // Buffers are all added, done callback is set, and Prepare()
                // function is complete.
    kDone,      // All TPU requests are finished.
  };

  // Sets the state of the request. Returns an error for an illegal transition.
  Status SetState(State next_state) EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Verifies that the current state is equal to the provided state.
  Status ValidateState(State state) const EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Prepares a single TPU request for a request that has no input/outputs.
  Status PrepareNoIORequest(std::shared_ptr<TpuRequest> tpu_request)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Sets the input/output buffers and callback of the provided TPU request
  // based on the input/output buffers in this request.
  Status PrepareIORequest(std::shared_ptr<TpuRequest> tpu_request)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Gets called on every TPU request callback.
  void TpuRequestDone(int id, const Status& status) LOCKS_EXCLUDED(mutex_);

  // The unique ID of this request.
  const int id_;

  // A reference to the package this request is tied to.
  const PackageReference& package_ref_;

  // The main executable reference this request needs to execute.
  const ExecutableReference& main_executable_ref_;

  // Number of individual inferences that can be run in a single request to TPU.
  // This is also referred to as data-parallelism.
  const int hardware_batch_size_;

  // Maintains integrity of the request object.
  mutable std::mutex mutex_;

  // Current state of the request.
  State state_ GUARDED_BY(mutex_) = kInitial;

  // The batch size of this request (no batching = 1). This field is valid only
  // on kPrepared state and after.
  int request_batch_size_ GUARDED_BY(mutex_);

  // Number of requests that runtime needs to make to TPU in order to process
  // the entire request_batch_size_. This field is valid only on kPrepared state
  // and after.
  int required_tpu_request_count_ GUARDED_BY(mutex_);

  // All input buffers in this request (name->batch_index->buffer).
  Buffer::NamedMap inputs_ GUARDED_BY(mutex_);

  // All output buffers in this request (name->batch_index->buffer).
  Buffer::NamedMap outputs_ GUARDED_BY(mutex_);

  // Final request completion callback.
  Done done_ GUARDED_BY(mutex_);

  // Number of tpu requests we are waiting for to finish.
  int pending_tpu_requests_ GUARDED_BY(mutex_) = 0;

  // Stores the request done status. Each tpu_request done status updates this.
  Status done_status_ GUARDED_BY(mutex_);

  // Gets the current time in nanoseconds.
  const driver_shared::TimeStamper& current_time_;

  // Timing information of this request.
  Timing timing_;

  // The scheduling priority of this request with respect to others. 0 is
  // highest priority and the larger the number the lower the priority. Negative
  // priorities are invalid.
  int priority_ GUARDED_BY(mutex_) = 0;

  // Number of tpu requests that are already prepared. This field will max out
  // on required_tpu_request_count_ and only after then the entire request will
  // be completed.
  int tpu_requests_prepared_ GUARDED_BY(mutex_) = 0;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_REQUEST_H_
