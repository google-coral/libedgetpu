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

#ifndef DARWINN_TFLITE_EDGE_TPU_CONTEXT_FACTORY_H_
#define DARWINN_TFLITE_EDGE_TPU_CONTEXT_FACTORY_H_

#include <memory>
#include <string>

#include "port/statusor.h"
#include "tflite/public/edgetpu.h"
#include "tensorflow/lite/context.h"

namespace platforms {
namespace darwinn {
namespace tflite {

// This class creates EdgeTPU context from various configurations.
// This class wraps edgetpu::EdgeTpuManager::NewEdgeTpuContext and makes it
// simpler to use with command line options.
class EdgeTpuContextFactory {
 public:
  static constexpr const char* kDeviceTypeDefault = "default";
  static constexpr const char* kDeviceTypeApexUsb = "apex_usb";
  static constexpr const char* kDeviceTypeApexPci = "apex_pci";

  // Internal-only option, which doesn't show up in
  // GetDescriptionForDeviceTypeOptions
  static constexpr const char* kDeviceTypeApexReference = "apex_ref";

  static constexpr const char* kDevicePathDefault = "default";

  static constexpr int kPerformanceExpectationDefault = 3;

  // Returns description for device type specifier.
  // Note that the return string is static to this class.
  static const char* GetDescriptionForDeviceTypeOptions();

  // Returns description for device path specifier.
  // Note that the return string is static to this class.
  static const char* GetDescriptionForDevicePathOptions();

  // Returns description for performance specifier.
  // Note that the return string is static to this class.
  static const char* GetDescriptionForPerformanceExpectationOptions();

  // Creates an EdgeTpu context holder on success.
  static StatusOr<std::unique_ptr<edgetpu::EdgeTpuContext>>
  CreateEdgeTpuContext(const std::string& device_type,
                       const std::string& device_path,
                       int performance_expectation);

  // Creates an EdgeTpu context holder on success, intended to be shared.
  static StatusOr<std::shared_ptr<edgetpu::EdgeTpuContext>> OpenEdgeTpuContext(
      const std::string& device_type, const std::string& device_path,
      int performance_expectation);

  // Enumerates Edge TPU devices of the specified type.
  static StatusOr<std::vector<edgetpu::EdgeTpuManager::DeviceEnumerationRecord>>
  EnumerateEdgeTpu(const std::string& device_type);
};

}  // namespace tflite
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_TFLITE_EDGE_TPU_CONTEXT_FACTORY_H_
