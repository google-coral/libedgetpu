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

#ifndef DARWINN_API_REQUEST_H_
#define DARWINN_API_REQUEST_H_

#include <functional>
#include <memory>
#include <string>

#include "api/buffer.h"
#include "port/integral_types.h"
#include "port/status_macros.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {
namespace api {

// Compute request. Thread-unsafe.
class Request {
 public:
  // A type for request completion callback.
  // The int argument is the same as return value of id().
  using Done = std::function<void(int, Status)>;

  // Fine grain timing information
  struct TimingEvent {
    // Classify each TPU Request (sub-requests) for logging.
    enum class TpuRequestType {
      PARAMETER_CACHING,  // Request for parameter caching.
      INFERENCE           // Inference request, single hardware batch.
    };

    // Classify the TimingEvents based on what is happening to the TPU Request.
    enum class EventType {
      SUBMITTED,  // The sub-request was submitted.
      COMPLETED   // The sub-request was completed.
    };

    int64 timestamp;              // When the event occurred.
    TpuRequestType request_type;  // Request classification for logging.
    EventType event_type;  // What happened (request creation, completion).

    // In DarwiNN 1.0, requests are sent in order. If that changes in the
    // future, need to add a request_id to correlate events belonging to a
    // single request, while multiple requests are in flight.

    TimingEvent(int64 timestamp, TpuRequestType type, EventType state)
        : timestamp(timestamp),
          request_type(type),
          event_type(state){}
  };

  // Encapsulates timing information of a request.
  struct Timing {
    // Timestamp (in nanoseconds) of when the request was first created.
    int64 created_ns;

    // Timestamp (in nanoseconds) of when the request was submitted to the
    // device for execution. In case of batched requests, this is the time when
    // the first batch element is submitted.
    int64 submitted_ns;

    // Timestamp (in nanoseconds) of when the request was completed in hardware.
    // In case of batched requests, this is the time that the last batch element
    // completed execution.
    int64 completed_ns;

    // Capture finegrain event timestamps for each single_tpu_request
    std::vector<TimingEvent> detail_timing;
  };

  Request() = default;
  virtual ~Request() = default;

  // This class is neither copyable nor movable.
  Request(const Request&) = delete;
  Request& operator=(const Request&) = delete;

  // Adds an input buffer. This may be called repeatedly depending
  // on the batch size as long as the request instance is not submitted. The
  // size constraints on the input and output buffers will be evaluated during
  // Device#Submit. Memory backing the buffer instance must be valid throughout
  // the life of the request.
  // IMPORTANT: For better performance, please make sure input buffers are
  // aligned with at least minimum_alignment_bytes (architecture dependent). If
  // possible use Driver::MakeBuffer to get a buffer with this requirement met.
  // Buffers with and without padding are both acceptable.
  virtual Status AddInput(const std::string& name, const Buffer& input) = 0;

  // Adds an output buffer. This may be called repeatedly depending
  // on the batch size as long as the request instance is not submitted. The
  // size constraints on the input and output buffers will be evaluated during
  // Device#Submit. Memory backing the buffer instance must be valid throughout
  // the life of the request.
  //
  // If the output buffer is user-allocated on-device DRAM, the model must
  // ensure that no post-processing will be needed for this output, such as
  // re-layout or sign processing.
  // TODO -- the API implementation does not currently validate
  // that no post-processing will be needed for a user-allocated on-device DRAM
  // output.
  virtual Status AddOutput(const std::string& name, Buffer output) = 0;

  // Sets the scheduling priority of this request (must be a positive int) where
  // 0 is highest priority. P0 requests are immediately scheduled for execution
  // while lower priorities (higher in value) may get preempted if device is
  // busy. By default, a request is P0.
  virtual Status SetPriority(int priority) = 0;

  // Returns timing information of this request. It can only be called when the
  // request is done.
  virtual StatusOr<Timing> GetTiming() const = 0;

  // Returns an ID to track the request.
  virtual int id() const = 0;
};

}  // namespace api
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_API_REQUEST_H_
