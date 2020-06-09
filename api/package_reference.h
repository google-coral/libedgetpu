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

#ifndef DARWINN_API_PACKAGE_REFERENCE_H_
#define DARWINN_API_PACKAGE_REFERENCE_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "api/execution_context_interface.h"
#include "api/layer_information.h"
#include "executable/executable_generated.h"
#include "port/integral_types.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {
namespace api {

// Specifies the most recent package identifier for executable.fbs.
constexpr const char* kHeadPackageIdentifier = "DWN1";

// Type for a registered executable.
class PackageReference {
 public:
  virtual ~PackageReference() = default;

  // This class is neither copyable nor movable.
  PackageReference(const PackageReference&) = delete;
  PackageReference& operator=(const PackageReference&) = delete;

  // Verifies the digital signature of the backing executable package.
  virtual util::Status VerifySignature() const = 0;

  // Returns the index of input layer with given name.
  virtual util::StatusOr<int> InputIndex(const std::string& name) const = 0;

  // Returns the index of output layer with given name.
  virtual util::StatusOr<int> OutputIndex(const std::string& name) const = 0;

  // Returns number of input layers.
  virtual int NumInputLayers() const = 0;

  // Returns number of output layers.
  virtual int NumOutputLayers() const = 0;

  // Returns list of input layer names.
  virtual const std::vector<std::string>& InputLayerNames() const = 0;

  // Returns list of output layer names.
  virtual const std::vector<std::string>& OutputLayerNames() const = 0;

  // Returns information on given input layer. Returns nullptr if index is out
  // of bounds.
  virtual const InputLayerInformation* InputLayer(int index) const = 0;

  // Returns information on given output layer. Returns nullptr if index is out
  // of bounds.
  virtual const OutputLayerInformation* OutputLayer(int index) const = 0;

  // Returns information on given input layer.
  virtual util::StatusOr<const api::InputLayerInformation*> InputLayer(
      const std::string& layer_name) const = 0;

  // Returns information on given output layer.
  virtual util::StatusOr<const api::OutputLayerInformation*> OutputLayer(
      const std::string& layer_name) const = 0;

  // Returns the expected byte size of activations for given input layer index.
  virtual int InputLayerSizeBytes(int index) const = 0;

  // Returns the expected byte size of activations for given input layer index.
  // This is post-padding, if any.
  // TODO Remove this method.
  virtual int InputLayerPaddedSizeBytes(int index) const = 0;

  // Returns the expected byte size of activations for given output layer index.
  virtual int OutputLayerSizeBytes(int index) const = 0;

  // Returns the expected size (in value count) of activations for given input
  // layer index. This is pre-padding, if any.
  virtual int InputLayerSize(int index) const = 0;

  // Returns the expected size (in value count) of activations for given input
  // layer index. This is pre-padding, if any.
  virtual int OutputLayerSize(int index) const = 0;

  // Returns the expected size of activations for given input layer.
  // Prefer index based APIs for performance.
  virtual util::StatusOr<int> InputLayerSizeBytes(
      const std::string& name) const = 0;

  // Returns the expected size of activations for given input layer including
  // padding bytes.
  // Prefer index based APIs for performance.
  // TODO Remove this method.
  virtual util::StatusOr<int> InputLayerPaddedSizeBytes(
      const std::string& name) const = 0;

  // Returns the expected size of activations for given output layer.
  // Prefer index based APIs for performance.
  virtual util::StatusOr<int> OutputLayerSizeBytes(
      const std::string& name) const = 0;

  // Returns name for given input layer index.
  virtual std::string InputLayerName(int index) const = 0;

  // Returns name for given output layer index.
  virtual std::string OutputLayerName(int index) const = 0;

  // Returns batch size.
  virtual int BatchSize() const = 0;

  // Sets the execution context (info related to execution). The execution
  // context is later used for logging purposes.
  virtual void SetExecutionContextInterface(
      std::unique_ptr<ExecutionContextInterface>
          execution_context_interface) = 0;

  // Sets the maximum amount of time this package can tolerate for an inference
  // to finish. Setting this will make driver check if it can meet the latency
  // target on each inference. If it cannot, it will immediately return a
  // deadline exceeded error. Parameter-caching or anything extra that driver
  // needs to run in order to complete an inference will be counted towards this
  // target. If a batch request is submitted, the total time to complete the
  // batch is counted (not a single batch element).
  virtual util::Status SetLatencyTolerance(int64 max_latency_ms) = 0;

  // Returns a unique user-specified string identifies the model. It returns
  // empty string if no identifier is set. This is available for limited cases
  // only.
  virtual std::string ModelIdentifier() const = 0;

 protected:
  PackageReference() = default;
};

}  // namespace api
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_API_PACKAGE_REFERENCE_H_

