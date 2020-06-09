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

#ifndef DARWINN_DRIVER_TPU_REQUEST_H_
#define DARWINN_DRIVER_TPU_REQUEST_H_

#include <functional>
#include <list>
#include <string>

#include "api/buffer.h"
#include "api/request.h"
#include "driver/dma_info.h"
#include "driver/package_registry.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {
namespace driver {

// An abstract class to represent an inference request to TPU.
class TpuRequest {
 public:
  // A type for request completion callback.
  // The int argument is the same as return value of id().
  using Done = std::function<void(int, const util::Status&)>;

  // Classify each TPU Request for logging.
  using RequestType = api::Request::TimingEvent::TpuRequestType;

  TpuRequest() = default;
  virtual ~TpuRequest() = default;

  // This class is not copyable nor movable.
  TpuRequest(const TpuRequest&) = delete;
  TpuRequest& operator=(const TpuRequest&) = delete;

  // Sets the callback function executed when request is complete.
  virtual util::Status SetDone(Done done) = 0;

  // Adds an input or output buffer. This may be called repeatedly depending
  // on the batch size as long as the request instance is not submitted. If
  // supplied "name" does not exist or size constraints on the input and output
  // buffers do not match executable, will return failure. Memory backing the
  // |Buffer| instance must be valid throughout the life of the request.
  virtual util::Status AddInput(const std::string& name,
                                const Buffer& input) = 0;
  virtual util::Status AddOutput(const std::string& name, Buffer output) = 0;

  // Add a provided number of dummy input/output buffers. This is helpful for
  // evening out the number of buffers to native batch size.
  virtual util::Status AddNoopInputs(const std::string& name, int count) = 0;
  virtual util::Status AddNoopOutputs(const std::string& name, int count) = 0;

  // Returns the input and output buffers that the TPU DMAs to. This is only for
  // use in reference driver and similar.
  virtual const Buffer& InputBuffer(const std::string& name,
                                    int batch) const = 0;
  virtual Buffer OutputBuffer(const std::string& name, int batch) const = 0;

  // Validates the constraints.
  virtual util::Status Validate() = 0;

  // Prepares the request to be submitted.
  virtual util::Status Prepare() = 0;

  // Cancels the pending request. Cancellation is best effort. Completion
  // callback is called if not already. Canceling a completed request has
  // no effect.
  // Note: A single TpuRequest cancelation will not cause an immediate
  // cancellation of the parent driver::Request. However, it will guarantee a
  // cancellation status once the parent request calls its Done callback.
  virtual util::Status Cancel() = 0;

  // Notifies that request is submitted to the driver, but not yet issued.
  virtual util::Status NotifyRequestSubmitted() = 0;

  // Notifies that request is active. That is, request is issued to DarwiNN.
  virtual util::Status NotifyRequestActive() = 0;

  // Notifies completion of the request with the given status.
  virtual util::Status NotifyCompletion(util::Status status) = 0;

  // Returns request id.
  virtual int id() const = 0;

  // Returns the TPU request type that is used for logging.
  virtual RequestType type() const = 0;

  // Returns the number of instruction bitstream chunks.
  virtual int num_instruction_bitstream_chunks() const = 0;

  // Returns a list of DMAs to be performed.
  virtual util::StatusOr<std::list<DmaInfo>> GetDmaInfos() const = 0;

  // Returns executable reference.
  virtual const ExecutableReference& executable_reference() const = 0;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_TPU_REQUEST_H_
