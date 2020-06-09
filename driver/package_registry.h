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

#ifndef DARWINN_DRIVER_PACKAGE_REGISTRY_H_
#define DARWINN_DRIVER_PACKAGE_REGISTRY_H_

#include <memory>
#include <mutex>  // NOLINT
#include <string>
#include <unordered_map>
#include <vector>

#include "api/buffer.h"
#include "api/chip.h"
#include "api/driver_options_generated.h"
#include "api/execution_context_interface.h"
#include "api/layer_information.h"
#include "api/package_reference.h"
#include "driver/aligned_allocator.h"
#include "driver/device_buffer_mapper.h"
#include "driver/instruction_buffers.h"
#include "driver/memory/dram_allocator.h"
#include "driver/package_verifier.h"
#include "executable/executable_generated.h"
#include "port/status_macros.h"
#include "port/statusor.h"
#include "port/std_mutex_lock.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

class PackageReference;

// Holds the input and output layer info from an executable.
class ExecutableLayersInfo {
 public:
  ExecutableLayersInfo(const Executable* executable);
  ~ExecutableLayersInfo() = default;

  // This class is neither copyable nor movable.
  ExecutableLayersInfo(const ExecutableLayersInfo&) = delete;
  ExecutableLayersInfo& operator=(const ExecutableLayersInfo&) = delete;

  // Returns the index of input layer with given name.
  util::StatusOr<int> InputIndex(const std::string& name) const;

  // Returns the index of output layer with given name.
  util::StatusOr<int> OutputIndex(const std::string& name) const;

  // Returns number of input layers.
  int NumInputLayers() const { return inputs_.size(); }

  // Returns number of output layers.
  int NumOutputLayers() const { return outputs_.size(); }

  // Returns list of input layer names.
  const std::vector<std::string>& InputLayerNames() const {
    return input_layer_names_;
  }

  // Returns list of output layer names.
  const std::vector<std::string>& OutputLayerNames() const {
    return output_layer_names_;
  }

  // Returns information on given input layer. Returns nullptr if index is out
  // of bounds.
  const api::InputLayerInformation* InputLayer(int index) const;

  // Returns information on given output layer. Returns nullptr if index is out
  // of bounds.
  const api::OutputLayerInformation* OutputLayer(int index) const;

  // Returns information on given input layer.
  util::StatusOr<const api::InputLayerInformation*> InputLayer(
      const std::string& layer_name) const;

  // Returns information on given output layer.
  util::StatusOr<const api::OutputLayerInformation*> OutputLayer(
      const std::string& layer_name) const;

  // Returns the expected byte size of activations for given input layer index.
  int InputLayerSizeBytes(int index) const {
    CHECK(InputLayer(index) != nullptr);
    return InputLayer(index)->ActualSizeBytes();
  }

  // Returns the expected byte size of activations for given input layer index.
  // This is post-padding, if any.
  // TODO Remove this method.
  int InputLayerPaddedSizeBytes(int index) const {
    CHECK(InputLayer(index) != nullptr);
    return InputLayer(index)->PaddedSizeBytes();
  }

  // Returns the expected byte size of activations for given output layer index.
  int OutputLayerSizeBytes(int index) const {
    CHECK(OutputLayer(index) != nullptr);
    return OutputLayer(index)->ActualSizeBytes();
  }

  // Returns the expected size (in value count) of activations for given input
  // layer index. This is pre-padding, if any.
  int InputLayerSize(int index) const {
    auto layer = InputLayer(index);
    CHECK(layer != nullptr);
    return layer->y_dim() * layer->x_dim() * layer->z_dim() *
           layer->execution_count_per_inference();
  }

  // Returns the expected size (in value count) of activations for given input
  // layer index. This is pre-padding, if any.
  int OutputLayerSize(int index) const {
    auto layer = OutputLayer(index);
    CHECK(layer != nullptr);
    return layer->y_dim() * layer->x_dim() * layer->z_dim() *
           layer->execution_count_per_inference();
  }

  // Returns the expected size of activations for given input layer.
  // Prefer index based APIs for performance.
  util::StatusOr<int> InputLayerSizeBytes(const std::string& name) const;

  // Returns the expected size of activations for given input layer including
  // padding bytes (if any).
  // Prefer index based APIs for performance.
  // TODO Remove this method.
  util::StatusOr<int> InputLayerPaddedSizeBytes(const std::string& name) const;

  // Returns the expected size of activations for given output layer.
  // Prefer index based APIs for performance.
  util::StatusOr<int> OutputLayerSizeBytes(const std::string& name) const;

  // Returns name for given input layer index.
  std::string InputLayerName(int index) const {
    CHECK(InputLayer(index) != nullptr);
    return InputLayer(index)->name();
  }

  // Returns name for given output layer index.
  std::string OutputLayerName(int index) const {
    CHECK(OutputLayer(index) != nullptr);
    return OutputLayer(index)->name();
  }

  // Returns if on-chip DRAM is needed in either input or output layers.
  bool NeedsDramInLayers() const { return needs_dram_in_layers_; }

 private:
  // Vector with list of input layer names.
  std::vector<std::string> input_layer_names_;

  // Vector with list of output layer names.
  std::vector<std::string> output_layer_names_;

  // Vector with detailed input layer information.
  std::vector<api::InputLayerInformation> inputs_;

  // Vector with detailed outpu layer information.
  std::vector<api::OutputLayerInformation> outputs_;

  // Maps input layer names to indices.
  std::unordered_map<std::string, int> input_map_;

  // Maps output layer names to indices.
  std::unordered_map<std::string, int> output_map_;

  // Specifies if this executable needs on-chip DRAM for input or output layers.
  bool needs_dram_in_layers_ = false;
};

// Reference to a single executable.
class ExecutableReference {
 public:
  // This class is neither copyable nor movable.
  ExecutableReference(const ExecutableReference&) = delete;
  ExecutableReference& operator=(const ExecutableReference&) = delete;

  // Returns the index of input layer with given name.
  util::StatusOr<int> InputIndex(const std::string& name) const {
    return executable_layers_info_->InputIndex(name);
  }

  // Returns the index of output layer with given name.
  util::StatusOr<int> OutputIndex(const std::string& name) const {
    return executable_layers_info_->OutputIndex(name);
  }

  // Returns number of input layers.
  int NumInputLayers() const {
    return executable_layers_info_->NumInputLayers();
  }

  // Returns number of output layers.
  int NumOutputLayers() const {
    return executable_layers_info_->NumOutputLayers();
  }

  // Returns list of input layer names.
  const std::vector<std::string>& InputLayerNames() const {
    return executable_layers_info_->InputLayerNames();
  }

  // Returns list of output layer names.
  const std::vector<std::string>& OutputLayerNames() const {
    return executable_layers_info_->OutputLayerNames();
  }

  // Returns information on given input layer. Returns nullptr if index is out
  // of bounds.
  const api::InputLayerInformation* InputLayer(int index) const {
    return executable_layers_info_->InputLayer(index);
  }

  // Returns information on given output layer. Returns nullptr if index is out
  // of bounds.
  const api::OutputLayerInformation* OutputLayer(int index) const {
    return executable_layers_info_->OutputLayer(index);
  }

  // Returns information on given input layer.
  util::StatusOr<const api::InputLayerInformation*> InputLayer(
      const std::string& layer_name) const {
    return executable_layers_info_->InputLayer(layer_name);
  }

  // Returns information on given output layer.
  util::StatusOr<const api::OutputLayerInformation*> OutputLayer(
      const std::string& layer_name) const {
    return executable_layers_info_->OutputLayer(layer_name);
  }

  // Returns the expected byte size of activations for given input layer index.
  int InputLayerSizeBytes(int index) const {
    return executable_layers_info_->InputLayerSizeBytes(index);
  }

  // Returns the expected byte size of activations for given input layer index.
  // This is post-padding, if any.
  // TODO Remove this method.
  int InputLayerPaddedSizeBytes(int index) const {
    return executable_layers_info_->InputLayerPaddedSizeBytes(index);
  }

  // Returns the expected byte size of activations for given output layer index.
  int OutputLayerSizeBytes(int index) const {
    return executable_layers_info_->OutputLayerSizeBytes(index);
  }

  // Returns the expected size (in value count) of activations for given input
  // layer index. This is pre-padding, if any.
  int InputLayerSize(int index) const {
    return executable_layers_info_->InputLayerSize(index);
  }

  // Returns the expected size (in value count) of activations for given input
  // layer index. This is pre-padding, if any.
  int OutputLayerSize(int index) const {
    return executable_layers_info_->OutputLayerSize(index);
  }

  // Returns the expected size of activations for given input layer.
  // Prefer index based APIs for performance.
  util::StatusOr<int> InputLayerSizeBytes(const std::string& name) const {
    return executable_layers_info_->InputLayerSizeBytes(name);
  }

  // Returns the expected size of activations for given input layer including
  // padding bytes (if any).
  // Prefer index based APIs for performance.
  // TODO Remove this method.
  util::StatusOr<int> InputLayerPaddedSizeBytes(const std::string& name) const {
    return executable_layers_info_->InputLayerPaddedSizeBytes(name);
  }

  // Returns the expected size of activations for given output layer.
  // Prefer index based APIs for performance.
  util::StatusOr<int> OutputLayerSizeBytes(const std::string& name) const {
    return executable_layers_info_->OutputLayerSizeBytes(name);
  }

  // Returns name for given input layer index.
  std::string InputLayerName(int index) const {
    return executable_layers_info_->InputLayerName(index);
  }

  // Returns name for given output layer index.
  std::string OutputLayerName(int index) const {
    return executable_layers_info_->OutputLayerName(index);
  }

  // Returns batch size.
  int BatchSize() const { return executable_->batch_size(); }

  // Executable
  const darwinn::Executable& executable() const { return *executable_; }

  // Memory aligned copy of the parameters.
  const Buffer& parameters() const { return parameters_; }

  // Sets mapped parameters.
  // Move-only. The given mapped_parameter will be unmapped during destruction
  // time, so we cannot allow copy-construction, to avoid doubly unmapping a
  // Device Buffer.
  util::Status SetMappedParameters(MappedDeviceBuffer&& mapped_parameters);

  // Unmaps the parameters buffer from the device.
  util::Status UnmapParameters();

  // Returns true if the parameters buffer is already mapped to the device.
  bool ParametersMapped() const { return parameters_mapped_; }

  // Returns the device mapped buffer for the parameters in this executable.
  const DeviceBuffer& GetParameterDeviceBuffer() const {
    return mapped_parameters_.device_buffer();
  }

  // Scratch buffer, if the executable requires scratch space. If not, then the
  // buffer will be invalid.
  Buffer scratch() const { return scratch_; }

  // Validates that the given input buffer is compatible with the executable.
  util::Status ValidateInput(const std::string& input_name,
                             const Buffer& input) const;

  // Validates that the given output buffer is compatible with the executable.
  util::Status ValidateOutput(const std::string& output_name,
                              const Buffer& output) const;

  // Returns the parameter-caching token which is unique across models that are
  // compiled together and can cache their parameters on TPU SRAM at the same
  // time. If 0, it means this executable's parameters cannot safely co-exist
  // with those of others. Please note that tokens are not limited to parameter
  // cached models. We could have a stand-alone compiled model that still has
  // a token to ensure us it will not overwite cached parameters of other
  // models.
  uint64 ParameterCachingToken() const {
    return executable().parameter_caching_token();
  }

  // Returns the estimated runtime in cycles for this executable.
  int64 EstimatedCycles() const {
    return executable().estimated_cycles_64bit();
  }

  // Reuses or creates instruction buffers.
  std::unique_ptr<InstructionBuffers> GetInstructionBuffers(
      Allocator* allocator);

  // Returns instruction buffers back to executable reference.
  // TODO: Add pool size limit. This currently doesn't have size
  // limit, and if there are many requests happened at the same time, we might
  // increase the total memory footprint. Notice that this won't increase
  // the memory peak size but will hold them longer. If this becomes an issue
  // we should investigate what's the correct size limit.
  void ReturnInstructionBuffers(
      std::unique_ptr<InstructionBuffers> instruction_buffers);

  // Makes sure parameters are present in host or TPU DRAM to be used in an
  // inference. This method is not thread-safe.
  util::Status PrepareParameters();

  // Resets any assumptions about parameters being loaded on TPU DRAM. This
  // method is not thread-safe.
  void ResetParametersLoaded();

  // Specifies if this executable needs on-chip DRAM to execute.
  bool NeedsDram() const { return needs_dram_; }

  // Returns the amount of narrow memory (in bytes) used by each tile in this
  // executable.
  int64 UsedNarrowMemoryBytesPerTile() const {
    return executable_->used_narrow_memory_bytes_per_tile();
  }

  const PackageReference& GetPackageReference() const {
    return *package_reference_;
  }

 private:
  friend class PackageReference;

  // Allow constructions through the friend classes only.
  ExecutableReference(const Executable* executable, Allocator* allocator,
                      DramAllocator* dram_allocator, PackageReference* pkg_ref);

  // Memory aligned copy of parameters.
  Buffer parameters_;

  // Mapped parameters.
  MappedDeviceBuffer mapped_parameters_;

  // Scratch buffer, if the executable requires scratch space. If not, then the
  // buffer will be invalid.
  Buffer scratch_;

  // Holds the backing executable.
  const Executable* executable_;

  // Holds the information on input and output layers.
  std::unique_ptr<ExecutableLayersInfo> executable_layers_info_;

  mutable std::mutex instruction_buffers_vector_mutex_;
  std::vector<std::unique_ptr<InstructionBuffers>> instruction_buffers_vector_
      GUARDED_BY(instruction_buffers_vector_mutex_);

  // Specifies if parameters of this executable are mapped to the device.
  bool parameters_mapped_ = false;

  // Specifies if parameters are already loaded to on-chip DRAM.
  bool parameters_loaded_ = false;

  // Specifies if this executable needs on-chip DRAM to execute.
  // The DRAM might be needed in input and output layers, parameters, or scratch
  // memory.
  bool needs_dram_ = false;

  // Pointer to package reference that contains this object. This object does
  // not own package_reference_.
  PackageReference *package_reference_;
};

// Contains an executable package.
class PackageReference : public api::PackageReference {
 public:
  // This class is neither copyable nor movable.
  PackageReference(const PackageReference&) = delete;
  PackageReference& operator=(const PackageReference&) = delete;

  // Verifies the digital signature in the package.
  util::Status VerifySignature() const override {
    return verifier_->VerifySignature(package_buffer_.ptr());
  }

  // The following methods just call their counterpart in
  // MainExecutableReference().
  util::StatusOr<int> InputIndex(const std::string& name) const override {
    return MainExecutableReference()->InputIndex(name);
  }

  util::StatusOr<int> OutputIndex(const std::string& name) const override {
    return MainExecutableReference()->OutputIndex(name);
  }

  int NumInputLayers() const override {
    return MainExecutableReference()->NumInputLayers();
  }

  int NumOutputLayers() const override {
    return MainExecutableReference()->NumOutputLayers();
  }

  const std::vector<std::string>& InputLayerNames() const override {
    return MainExecutableReference()->InputLayerNames();
  }

  const std::vector<std::string>& OutputLayerNames() const override {
    return MainExecutableReference()->OutputLayerNames();
  }

  const api::InputLayerInformation* InputLayer(int index) const override {
    return MainExecutableReference()->InputLayer(index);
  }

  const api::OutputLayerInformation* OutputLayer(int index) const override {
    return MainExecutableReference()->OutputLayer(index);
  }

  util::StatusOr<const api::InputLayerInformation*> InputLayer(
      const std::string& layer_name) const override {
    return MainExecutableReference()->InputLayer(layer_name);
  }

  util::StatusOr<const api::OutputLayerInformation*> OutputLayer(
      const std::string& layer_name) const override {
    return MainExecutableReference()->OutputLayer(layer_name);
  }

  int InputLayerSizeBytes(int index) const override {
    return MainExecutableReference()->InputLayerSizeBytes(index);
  }

  // TODO Remove this method.
  int InputLayerPaddedSizeBytes(int index) const override {
    return MainExecutableReference()->InputLayerPaddedSizeBytes(index);
  }

  int OutputLayerSizeBytes(int index) const override {
    return MainExecutableReference()->OutputLayerSizeBytes(index);
  }

  int InputLayerSize(int index) const override {
    return MainExecutableReference()->InputLayerSize(index);
  }

  int OutputLayerSize(int index) const override {
    return MainExecutableReference()->OutputLayerSize(index);
  }

  util::StatusOr<int> InputLayerSizeBytes(
      const std::string& name) const override {
    return MainExecutableReference()->InputLayerSizeBytes(name);
  }

  // TODO Remove this method.
  util::StatusOr<int> InputLayerPaddedSizeBytes(
      const std::string& name) const override {
    return MainExecutableReference()->InputLayerPaddedSizeBytes(name);
  }

  util::StatusOr<int> OutputLayerSizeBytes(
      const std::string& name) const override {
    return MainExecutableReference()->OutputLayerSizeBytes(name);
  }

  std::string InputLayerName(int index) const override {
    return MainExecutableReference()->InputLayerName(index);
  }

  std::string OutputLayerName(int index) const override {
    return MainExecutableReference()->OutputLayerName(index);
  }

  int BatchSize() const override {
    return MainExecutableReference()->BatchSize();
  }

  util::Status SetLatencyTolerance(int64 latency_tolerance_ms) override;

  // Returns a vector of all executable references in this package.
  std::vector<driver::ExecutableReference*> AllExecutableReferences() const;

  // Returns the main executable reference to refer to for read-only information
  // (e.g. number of layers).
  const driver::ExecutableReference* MainExecutableReference() const {
    return standalone_reference_ ? standalone_reference_.get()
                                 : inference_reference_.get();
  }

  // Returns true if this package is parameter-caching-enabled.
  bool ParameterCachingEnabled() const {
    return parameter_caching_reference_ != nullptr;
  }

  // Returns the inference executable reference in this package. You can make
  // sure such reference exists by calling ParameterCachingEnabled method.
  const driver::ExecutableReference* InferenceExecutableReference() const {
    return inference_reference_.get();
  }

  // Returns the parameter-caching executable reference in this package. You can
  // make sure such reference exists by calling ParameterCachingEnabled method.
  const driver::ExecutableReference* ParameterCachingExecutableReference()
      const {
    return parameter_caching_reference_.get();
  }

  // Returns true if parameters of this package are mapped to the device.
  util::StatusOr<bool> ParametersMapped() const;

  // Specifies if this package needs on-chip DRAM to execute.
  bool NeedsDram() const;

  // Sets the execution context interface. This class owns the execution
  // context.
  void SetExecutionContextInterface(
      std::unique_ptr<api::ExecutionContextInterface>
          execution_context_interface) override {
    execution_context_interface_ = std::move(execution_context_interface);
  }

  std::string ModelIdentifier() const override {
    return flatbuffers::GetString(package_->model_identifier());
  }

  // Returns the stored execution context interface. This class still owns the
  // object.
  api::ExecutionContextInterface* GetExecutionContextInterface() const {
    return execution_context_interface_.get();
  }

  // Returns the amount of time in milliseconds that this package can tolerate
  // for an inference to run (including parameter-caching). If batched, then for
  // all batch elements to complete.
  int64 LatencyToleranceMs() const { return latency_tolerance_ms_; }

 private:
  friend class PackageRegistry;

  // Allow constructions through the ExecutableRegistry class only.
  //
  // The current implementation allows either a stand-alone executable or
  // parameter-caching + inference.

  // Constructor for stand-alone executable.
  PackageReference(const Buffer& package_buffer,
                   const Executable* standalone_executable,
                   Allocator* allocator, DramAllocator* dram_allocator,
                   PackageVerifier* verifier);

  // Constructor for a parameter cached package.
  PackageReference(const Buffer& package_buffer,
                   const Executable* parameter_caching_executable,
                   const Executable* inference_executable, Allocator* allocator,
                   DramAllocator* dram_allocator, PackageVerifier* verifier);

  // Unmaps parameters of all executables in this package.
  util::Status UnmapParameters();

  // Buffer backing the package buffer.
  Buffer package_buffer_;

  // The flatbuffer representation of the package we are wrapping.
  const Package* package_;

  // A ExecutableVerifier for checking digital signatures on executable
  // packages.
  const PackageVerifier* const verifier_;

  // The ExecutableReference for the parameter-caching executable.
  std::unique_ptr<driver::ExecutableReference> parameter_caching_reference_;

  // The Executable reference for the inference executable.
  std::unique_ptr<driver::ExecutableReference> inference_reference_;

  // The Executable reference for the stand-alone executable.
  std::unique_ptr<driver::ExecutableReference> standalone_reference_;

  // The execution context for the package reference.
  std::unique_ptr<api::ExecutionContextInterface> execution_context_interface_;

  // Maximum number of milliseconds this package can tolerate for an inference
  // request to run.
  int64 latency_tolerance_ms_ = -1;
};

// A registry for executable files. Most methods do not have built-in thread-
// safety and rely on base driver class to ensure that. Some methods require
// thread-safety with respect to their call-sites in base driver and that is
// implemented in this class.
class PackageRegistry {
 public:
  PackageRegistry(api::Chip chip,
                  std::unique_ptr<PackageVerifier> executable_verifier,
                  DramAllocator* dram_allocator);

  // Constructs an ExecutableRegistry that will check to make sure all
  // registered executables are for the correct chip. However, it does not
  // support DRAM nor signature verification. Please prefer the first
  // constructor.
  explicit PackageRegistry(api::Chip chip);

  // Constructs an ExecutableRegistry that does not check if the executables
  // being registered are for the correct device. Please prefer the first
  // constructor.
  PackageRegistry();

  ~PackageRegistry() = default;

  // This class is neither copyable nor movable.
  PackageRegistry(const PackageRegistry&) = delete;
  PackageRegistry& operator=(const PackageRegistry&) = delete;

  // Returns the main executable layer info from the given executable binary.
  static util::StatusOr<std::unique_ptr<ExecutableLayersInfo>>
  GetMainExecutableLayersInfoFromBinary(const char* executable_content,
                                        size_t length);

  // Registers a serialized executable binary. Once the executable is
  // registered, driver has its own copy of it so there would be no need to keep
  // the executable_content in memory.
  util::StatusOr<const api::PackageReference*> RegisterSerialized(
      const std::string& executable_content);
  util::StatusOr<const api::PackageReference*> RegisterSerialized(
      const char* executable_content, size_t length);

  // Same as above, but the executable is read from the given file.
  util::StatusOr<const api::PackageReference*> RegisterFile(
      const std::string& executable_filename);

  // Unregisters an executable. Invokes the callback to unmap the parameter.
  util::Status Unregister(const api::PackageReference* package_reference);

  // Unregisteres all registered executables.
  util::Status UnregisterAll() LOCKS_EXCLUDED(registrations_mutex_);

  // Unmaps all parameters in all registered packages.
  util::Status UnmapAllParameters() LOCKS_EXCLUDED(registrations_mutex_);

  // Returns the number of registered executables.
  int GetRegistrySize() const LOCKS_EXCLUDED(registrations_mutex_) {
    StdMutexLock registration_lock(&registrations_mutex_);
    return registrations_.size();
  }

  // Returns all the package references registered here.
  std::vector<api::PackageReference*> GetAllRegistrations() const
      LOCKS_EXCLUDED(registrations_mutex_);

  // Resets any assumptions about parameters of any current registrations being
  // loaded on TPU DRAM.
  void ResetParametersLoaded() LOCKS_EXCLUDED(registrations_mutex_);

 private:
  // Returns the main executable from the executable map.
  // Returns error if failed to find main executable or had unexpected
  // executable combinations.
  static util::StatusOr<const Executable*> GetMainExecutableFromExecutableMap(
      std::unordered_map<ExecutableType, const Executable*>);

  // Returns the parameter caching executable from the executable map.
  // Returns nullptr if no parameter caching executable could be found in the
  // map.
  // Returns error if had unexpected executable combinations.
  static util::StatusOr<const Executable*> GetPCExecutableFromExecutableMap(
      std::unordered_map<ExecutableType, const Executable*>);

  // Registers an executable package binary.
  util::StatusOr<const api::PackageReference*> RegisterPackage(
      const Buffer& package_buffer);

  // Takes in a multi-executable and returns a map of each executable type to
  // its executable pointer. It will return an error in any illegal combination.
  // Legitimate combinations are:
  //
  //   1. A single executable (no matter what type).
  //   2. 1 parameter-caching and 1 inference executable.
  //   3. 1 parameter-caching, 1 inference and 1 stand-alone executable.
  static util::StatusOr<std::unordered_map<ExecutableType, const Executable*>>
  ExtractExecutables(const MultiExecutable& multi_executable);

  // Takes in executable binary content and returns a map of each executable
  // type to its executable pointer.
  // The inputs are pointers to the executable binary, the length of the binary,
  // and the targeted chip to run this executables on.
  static util::StatusOr<std::unordered_map<ExecutableType, const Executable*>>
  GetExecutablesFromBinary(const char* executable_content, size_t length);

  // Fetches an Executable from its serialized version and performs some
  // verification checks (does not include signature verification).
  static util::StatusOr<const Executable*> FetchAndVerifyExecutable(
      const char* executable_serialized, size_t length);

  // Checks if the chip config specified in the executable binary matches the
  // one registered to this package registry.
  util::Status VerifyExecutableMatchesChip(const Executable*) const;

  const api::PackageReference* SetRegistrations(
      std::unique_ptr<api::PackageReference> api_package_ref)
      LOCKS_EXCLUDED(registrations_mutex_);

  // Allocator.
  AlignedAllocator allocator_;

  // A pointer to the entity responsible for allocating on-chip DRAM buffers.
  DramAllocator* dram_allocator_;

  // A mutex to synchronize access to registrations_.
  mutable std::mutex registrations_mutex_;

  // Tracks registrations.
  // Uses a map instead of a set, since looking up an std::set of unique_ptr is
  // tricky.
  std::unordered_map<const api::PackageReference*,
                     std::unique_ptr<api::PackageReference>>
      registrations_ GUARDED_BY(registrations_mutex_);

  // Executables will be checked to ensure they were compiled for this chip.
  // Can be kUnknown to disable checking.
  const api::Chip chip_;

  // A verifier for checking digital signatures on executable packages.
  std::unique_ptr<PackageVerifier> verifier_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms


namespace std {
  template<>
  struct hash<::platforms::darwinn::ExecutableType> {
    typedef ::platforms::darwinn::ExecutableType argument_type;
    typedef std::underlying_type<argument_type>::type underlying_type;
    typedef std::hash<underlying_type>::result_type result_type;
    result_type operator()(const argument_type& arg) const {
        std::hash<underlying_type> hasher;
        return hasher(static_cast<underlying_type>(arg));
    }
  };
}

#endif  // DARWINN_DRIVER_PACKAGE_REGISTRY_H_
