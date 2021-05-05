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

#include "driver/package_registry.h"

#include <fstream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "api/package_reference.h"
#include "api/runtime_version.h"
#include "driver/aligned_allocator.h"
#include "driver/package_verifier.h"
#include "executable/executable_generated.h"
#include "port/errors.h"
#include "port/ptr_util.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"
#include "port/tracing.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace {

// Alignment for buffers allocated by the registry.
constexpr uint64 kAlignment = 4096;

}  // namespace

PackageRegistry::PackageRegistry() : PackageRegistry(api::Chip::kUnknown) {}

PackageRegistry::PackageRegistry(api::Chip chip)
    : PackageRegistry(chip, gtl::MakeUnique<NoopPackageVerifier>(), nullptr) {}

PackageRegistry::PackageRegistry(
    api::Chip chip, std::unique_ptr<PackageVerifier> executable_verifier,
    DramAllocator* dram_allocator)
    : allocator_(kAlignment),
      dram_allocator_(dram_allocator),
      chip_(chip),
      verifier_(std::move(executable_verifier)) {}

StatusOr<std::unordered_map<ExecutableType, const Executable*>>
PackageRegistry::GetExecutablesFromBinary(const char* executable_content,
                                          size_t length) {
  // Check the file identifier of the package.
  std::string package_identifier(
      flatbuffers::GetBufferIdentifier(executable_content),
      flatbuffers::FlatBufferBuilder::kFileIdentifierLength);
  if (package_identifier != api::kHeadPackageIdentifier) {
    LOG(WARNING) << StringPrintf("Package file identifier %s not supported.",
                                 package_identifier.c_str());
  }

  // Verify and get the package from the memory mapped buffer.
  flatbuffers::Verifier package_verifier(
      reinterpret_cast<const uint8*>(executable_content), length);
  if (!package_verifier.VerifyBuffer<Package>()) {
    return InternalError("Package verification failed.");
  }
  auto* package = flatbuffers::GetRoot<Package>(executable_content);

  // The runtime version check shall always be the first after parsing, so it's
  // possible to introduce non-backward-compatible changes.
  const auto min_runtime_version = package->min_runtime_version();
  if (min_runtime_version < api::RuntimeVersion::kMinValidRuntimeVersion) {
    LOG(WARNING) << StringPrintf(
        "Minimum runtime version required by package (%d) is lower than "
        "expected (%d).",
        min_runtime_version, api::RuntimeVersion::kMinValidRuntimeVersion);
  } else if (min_runtime_version > api::RuntimeVersion::kCurrent) {
    return FailedPreconditionError(StringPrintf(
        "Package requires runtime version (%d), which is newer "
        "than this runtime version (%d).",
        package->min_runtime_version(), api::RuntimeVersion::kCurrent));
  }

  constexpr int kVirtualChipIdForMultiChipPackage = -1;
  if (package->virtual_chip_id() == kVirtualChipIdForMultiChipPackage) {
    return FailedPreconditionError("This is a multi-chip package.");
  }

  if (flatbuffers::VectorLength(package->serialized_multi_executable()) == 0) {
    return FailedPreconditionError("No executables to register.");
  }

  // Verify and get the MultiExecutable table from the package.
  flatbuffers::Verifier multi_executable_verifier(
      package->serialized_multi_executable()->data(),
      flatbuffers::VectorLength(package->serialized_multi_executable()));
  if (!multi_executable_verifier.VerifyBuffer<MultiExecutable>()) {
    return InternalError("MultiExecutable verification failed.");
  }
  auto* multi_executable = flatbuffers::GetRoot<MultiExecutable>(
      package->serialized_multi_executable()->data());

  // Extract the buffer pointer for the serialized executable from the
  // MultiExecutable.

  if (flatbuffers::VectorLength(multi_executable->serialized_executables()) ==
      0) {
    return NotFoundError("No executables provided.");
  }

  return ExtractExecutables(*multi_executable);
}

StatusOr<const Executable*> PackageRegistry::GetMainExecutableFromExecutableMap(
    std::unordered_map<ExecutableType, const Executable*> executables) {
  switch (executables.size()) {
    case 1:
      // TODO Here we are considering the sole executable in a
      // package as stand-alone no matter what the type specifies. This is for
      // being backward-compatible with the old-style parameter-caching. Once
      // that is deprecated, here we should look for the STAND_ALONE type.
      return executables.begin()->second;

    case 2:
      return executables[ExecutableType_EXECUTION_ONLY];

    // TODO Once this feature is implemented, we need to update the
    // constructor used here. Right now we still allow 3 executables in a
    // package to avoid future backward-incompatibility. The current behavior is
    // to always use the stand-alone one.
    case 3:
      return executables[ExecutableType_STAND_ALONE];

    default:
      return InternalError("Unexpected combination of executables.");
  }
}

StatusOr<const Executable*> PackageRegistry::GetPCExecutableFromExecutableMap(
    std::unordered_map<ExecutableType, const Executable*> executables) {
  switch (executables.size()) {
    case 1:
      return nullptr;
    case 2:
      return executables[ExecutableType_PARAMETER_CACHING];
    case 3:
      return nullptr;
    default:
      return InternalError("Unexpected combination of executables.");
  }
}

StatusOr<const api::PackageReference*> PackageRegistry::RegisterPackage(
    const Buffer& package_buffer) {
  ASSIGN_OR_RETURN(auto executables,
                   GetExecutablesFromBinary(
                       reinterpret_cast<const char*>(package_buffer.ptr()),
                       package_buffer.size_bytes()));

  for (const auto& it : executables) {
    RETURN_IF_ERROR(VerifyExecutableMatchesChip(it.second));
  }

  ASSIGN_OR_RETURN(const Executable* main_executable,
                   GetMainExecutableFromExecutableMap(executables));
  ASSIGN_OR_RETURN(const Executable* parameter_caching_executable,
                   GetPCExecutableFromExecutableMap(executables));

  PackageReference* package_reference;

  if (parameter_caching_executable != nullptr) {
    package_reference = new PackageReference(
        package_buffer, parameter_caching_executable, main_executable,
        &allocator_, dram_allocator_, verifier_.get());
  } else {
    package_reference =
        new PackageReference(package_buffer, main_executable, &allocator_,
                             dram_allocator_, verifier_.get());
  }

  return SetRegistrations(
      std::unique_ptr<api::PackageReference>(package_reference));
}

StatusOr<std::unique_ptr<ExecutableLayersInfo>>
PackageRegistry::GetMainExecutableLayersInfoFromBinary(
    const char* executable_content, size_t length) {
  ASSIGN_OR_RETURN(auto executables,
                   GetExecutablesFromBinary(executable_content, length));

  ASSIGN_OR_RETURN(const Executable* main_executable,
                   GetMainExecutableFromExecutableMap(executables));

  return gtl::MakeUnique<ExecutableLayersInfo>(main_executable);
}

StatusOr<std::unordered_map<ExecutableType, const Executable*>>
PackageRegistry::ExtractExecutables(const MultiExecutable& multi_executable) {
  std::unordered_map<ExecutableType, const Executable*> executables;

  // Fetch executables to a map of type -> executable.
  for (const auto* executable_serialized :
       *multi_executable.serialized_executables()) {
    ASSIGN_OR_RETURN(auto executable,
                     FetchAndVerifyExecutable(executable_serialized->c_str(),
                                              executable_serialized->size()));

    if (executables.find(executable->type()) != executables.end()) {
      return InvalidArgumentError(
          "Multiple executables of the same type were found in the package.");
    }
    executables[executable->type()] = executable;
  }

  // Sanity check for legal combinations.
  switch (executables.size()) {
    case 0:
      return InternalError("No executables provided.");

    case 1:
      break;

    case 2:
      if (executables.find(ExecutableType_PARAMETER_CACHING) ==
              executables.end() ||
          executables.find(ExecutableType_EXECUTION_ONLY) ==
              executables.end()) {
        return InvalidArgumentError(
            "Invalid combination of executables in the package.");
      }
      break;

    case 3:
      if (executables.find(ExecutableType_PARAMETER_CACHING) ==
              executables.end() ||
          executables.find(ExecutableType_EXECUTION_ONLY) ==
              executables.end() ||
          executables.find(ExecutableType_STAND_ALONE) == executables.end()) {
        return InvalidArgumentError(
            "Invalid combination of executables in the package.");
      }
      break;

    default:
      return InvalidArgumentError(
          "Found executable types that are not yet supported.");
  }

  return executables;
}

StatusOr<const Executable*> PackageRegistry::FetchAndVerifyExecutable(
    const char* executable_serialized, size_t length) {
  flatbuffers::Verifier verifier(
      reinterpret_cast<const uint8*>(executable_serialized), length);
  if (!verifier.VerifyBuffer<Executable>()) {
    return InvalidArgumentError("Executable verification failed.");
  }

  const auto* executable = flatbuffers::GetRoot<Executable>(
      reinterpret_cast<const uint8*>(executable_serialized));

  // All executables must have a batch size of at least one.
  if (executable->batch_size() < 1) {
    return InvalidArgumentError("Executable has invalid batch size.");
  }

  return executable;
}

Status PackageRegistry::VerifyExecutableMatchesChip(
    const Executable* executable) const {
  (void)chip_;
  return OkStatus();
}

StatusOr<const api::PackageReference*> PackageRegistry::RegisterSerialized(
    const std::string& executable_content) {
  return RegisterSerialized(executable_content.data(),
                            executable_content.size());
}

StatusOr<const api::PackageReference*> PackageRegistry::RegisterSerialized(
    const char* executable_content, size_t length) {
  Buffer package_buffer = allocator_.MakeBuffer(length);
  CHECK(package_buffer.ptr() != nullptr);
  memcpy(package_buffer.ptr(), executable_content, length);
  return RegisterPackage(package_buffer);
}

StatusOr<const api::PackageReference*> PackageRegistry::RegisterFile(
    const std::string& executable_filename) {
  std::ifstream ifs;
  ifs.open(executable_filename, std::ifstream::in);
  if (!ifs.is_open()) {
    return InvalidArgumentError(
        StringPrintf("Cannot open %s.", executable_filename.c_str()));
  }

  ifs.seekg(0, std::ios_base::end);
  size_t file_size(ifs.tellg());
  ifs.seekg(std::ios_base::beg);

  Buffer package_buffer = allocator_.MakeBuffer(file_size);
  CHECK(package_buffer.ptr() != nullptr);
  ifs.read(reinterpret_cast<char*>(package_buffer.ptr()), file_size);
  ifs.close();

  return RegisterPackage(package_buffer);
}

Status PackageRegistry::Unregister(
    const api::PackageReference* package_reference) {
  StdMutexLock registrations_lock(&registrations_mutex_);

  // Bail out early if package_reference isn't valid.
  if (package_reference == nullptr) {
    return InvalidArgumentError("Provided package reference in null.");
  }
  if (registrations_.count(package_reference) == 0) {
    return NotFoundError(
        "Attempting to unregister a nonexistent executable reference.");
  }

  PackageReference* driver_package_ref = const_cast<PackageReference*>(
      static_cast<const PackageReference*>(package_reference));

  ASSIGN_OR_RETURN(auto parameters_mapped,
                   driver_package_ref->ParametersMapped());
  if (parameters_mapped) {
    RETURN_IF_ERROR(driver_package_ref->UnmapParameters());
  }

  // TODO : Need to track outstanding requests and error when
  // there are pending/in-flight requests at un-registration time.
  if (registrations_.erase(driver_package_ref) == 0) {
    return NotFoundError(
        "Attempting to unregister a nonexistent executable reference.");
  }

  return Status();  // OK.
}

Status PackageRegistry::UnregisterAll() {
  RETURN_IF_ERROR(UnmapAllParameters());

  StdMutexLock registrations_lock(&registrations_mutex_);
  // TODO : Need to track outstanding requests and error when
  // there are pending/in-flight requests at un-registration time.
  registrations_.clear();

  return OkStatus();
}

Status PackageRegistry::UnmapAllParameters() {
  StdMutexLock registrations_lock(&registrations_mutex_);
  Status status;

  for (auto& it : registrations_) {
    if (it.first == nullptr) {
      return InternalError("Encountered nullptr key to package reference.");
    }
    PackageReference* package = const_cast<PackageReference*>(
        static_cast<const PackageReference*>(it.first));

    const auto parameters_mapped = package->ParametersMapped();
    if (!parameters_mapped.ok()) {
      status.Update(parameters_mapped.status());
      continue;
    }

    if (parameters_mapped.ValueOrDie()) {
      status.Update(package->UnmapParameters());
    }
  }

  return status;
}

std::vector<api::PackageReference*> PackageRegistry::GetAllRegistrations()
    const {
  StdMutexLock registrations_lock(&registrations_mutex_);

  std::vector<api::PackageReference*> package_refs;
  package_refs.reserve(registrations_.size());
  for (auto& registration : registrations_) {
    package_refs.push_back(registration.second.get());
  }
  return package_refs;
}

const api::PackageReference* PackageRegistry::SetRegistrations(
    std::unique_ptr<api::PackageReference> api_package_ref) {
  StdMutexLock registrations_lock(&registrations_mutex_);

  auto api_reference =
      registrations_.emplace(api_package_ref.get(), std::move(api_package_ref))
          .first->first;

  return api_reference;
}

void PackageRegistry::ResetParametersLoaded() {
  StdMutexLock registrations_lock(&registrations_mutex_);
  for (auto& registration : registrations_) {
    auto package_ref =
        static_cast<PackageReference*>(registration.second.get());
    for (auto exec_ref : package_ref->AllExecutableReferences()) {
      exec_ref->ResetParametersLoaded();
    }
  }
}

ExecutableLayersInfo::ExecutableLayersInfo(const Executable* executable) {
  // Set layer information.
  const int input_layer_count =
      flatbuffers::VectorLength(executable->input_layers());
  inputs_.reserve(input_layer_count);
  input_layer_names_.reserve(input_layer_count);

  for (int i = 0; i < input_layer_count; ++i) {
    const auto& layer_name = executable->input_layers()->Get(i)->name()->str();
    api::InputLayerInformation layer(executable->input_layers()->Get(i));
    if (layer.CacheOnDram()) {
      needs_dram_in_layers_ = true;
    }
    inputs_.emplace_back(layer);
    input_layer_names_.emplace_back(layer_name);
    input_map_[layer_name] = i;
  }

  const int output_layer_count =
      flatbuffers::VectorLength(executable->output_layers());
  outputs_.reserve(output_layer_count);
  output_layer_names_.reserve(output_layer_count);

  for (int i = 0; i < output_layer_count; ++i) {
    const auto& layer_name = executable->output_layers()->Get(i)->name()->str();
    api::OutputLayerInformation layer(executable->output_layers()->Get(i));
    if (layer.CacheOnDram()) {
      needs_dram_in_layers_ = true;
    }
    outputs_.emplace_back(layer);
    output_layer_names_.emplace_back(layer_name);
    output_map_[layer_name] = i;
  }
}

StatusOr<int> ExecutableLayersInfo::InputIndex(const std::string& name) const {
  auto iter = input_map_.find(name);
  if (iter != input_map_.end()) {
    return iter->second;
  }
  return NotFoundError(
      StringPrintf("Input layer '%s' not found.", name.c_str()));
}

StatusOr<int> ExecutableLayersInfo::OutputIndex(const std::string& name) const {
  auto iter = output_map_.find(name);
  if (iter != output_map_.end()) {
    return iter->second;
  }
  return NotFoundError(
      StringPrintf("Output layer '%s' not found.", name.c_str()));
}

const api::InputLayerInformation* ExecutableLayersInfo::InputLayer(
    int index) const {
  if (index < inputs_.size()) {
    return &inputs_[index];
  }
  return nullptr;
}

const api::OutputLayerInformation* ExecutableLayersInfo::OutputLayer(
    int index) const {
  if (index < outputs_.size()) {
    return &outputs_[index];
  }
  return nullptr;
}

// TODO If possible, refactor this to only have a name map and we
// will not need to do 2 lookups.
StatusOr<const api::InputLayerInformation*> ExecutableLayersInfo::InputLayer(
    const std::string& layer_name) const {
  ASSIGN_OR_RETURN(auto index, InputIndex(layer_name));
  const auto* input_info = InputLayer(index);
  if (input_info == nullptr) {
    return InternalError(
        StringPrintf("Input layer %s was not found in executable reference.",
                     layer_name.c_str()));
  }
  return input_info;
}

StatusOr<const api::OutputLayerInformation*> ExecutableLayersInfo::OutputLayer(
    const std::string& layer_name) const {
  ASSIGN_OR_RETURN(auto index, OutputIndex(layer_name));
  const auto* output_info = OutputLayer(index);
  if (output_info == nullptr) {
    return InternalError(
        StringPrintf("Output layer %s was not found in executable reference.",
                     layer_name.c_str()));
  }
  return output_info;
}

StatusOr<int> ExecutableLayersInfo::InputLayerSizeBytes(
    const std::string& name) const {
  ASSIGN_OR_RETURN(int index, InputIndex(name));
  return inputs_[index].ActualSizeBytes();
}

StatusOr<int> ExecutableLayersInfo::InputLayerPaddedSizeBytes(
    const std::string& name) const {
  ASSIGN_OR_RETURN(int index, InputIndex(name));
  return inputs_[index].PaddedSizeBytes();
}

StatusOr<int> ExecutableLayersInfo::OutputLayerSizeBytes(
    const std::string& name) const {
  ASSIGN_OR_RETURN(int index, OutputIndex(name));
  return outputs_[index].ActualSizeBytes();
}

ExecutableReference::ExecutableReference(const Executable* executable,
                                         Allocator* allocator,
                                         DramAllocator* dram_allocator,
                                         PackageReference* pkg_ref)
    : executable_(executable), package_reference_(pkg_ref) {
  // Create a buffer for parameters. This buffer is either in host or in the
  // on-chip DRAM. If on host, we already have a copy of the data in the package
  // flatbuffer. If on chip, we will copy the data the first time the buffer is
  // mapped.
  auto parameter_size_bytes =
      flatbuffers::VectorLength(executable->parameters());
  if (parameter_size_bytes > 0) {
    // TODO Remove the check on nullptr.
    if (executable->use_tpu_dram_for_parameters() &&
        dram_allocator != nullptr) {
      auto buffer_or_error =
          dram_allocator->AllocateBuffer(parameter_size_bytes);
      if (buffer_or_error.ok()) {
        parameters_ = Buffer(std::move(buffer_or_error.ValueOrDie()));
        needs_dram_ = true;
      } else {
        LOG(WARNING) << StringPrintf(
                            "Failed to allocate TPU DRAM buffer of size %zu "
                            "for parameters: ",
                            parameter_size_bytes)
                     << buffer_or_error.status().message();
        parameters_ = Buffer(
            reinterpret_cast<const uint8*>(executable->parameters()->data()),
            parameter_size_bytes);
      }
    } else {
      parameters_ = Buffer(
          reinterpret_cast<const uint8*>(executable->parameters()->data()),
          parameter_size_bytes);
    }
  }

  // Allocate scratch if necessary. It is preferred to have scratch in the
  // on-chip DRAM.
  //
  // TODO Check if the chip does have a DRAM.
  if (executable->scratch_size_bytes() > 0) {
    if (dram_allocator != nullptr) {
      auto buffer_or_error =
          dram_allocator->AllocateBuffer(executable->scratch_size_bytes());
      if (buffer_or_error.ok()) {
        scratch_ = Buffer(std::move(buffer_or_error.ValueOrDie()));
        needs_dram_ = true;
      } else {
        scratch_ = allocator->MakeBuffer(executable->scratch_size_bytes());
      }
    } else {
      scratch_ = allocator->MakeBuffer(executable->scratch_size_bytes());
    }
  }

  // Extracts the input and output layers info from the executable binary.
  executable_layers_info_ = gtl::MakeUnique<ExecutableLayersInfo>(executable);

  // The DRAM will be needed if any of the component needs to access it.
  if (executable_layers_info_->NeedsDramInLayers()) {
    needs_dram_ = true;
  }
}

Status ExecutableReference::ValidateInput(const std::string& input_name,
                                          const Buffer& input) const {
  ASSIGN_OR_RETURN(const auto* layer, InputLayer(input_name));

  // We can only accept buffers that are the same size as the input layer tensor
  // with or without padding.
  if (input.size_bytes() != layer->ActualSizeBytes() &&
      input.size_bytes() != layer->PaddedSizeBytes()) {
    return InvalidArgumentError(StringPrintf(
        "Unexpected input size for \"%s\". Expected %d or %d, got %zu",
        input_name.c_str(), layer->ActualSizeBytes(), layer->PaddedSizeBytes(),
        input.size_bytes()));
  }

  return OkStatus();
}

Status ExecutableReference::ValidateOutput(const std::string& output_name,
                                           const Buffer& output) const {
  ASSIGN_OR_RETURN(const int expected_size_bytes,
                   OutputLayerSizeBytes(output_name));
  if (output.size_bytes() != expected_size_bytes) {
    return InvalidArgumentError(StringPrintf(
        "Unexpected output size for \"%s\". expected=%d, actual=%zu.",
        output_name.c_str(), expected_size_bytes, output.size_bytes()));
  }
  return Status();  // OK
}

// Reuses the instruction buffers if available. Creates a new one if not.
std::unique_ptr<InstructionBuffers> ExecutableReference::GetInstructionBuffers(
    Allocator* const allocator) {
  TRACE_SCOPE("ExecutableReference::GetInstructionBuffers");
  StdMutexLock lock(&instruction_buffers_vector_mutex_);

  if (!instruction_buffers_vector_.empty()) {
    std::unique_ptr<InstructionBuffers> old_instruction_buffers =
        std::move(instruction_buffers_vector_.back());
    instruction_buffers_vector_.pop_back();
    VLOG(10) << "Reusing old instruction buffers.";

    return old_instruction_buffers;
  }

  auto instruction_buffers = gtl::MakeUnique<InstructionBuffers>(
      allocator, *executable().instruction_bitstreams());

  VLOG(10) << "Created new instruction buffers.";
  return instruction_buffers;
}

// Returns instruction buffers back to the executable references so that the
// next request could reuse it.
void ExecutableReference::ReturnInstructionBuffers(
    std::unique_ptr<InstructionBuffers> instruction_buffers) {
  StdMutexLock lock(&instruction_buffers_vector_mutex_);

  instruction_buffers_vector_.push_back(std::move(instruction_buffers));
  VLOG(10) << "Returned instruction buffers back to executable reference";
}

Status ExecutableReference::PrepareParameters() {
  // If parameters are not in on-chip DRAM or they have already been loaded
  // there, nothing else to do here.
  if (!parameters_.IsDramType() || parameters_loaded_) {
    return OkStatus();
  }

  ASSIGN_OR_RETURN(auto dram_buffer, parameters_.GetDramBuffer());
  // TODO Get rid of this const_cast.
  RETURN_IF_ERROR(dram_buffer->ReadFrom(const_cast<uint8*>(
      reinterpret_cast<const uint8*>(executable_->parameters()->data()))));

  parameters_loaded_ = true;
  VLOG(2) << "Parameters were loaded on DRAM.";

  return OkStatus();
}

void ExecutableReference::ResetParametersLoaded() {
  if (parameters_.IsDramType()) {
    parameters_loaded_ = false;
  }
}

Status ExecutableReference::SetMappedParameters(
    MappedDeviceBuffer&& mapped_parameters) {
  if (parameters_mapped_) {
    RETURN_IF_ERROR(mapped_parameters.Unmap());
    return FailedPreconditionError("Parameters are already mapped.");
  }

  mapped_parameters_ = std::move(mapped_parameters);
  parameters_mapped_ = true;

  return OkStatus();
}

Status ExecutableReference::UnmapParameters() {
  if (!parameters_mapped_) {
    return FailedPreconditionError("Parameters are not currently mapped.");
  }

  RETURN_IF_ERROR(mapped_parameters_.Unmap());
  parameters_mapped_ = false;

  return OkStatus();
}

PackageReference::PackageReference(const Buffer& package_buffer,
                                   const Executable* standalone_executable,
                                   Allocator* allocator,
                                   DramAllocator* dram_allocator,
                                   PackageVerifier* verifier)
    : package_buffer_(package_buffer),
      package_(flatbuffers::GetRoot<Package>(package_buffer.ptr())),
      verifier_(verifier),
      standalone_reference_(new driver::ExecutableReference(
          standalone_executable, allocator, dram_allocator, this)) {}

PackageReference::PackageReference(
    const Buffer& package_buffer,
    const Executable* parameter_caching_executable,
    const Executable* inference_executable, Allocator* allocator,
    DramAllocator* dram_allocator, PackageVerifier* verifier)
    : package_buffer_(package_buffer),
      package_(flatbuffers::GetRoot<Package>(package_buffer.ptr())),
      verifier_(verifier),
      parameter_caching_reference_(new driver::ExecutableReference(
          parameter_caching_executable, allocator, dram_allocator, this)),
      inference_reference_(new driver::ExecutableReference(
          inference_executable, allocator, dram_allocator, this)) {}

std::vector<driver::ExecutableReference*>
PackageReference::AllExecutableReferences() const {
  std::vector<driver::ExecutableReference*> all_references;
  if (standalone_reference_ != nullptr) {
    all_references.push_back(standalone_reference_.get());
  }
  if (parameter_caching_reference_ != nullptr) {
    all_references.push_back(parameter_caching_reference_.get());
  }
  if (inference_reference_ != nullptr) {
    all_references.push_back(inference_reference_.get());
  }
  return all_references;
}

Status PackageReference::UnmapParameters() {
  Status status;

  for (ExecutableReference* executable_ref : AllExecutableReferences()) {
    status.Update(executable_ref->UnmapParameters());
  }

  return status;
}

StatusOr<bool> PackageReference::ParametersMapped() const {
  auto all_executable_refs = AllExecutableReferences();
  if (all_executable_refs.empty()) {
    return FailedPreconditionError(
        "No executable references were found in the package reference.");
  }
  bool parameters_mapped = all_executable_refs.front()->ParametersMapped();

  for (auto* executable_ref : all_executable_refs) {
    if (executable_ref->ParametersMapped() != parameters_mapped) {
      return InternalError(
          "Inconsistent parameter mapping status across executables in the "
          "same package.");
    }
  }

  return parameters_mapped;
}

bool PackageReference::NeedsDram() const {
  auto all_executable_refs = AllExecutableReferences();

  for (auto* executable_ref : all_executable_refs) {
    if (executable_ref->NeedsDram()) {
      return true;
    }
  }

  return false;
}

Status PackageReference::SetLatencyTolerance(int64 latency_tolerance_ms) {
  latency_tolerance_ms_ = latency_tolerance_ms;
  return OkStatus();
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
