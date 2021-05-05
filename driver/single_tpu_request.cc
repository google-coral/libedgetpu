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

#include "driver/single_tpu_request.h"

#include <vector>

#include "api/allocated_buffer.h"
#include "api/buffer.h"
#include "driver/allocator.h"
#include "driver/executable_util.h"
#include "driver/hardware_structures.h"
#include "driver/instruction_buffers.h"
#include "driver/memory/address_space.h"
#include "driver/package_registry.h"
#include "driver/request.h"
#include "executable/executable_generated.h"
#include "port/array_slice.h"
#include "port/cleanup.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/logging.h"
#include "port/macros.h"
#include "port/ptr_util.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/statusor.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"
#include "port/tracing.h"

namespace platforms {
namespace darwinn {
namespace driver {

using ::flatbuffers::VectorLength;

SingleTpuRequest::SingleTpuRequest(
    int id, const std::shared_ptr<Request> parent_request,
    const ExecutableReference* executable_reference, Allocator* allocator,
    DramAllocator* dram_allocator,
    std::unique_ptr<DeviceBufferMapper> device_buffer_mapper,
    const DmaInfoExtractor* extractor, uint64 alignment_bytes, Done done,
    RequestType type)
    : id_(id),
      type_(type),
      parent_request_(parent_request),
      executable_reference_(*[executable_reference]() {
        CHECK(executable_reference != nullptr);
        return executable_reference;
      }()),
      allocator_([allocator]() {
        CHECK(allocator != nullptr);
        return allocator;
      }()),
      dram_allocator_([dram_allocator]() {
        CHECK(dram_allocator != nullptr);
        return dram_allocator;
      }()),
      device_buffer_mapper_(std::move(device_buffer_mapper)),
      extractor_(*[extractor]() {
        CHECK(extractor != nullptr);
        return extractor;
      }()),
      done_(std::move(done)),
      parameter_device_buffer_(
          executable_reference_.GetParameterDeviceBuffer()),
      alignment_bytes_(alignment_bytes) {
  VLOG(5) << StringPrintf("[%d] Request constructed.", id_);
}

SingleTpuRequest::SingleTpuRequest(
    int id, const std::shared_ptr<Request> parent_request,
    const ExecutableReference* executable_reference, Allocator* allocator,
    DramAllocator* dram_allocator,
    std::unique_ptr<DeviceBufferMapper> device_buffer_mapper,
    const DmaInfoExtractor* extractor, uint64 alignment_bytes,
    RequestType type)
    : SingleTpuRequest(id, parent_request, executable_reference, allocator,
                       dram_allocator, std::move(device_buffer_mapper),
                       extractor, alignment_bytes,
                       /*done=*/nullptr, type) {}

SingleTpuRequest::~SingleTpuRequest() {
  VLOG(5) << StringPrintf("[%d] Request destroyed.", id_);
  CHECK_OK(Cleanup());
}

Status SingleTpuRequest::SetDone(Done done) {
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateState(kUninitialized));
  done_ = std::move(done);
  return OkStatus();
}

Status SingleTpuRequest::AddInput(const std::string& name,
                                  const Buffer& user_input) {
  TRACE_SCOPE("SingleTpuRequest::AddInput");
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateState(kUninitialized));
  RETURN_IF_ERROR(executable_reference_.ValidateInput(name, user_input));
  VLOG(3) << StringPrintf("Adding input \"%s\" with %zu bytes.", name.c_str(),
                          user_input.size_bytes());

  ASSIGN_OR_RETURN(const auto* layer, executable_reference_.InputLayer(name));
  Buffer host_input = user_input;

  // For iterative models, we need to add padding after each iteration.
  if (layer->execution_count_per_inference() > 1 &&
      host_input.size_bytes() != layer->PaddedSizeBytes()) {
    if (user_input.IsDramType())
      return UnimplementedError(
          "DRAM input buffers currently do not support "
          "execution_count_per_inference > 1");
    host_input = ScatterInput(user_input, layer);
  }

  if (layer->SignedDataType()) {
    if (user_input.IsDramType())
      return UnimplementedError(
          "DRAM input buffers currently do not support "
          "signed data type");
    RETURN_IF_ERROR(layer->TransformSignedDataType(host_input));
  }

  // If this buffer needs to be cached on TPU DRAM, we should replace it with a
  // DRAM buffer and copy the contents. If DRAM buffer allocation fails, we will
  // carry on with the same host DRAM buffer.
  if (layer->CacheOnDram() && !user_input.IsDramType()) {
    TRACE_SCOPE("SingleTpuRequest::AddInput::AddDRAMBuffer");
    auto buffer_or_error =
        dram_allocator_->AllocateBuffer(layer->PaddedSizeBytes());
    if (buffer_or_error.ok()) {
      auto dram_buffer = buffer_or_error.ValueOrDie();
      RETURN_IF_ERROR(dram_buffer->ReadFrom(host_input.ptr()));
      host_input = Buffer(dram_buffer);
    } else {
      LOG(WARNING) << StringPrintf(
                          "Failed to allocate TPU DRAM buffer of size %d: ",
                          layer->PaddedSizeBytes())
                   << buffer_or_error.status().message();
    }
  }

  // At this point we are about to add host_input to the list of buffers
  // that get mapped to TPU. If it is on host DRAM, we should make sure it is
  // aligned, otherwise copy it to an aligned buffer.
  if (host_input.IsPtrType() && !IsBufferAligned(host_input)) {
    TRACE_SCOPE("SingleTpuRequest::AddInput::CopyForAlignment");
    // From here on, we need to make sure that accessing padding bytes will not
    // cause problems, however the input buffer supplied by the user may not
    // explicitly include padding bytes. To avoid always copying the input
    // buffer, instead we ensure that reading memory slightly past the end of
    // what was supplied by the user is safe and not going to page fault.

    // If the provided buffer is aligned, that implies that the padded end is
    // also aligned, and therefore the padding bytes cannot cross a page
    // boundary. So we can use it directly and avoid paying for a memcpy.
    // (Unless we need to pad in between elements for hardware looping support.)
    auto aligned_input = allocator_->MakeBuffer(layer->PaddedSizeBytes());
    memcpy(aligned_input.ptr(), host_input.ptr(), host_input.size_bytes());
    host_input = aligned_input;
  }

  host_inputs_[name].push_back(host_input);
  return OkStatus();
}

Status SingleTpuRequest::AddOutput(const std::string& name, Buffer output) {
  TRACE_SCOPE("SingleTpuRequest::AddOutput");

  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateState(kUninitialized));
  RETURN_IF_ERROR(executable_reference_.ValidateOutput(name, output));

  VLOG(3) << StringPrintf("Adding output \"%s\" with %zu bytes.", name.c_str(),
                          output.size_bytes());
  ASSIGN_OR_RETURN(const auto* layer, executable_reference_.OutputLayer(name));

  if (output.IsDramType() && !output.IsManagedType()) {
    TRACE_SCOPE("SingleTpuRequest::AddOutput::PushToHostOutput");
    // Handle special case for user-created on-device DRAM buffer.
    // 1. Use the user-provided buffer directly for model output.
    // 2. There is no separate user-buffer to synchronize output data with.
    // 3. There will be no opportunity for post-processing, e.g., re-layout.
    //    Therefore, we do not accept a user-created on-device DRAM buffer
    //    that needs post-processing.

    // TODO -- When the proper test is implemented, use it to
    // validate that this output buffer does not in fact need
    // post-processing.

    host_outputs_[name].push_back(output);
  } else {
    TRACE_SCOPE("SingleTpuRequest::AddOutput::CreateTmpAndPushToHostOutput");
    // In all other cases, create a temporary buffer in host memory
    // for the model output. The temporary output will need to be
    // synchronized (potentially after post-processing) with the
    // actual user-provided buffer.

    auto host_output =
        GetOrCreateBatchOutput(layer, name)
            .Slice(user_outputs_[name].size() * layer->PaddedSizeBytes(),
                   layer->PaddedSizeBytes());
    host_outputs_[name].push_back(std::move(host_output));
  }

  user_outputs_[name].push_back(std::move(output));

  return Status();  // OK
}

Status SingleTpuRequest::AddNoopInputs(const std::string& name, int count) {
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateState(kUninitialized));
  VLOG(3) << StringPrintf("Adding %d noop inputs for layer \"%s\".", count,
                          name.c_str());

  ASSIGN_OR_RETURN(const auto* layer, executable_reference_.InputLayer(name));
  auto& inputs = host_inputs_[name];
  inputs.reserve(count);

  auto batch_buffer = CreateActivationBuffer(layer, count);
  for (int i = 0; i < count; ++i) {
    auto buffer = batch_buffer.Slice(i * layer->PaddedSizeBytes(),
                                     layer->PaddedSizeBytes());
    inputs.push_back(buffer);
  }

  return OkStatus();
}

Status SingleTpuRequest::AddNoopOutputs(const std::string& name, int count) {
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateState(kUninitialized));
  VLOG(3) << StringPrintf("Adding %d noop outputs for layer \"%s\".", count,
                          name.c_str());

  ASSIGN_OR_RETURN(const auto* layer, executable_reference_.OutputLayer(name));
  auto& outputs = host_outputs_[name];
  outputs.reserve(count);

  const auto& batch_buffer = GetOrCreateBatchOutput(layer, name);
  const int total_batches = executable_reference_.BatchSize();
  for (int i = total_batches - count; i < total_batches; ++i) {
    auto buffer = batch_buffer.Slice(i * layer->PaddedSizeBytes(),
                                     layer->PaddedSizeBytes());
    outputs.push_back(buffer);
  }

  return OkStatus();
}

Status SingleTpuRequest::MapDataBuffers() {
  // Map activations except parameters, which is done at registration time.
  TRACE_SCOPE("Request::MapDataBuffers");
  RETURN_IF_ERROR(
      device_buffer_mapper_->MapScratch(executable_reference_.scratch()));
  RETURN_IF_ERROR(device_buffer_mapper_->MapInputs(host_inputs_));
  RETURN_IF_ERROR(device_buffer_mapper_->MapOutputs(host_outputs_));
  return Status();  // OK
}

Status SingleTpuRequest::MapInstructionBuffers() {
  TRACE_SCOPE("Request::MapInstructionBuffers");
  RETURN_IF_ERROR(device_buffer_mapper_->MapInstructions(
      instruction_buffers_->GetBuffers()));

  return Status();  // OK
}

Status SingleTpuRequest::Cleanup() {
  // Note that these calls are a no-op if request is already in a clean state.
  RETURN_IF_ERROR(device_buffer_mapper_->UnmapAll());
  if (instruction_buffers_) {
    // Returns the instruction buffers back to executable references, so that
    // we could reuse it in the next request.
    // This saves time allocating / copying new host memory buffers.
    const_cast<ExecutableReference&>(executable_reference_)
        .ReturnInstructionBuffers(std::move(instruction_buffers_));
  }

  return Status();  // OK
}

Status SingleTpuRequest::Validate() {
  TRACE_SCOPE("Request::Validate");
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateState(kUninitialized));

  // Validate instruction bit stream.
  if (VectorLength(executable().instruction_bitstreams()) == 0) {
    return InvalidArgumentError(
        "Executable does not contain instruction bitstream.");
  }
  for (const auto& chunk : *executable().instruction_bitstreams()) {
    if (VectorLength(chunk->bitstream()) == 0) {
      return InvalidArgumentError(
          "Executable contains empty instruction bitstream chunk.");
    }
  }

  // Number of input / outputs should match with executable.
  if (host_inputs_.size() != VectorLength(executable().input_layers())) {
    return InvalidArgumentError(
        "Added inputs does not match the number of required inputs for "
        "executable.");
  }

  if (host_outputs_.size() != VectorLength(executable().output_layers())) {
    return InvalidArgumentError(
        "Added outputs does not match the number of required outputs for "
        "executable.");
  }

  // Number of input / output buffers must match configured batch size.
  for (const auto& name_and_input : host_inputs_) {
    if (name_and_input.second.size() != executable().batch_size()) {
      return InvalidArgumentError(
          StringPrintf("Number of input buffers for \"%s\" does not match "
                       "configured batch size. expected=%d, actual=%zu.",
                       name_and_input.first.c_str(), executable().batch_size(),
                       name_and_input.second.size()));
    }
  }

  for (const auto& name_and_output : host_outputs_) {
    if (name_and_output.second.size() != executable().batch_size()) {
      return InvalidArgumentError(
          StringPrintf("Number of output buffers for \"%s\" does not match "
                       "configured batch size. expected=%d, actual=%zu.",
                       name_and_output.first.c_str(), executable().batch_size(),
                       name_and_output.second.size()));
    }
  }

  return Status();  // OK
}

Status SingleTpuRequest::Prepare() {
  TRACE_SCOPE("Request::Prepare");
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateState(kUninitialized));

  // Reuses old instruction buffers if available.
  // If not this will create new instruction buffers.
  if (!instruction_buffers_) {
    instruction_buffers_ =
        const_cast<ExecutableReference&>(executable_reference_)
            .GetInstructionBuffers(allocator_);
  }

  RETURN_IF_ERROR(MapDataBuffers());
  VLOG(10) << "MapDataBuffers() done.";

  // Update the instruction stream to link the input, output and parameter
  // addresses.
  instruction_buffers_->LinkInstructionBuffers(
      parameter_device_buffer_, device_buffer_mapper_.get(),
      *executable().instruction_bitstreams());

  // Mapping of instruction buffers must happen after instructions have been
  // been patched with linked addresses. Any further modifications to
  // instructions may not be visible to device due to cache coherency issues.
  auto status = MapInstructionBuffers();
  if (!status.ok()) {
    status.Update(device_buffer_mapper_->UnmapAll());
    return status;
  }
  VLOG(10) << "MapInstructionBuffers() done.";

  return SetState(kCreated);
}

Status SingleTpuRequest::NotifyRequestSubmitted() {
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateState(kCreated));

  VLOG(3) << StringPrintf("[%d] NotifyRequestSubmitted()", id_);
  return SetState(kSubmitted);
}

Status SingleTpuRequest::NotifyRequestActive() {
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateState(kSubmitted));

  VLOG(3) << StringPrintf("[%d] NotifyRequestActive()", id_);
  return SetState(kActive);
}

Status SingleTpuRequest::NotifyCompletion(Status status) {
  TRACE_SCOPE("Request::NotifyCompletion");
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateState(kActive));

  // First notify the parent request. This will affect timing measurements so it
  // needs to be done first.
  parent_request_->NotifyCompletion(type());
  VLOG(3) << StringPrintf("[%d] NotifyCompletion()", id_);

  // Cleanup first before notify, because we need to unmap buffers first to
  // guarantee that output buffers are coherent.
  status.Update(Cleanup());

  RETURN_IF_ERROR(PostProcessOutputBuffers());

  if (done_) {
    done_(id_, status);

    // The |done_| callback may be a lambda that directly or indirectly holds a
    // shared_ptr to this request. If that happens, we will have a circular
    // reference through a shared_ptr, which will cause a memory leak. Prevent
    // the leak by explicitly destructing the lambda here.
    done_ = nullptr;
  }

  return SetState(kDone);
}

StatusOr<std::list<DmaInfo>> SingleTpuRequest::GetDmaInfos() const {
  StdMutexLock lock(&mutex_);
  if (state_ != kCreated && state_ != kSubmitted) {
    return FailedPreconditionError(
        StringPrintf("Unexpected call to GetDmaInfos in state_ = %d.", state_));
  }

  return extractor_.ExtractDmaInfos(executable_reference_,
                                    *device_buffer_mapper_);
}

Status SingleTpuRequest::Cancel() {
  StdMutexLock lock(&mutex_);

  VLOG(3) << StringPrintf("[%d] Cancel()", id_);

  if (state_ == kUninitialized || state_ == State::kCreated) {
    return FailedPreconditionError(
        StringPrintf("Cannot cancel in state_=%d.", state_));
  }

  // If State::kSubmitted, or kActive OK to cancel.
  if (state_ == State::kSubmitted || state_ == State::kActive) {
    // Run completed callback.
    // TODO: Share common code with NotifyCompletion.
    if (done_) {
      done_(id_, CancelledError("Request cancelled."));
      done_ = nullptr;  // See above for why this is needed.
    }

    RETURN_IF_ERROR(Cleanup());
    return SetState(kDone);
  }

  // If State::kDone, do nothing because request is already complete.
  return Status();  // OK
}

Status SingleTpuRequest::ValidateState(State expected_state) const {
  if (state_ != expected_state) {
    return FailedPreconditionError(StringPrintf(
        "Bad request state. expected=%d, actual=%d.", expected_state, state_));
  }
  return Status();  // OK
}

Status SingleTpuRequest::SetState(State next_state) {
  VLOG(5) << StringPrintf("[%d] SetState old=%d, new=%d.", id_, state_,
                          next_state);
  switch (state_) {
    case kUninitialized:
      if (next_state == kCreated) {
        state_ = next_state;
        return Status();  // OK
      }
      break;

    case kCreated:
      if (next_state == kSubmitted) {
        state_ = next_state;
        return Status();  // OK
      }
      break;

    case kSubmitted:
      if (next_state == kActive || next_state == kDone) {
        state_ = next_state;
        return Status();  // OK
      }
      break;

    case kActive:
      if (next_state == kDone) {
        state_ = next_state;
        return Status();  // OK
      }
      break;

    case kDone:
      break;
  }

  // Illegal state transition.
  return FailedPreconditionError(StringPrintf(
      "Invalid state transition. current=%d, next=%d.", state_, next_state));
}

const Buffer& SingleTpuRequest::InputBuffer(const std::string& name,
                                            int batch) const {
  StdMutexLock lock(&mutex_);
  return host_inputs_.at(name)[batch];
}

Buffer SingleTpuRequest::OutputBuffer(const std::string& name,
                                      int batch) const {
  StdMutexLock lock(&mutex_);
  return host_outputs_.at(name)[batch];
}

bool SingleTpuRequest::IsBufferAligned(const Buffer& buffer) {
  return reinterpret_cast<intptr_t>(buffer.ptr()) % alignment_bytes_ == 0;
}

Status SingleTpuRequest::PostProcessOutputBuffers() {
  TRACE_SCOPE("SingleTpuRequest::PostProcessOutputBuffers");
  for (const auto& name_and_output : host_outputs_) {
    const auto& layer_name = name_and_output.first;
    auto user_output_name_and_buffers = user_outputs_.find(layer_name);
    if (user_output_name_and_buffers == user_outputs_.end()) {
      return InternalError(
          StringPrintf("Unable to find output layer %s in user outputs map.",
                       layer_name.c_str()));
    }

    const auto& host_output_buffers = name_and_output.second;
    auto& user_output_buffers = user_output_name_and_buffers->second;
    if (host_output_buffers.size() < user_output_buffers.size()) {
      return InternalError(
          StringPrintf("Found %zu user output buffers which is greater than "
                       "%zu host output buffers for layer %s.",
                       user_output_buffers.size(), host_output_buffers.size(),
                       layer_name.c_str()));
    }

    ASSIGN_OR_RETURN(const auto* layer,
                     executable_reference_.OutputLayer(layer_name));

    for (int i = 0; i < user_output_buffers.size(); ++i) {
      Buffer user_buffer = user_output_buffers[i];
      if (user_buffer.IsDramType() && !user_buffer.IsManagedType()) {
        // No support for post-processing of user output buffer allocated
        // on device DRAM.

        // TODO -- When the proper test is implemented, use it to
        // validate that this output buffer does not in fact need
        // post-processing.
        continue;
      }
      // Otherwise, always do post-processing even if tests indicate that
      // it is not needed: the post-processing will also synchronize data
      // between the runtime-managed (host) and user-provided output buffer.

      Buffer host_buffer = host_output_buffers[i];
      if (host_buffer.IsDramType()) {
        TRACE_SCOPE(
            "SingleTpuRequest::PostProcessOutputBuffers::DramToHostOutput");
        ASSIGN_OR_RETURN(auto dram_buffer, host_buffer.GetDramBuffer());
        host_buffer = allocator_->MakeBuffer(layer->PaddedSizeBytes());
        RETURN_IF_ERROR(dram_buffer->WriteTo(host_buffer.ptr()));
      }

      {
        TRACE_SCOPE("SingleTpuRequest::PostProcessOutputBuffers::Relayout");
        RETURN_IF_ERROR(layer->Relayout(user_buffer.ptr(), host_buffer.ptr()));
      }

      if (layer->SignedDataType()) {
        TRACE_SCOPE(
            "SingleTpuRequest::PostProcessOutputBuffers::"
            "TransformSignedDataType");
        RETURN_IF_ERROR(layer->TransformSignedDataType(user_buffer));
      }
    }
  }

  return OkStatus();
}

Buffer SingleTpuRequest::ScatterInput(const Buffer& input,
                                      const api::LayerInformation* layer) {
  // For iterative models, we need to add padding after each iteration.
  auto aligned_input = allocator_->MakeBuffer(layer->PaddedSizeBytes());
  auto padded_single_execution_size =
      layer->PaddedSizeBytes() / layer->execution_count_per_inference();
  auto actual_single_execution_size =
      layer->ActualSizeBytes() / layer->execution_count_per_inference();
  for (int i = 0; i < layer->execution_count_per_inference(); i++) {
    memcpy(aligned_input.ptr() + padded_single_execution_size * i,
           input.ptr() + actual_single_execution_size * i,
           actual_single_execution_size);
  }

  return aligned_input;
}

Buffer SingleTpuRequest::TryCreateDramBuffer(size_t size_bytes) {
  auto buffer_or_error = dram_allocator_->AllocateBuffer(size_bytes);
  if (buffer_or_error.ok()) {
    return Buffer(std::move(buffer_or_error).ValueOrDie());
  }

  LOG(WARNING) << StringPrintf(
                      "Failed to allocate TPU DRAM buffer of size %zu: ",
                      size_bytes)
               << buffer_or_error.status().message();
  return allocator_->MakeBuffer(size_bytes);
}

Buffer SingleTpuRequest::CreateActivationBuffer(
    const api::LayerInformation* layer, int batches) {
  // TODO: We can't use DRAM buffers when also using batching.
  // Note that we could have allocated separate per-batch on-chip DRAM buffers
  // instead of using host DRAM, but we don't have a clear use case to evaluate
  // the power/perf tradeoff.
  if (layer->CacheOnDram() && batches == 1) {
    return TryCreateDramBuffer(layer->PaddedSizeBytes());
  } else {
    return allocator_->MakeBuffer(layer->PaddedSizeBytes() * batches);
  }
}

Buffer SingleTpuRequest::GetOrCreateBatchOutput(
    const api::LayerInformation* layer, const std::string& name) {
  const auto existing = batch_outputs_.find(name);
  if (existing == batch_outputs_.end()) {
    auto batch_output =
        CreateActivationBuffer(layer, executable_reference_.BatchSize());
    batch_outputs_[name] = batch_output;
    return batch_output;
  } else {
    return existing->second;
  }
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
