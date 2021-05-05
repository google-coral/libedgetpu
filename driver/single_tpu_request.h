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

#ifndef DARWINN_DRIVER_SINGLE_TPU_REQUEST_H_
#define DARWINN_DRIVER_SINGLE_TPU_REQUEST_H_

#include <stddef.h>

#include <functional>
#include <list>
#include <memory>
#include <mutex>  // NOLINT
#include <string>
#include <unordered_map>
#include <vector>

#include "api/buffer.h"
#include "driver/allocator.h"
#include "driver/device_buffer.h"
#include "driver/device_buffer_mapper.h"
#include "driver/dma_info.h"
#include "driver/dma_info_extractor.h"
#include "driver/executable_util.h"
#include "driver/memory/address_space.h"
#include "driver/memory/dram_allocator.h"
#include "driver/package_registry.h"
#include "driver/request.h"
#include "driver/tpu_request.h"
#include "executable/executable_generated.h"
#include "port/array_slice.h"
#include "port/integral_types.h"
#include "port/statusor.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// An single inference request to TPU. This class is thread-safe.
class SingleTpuRequest : public TpuRequest {
 public:
  // Constructs a request object for executing the given |executable_reference|.
  // |done| is the callback function executed when request is complete.
  // TODO: Make this constructor private and create request objects
  // through a factory function in the driver.
  explicit SingleTpuRequest(
      int id, const std::shared_ptr<Request> parent_request,
      const ExecutableReference* executable_reference, Allocator* allocator,
      DramAllocator* dram_allocator,
      std::unique_ptr<DeviceBufferMapper> device_buffer_mapper,
      const DmaInfoExtractor* extractor, uint64 alignment_bytes,
      RequestType type);
  explicit SingleTpuRequest(
      int id, const std::shared_ptr<Request> parent_request,
      const ExecutableReference* executable_reference, Allocator* allocator,
      DramAllocator* dram_allocator,
      std::unique_ptr<DeviceBufferMapper> device_buffer_mapper,
      const DmaInfoExtractor* extractor, uint64 alignment_bytes, Done done,
      RequestType type);

  SingleTpuRequest(SingleTpuRequest&& rhs) = delete;
  SingleTpuRequest& operator=(SingleTpuRequest&& rhs) = delete;
  SingleTpuRequest(const SingleTpuRequest&) = delete;
  SingleTpuRequest& operator=(const SingleTpuRequest&) = delete;
  ~SingleTpuRequest() override;

  Status SetDone(Done done) LOCKS_EXCLUDED(mutex_) override;
  Status AddInput(const std::string& name, const Buffer& input)
      LOCKS_EXCLUDED(mutex_) override;
  Status AddOutput(const std::string& name, Buffer output)
      LOCKS_EXCLUDED(mutex_) override;
  Status AddNoopInputs(const std::string& name, int count)
      LOCKS_EXCLUDED(mutex_) override;
  Status AddNoopOutputs(const std::string& name, int count)
      LOCKS_EXCLUDED(mutex_) override;
  const Buffer& InputBuffer(const std::string& name, int batch) const override
      LOCKS_EXCLUDED(mutex_);
  Buffer OutputBuffer(const std::string& name, int batch) const override
      LOCKS_EXCLUDED(mutex_);
  Status Validate() LOCKS_EXCLUDED(mutex_) override;
  Status Prepare() LOCKS_EXCLUDED(mutex_) override;
  Status Cancel() LOCKS_EXCLUDED(mutex_) override;

  // TODO: The following functions needs to restricted for use
  // by the driver only.
  Status NotifyRequestSubmitted() LOCKS_EXCLUDED(mutex_) override;
  Status NotifyRequestActive() LOCKS_EXCLUDED(mutex_) override;
  Status NotifyCompletion(Status status) LOCKS_EXCLUDED(mutex_) override;

  int id() const override { return id_; }

  RequestType type() const override { return type_; }

  int num_instruction_bitstream_chunks() const override {
    return executable().instruction_bitstreams()->size();
  }

  StatusOr<std::list<DmaInfo>> GetDmaInfos() const
      LOCKS_EXCLUDED(mutex_) override;

  const ExecutableReference& executable_reference() const override {
    return executable_reference_;
  }

  DeviceBufferMapper* device_buffer_mapper() const {
    return device_buffer_mapper_.get();
  }

 private:
  // Compute request state. State transitions :
  //  kUninitialized -> kCreated -> kSubmitted -> kActive -> kDone
  //  kUninitialized -> kCreated -> kSubmitted -> kDone [if cancelled].
  enum State {
    kUninitialized,  // Request not initialized.
    kCreated,        // Request created, but pending issue to DarwiNN.
    kSubmitted,      // Request submitted and in queue for issuing to DarwiNN.
    kActive,         // Request issued to DarwiNN, pending results.
    kDone,           // Request in terminal state.
  };

  // Attempts a state transition to the given state.
  Status SetState(State next_state) EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Validates that we are in the expected state.
  Status ValidateState(State expected_state) const
      SHARED_LOCKS_REQUIRED(mutex_);

  // Maps all data buffers (input, output, parameters) to the DarwiNN address
  // space.
  Status MapDataBuffers() EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Map instruction buffers to the DarwiNN address space.
  Status MapInstructionBuffers() EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Unmaps all buffers and frees the allocated instruction and parameter
  // buffers if any. Reverse of what is done in #Prepare().
  Status Cleanup() EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Convenience function that returns the backing executable in
  // |executable_reference_|.
  const darwinn::Executable& executable() const {
    return executable_reference_.executable();
  }

  // Returns true if the alignment requirement for a provided buffer is met.
  bool IsBufferAligned(const Buffer& buffer);

  // Post processes the output buffers. This includes:
  // 1. Relayout the outputs in host_outputs_ to user-expected layouts and
  //    store them in the user_outputs_. Some outputs do not need a relayout
  //    and for those we set the same user-provided buffer in the host_outputs_.
  //    Those are ignored by this method.
  // 2. Perform sign conversion.
  Status PostProcessOutputBuffers() EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Tries to create a TPU DRAM buffer. If it fails, it falls back to create a
  // host DRAM buffer.
  Buffer TryCreateDramBuffer(size_t size_bytes);

  // Creates and returns a new buffer for |batches| copies of the activations of
  // a provided layer.
  Buffer CreateActivationBuffer(const api::LayerInformation* layer,
                                int batches);

  // Gets a contiguous buffer that holds all batched host_outputs_ for a given
  // layer. Lazily creates the buffer on first access, but always returns the
  // same buffer when called for the same layer.
  Buffer GetOrCreateBatchOutput(const api::LayerInformation* layer,
                                const std::string& name)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Copies a provided input buffer in such a way that inputs of each iteration
  // has the alignment requirements.
  Buffer ScatterInput(const Buffer& input, const api::LayerInformation* layer);

  // Unique ID for request.
  const int id_;

  // Track the type of TPU request for logging purposes.
  const RequestType type_;

  // The parent driver request this TpuRequest is a part of. We mostly hold on
  // to a shared_pointer to the parent here to make sure it outlives all its
  // TPU requests.
  const std::shared_ptr<Request> parent_request_;

  // Executable for the compute request.
  const ExecutableReference& executable_reference_;

  // Buffer allocator.
  Allocator* const allocator_;

  // On-Chip DRAM Buffer allocator.
  DramAllocator* dram_allocator_;

  // Maps and stores all device buffers.
  std::unique_ptr<DeviceBufferMapper> device_buffer_mapper_;

  // DMA info extractor.
  const DmaInfoExtractor& extractor_;

  // Maintains integrity of the request object.
  mutable std::mutex mutex_;

  // Request state.
  State state_ GUARDED_BY(mutex_){kUninitialized};

  // Infeed and outfeed host buffers.
  // host_*[layer_name][batch_id] = buffer.
  Buffer::NamedMap host_inputs_ GUARDED_BY(mutex_);
  Buffer::NamedMap host_outputs_ GUARDED_BY(mutex_);

  // Cache of batch-sized output buffers that are used to ensure
  // host_outputs_ are contiguous.
  std::unordered_map<std::string, Buffer> batch_outputs_ GUARDED_BY(mutex_);

  // Buffers to contain the user-facing outputs. The difference between user and
  // host outputs is that host_outputs have a TPU-friendly layout while user
  // outputs have a user-friendly layout. For DMAs and basically anything from
  // driver down, we mostly deal host_outputs_. And for anything from driver up
  // or methods exposed in the API, we deal with user_outputs_.
  Buffer::NamedMap user_outputs_ GUARDED_BY(mutex_);

  // Final request completion callback.
  Done done_ GUARDED_BY(mutex_);

  // A copy of the mapped parameters owned by executable reference.
  const DeviceBuffer parameter_device_buffer_ GUARDED_BY(mutex_);
  // Buffers for instructions.
  std::unique_ptr<InstructionBuffers> instruction_buffers_ GUARDED_BY(mutex_);

  // The alignment requirement for input and output buffers provided by the
  // user.
  const uint64 alignment_bytes_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_SINGLE_TPU_REQUEST_H_
