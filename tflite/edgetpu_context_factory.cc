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

#include "tflite/edgetpu_context_factory.h"

#include "port/errors.h"
#include "port/logging.h"
#include "port/ptr_util.h"
#include "port/stringprintf.h"
#include "tflite/edgetpu_context_direct.h"

namespace platforms {
namespace darwinn {
namespace tflite {

const char* EdgeTpuContextFactory::GetDescriptionForDeviceTypeOptions() {
  static const std::string desc = [] {
    auto s = StringPrintf(
        "Type of Edge TPU device. Possible choices are %s | %s | %s",
        kDeviceTypeDefault, kDeviceTypeApexUsb, kDeviceTypeApexPci);
    return s;
  }();

  return desc.c_str();
}

const char* EdgeTpuContextFactory::GetDescriptionForDevicePathOptions() {
  static const char* desc = "Path to Edge TPU device.";
  return desc;
}

const char*
EdgeTpuContextFactory::GetDescriptionForPerformanceExpectationOptions() {
  static const char* desc =
      "Clock rate settings affecting performance: 0: Low, 1: Medium, "
      "2: High (default), 3: Max";
  return desc;
}

StatusOr<std::unique_ptr<edgetpu::EdgeTpuContext>>
EdgeTpuContextFactory::CreateEdgeTpuContext(const std::string& device_type,
                                            const std::string& device_path,
                                            int performance_expectation) {
  auto tpu_manager = edgetpu::EdgeTpuManager::GetSingleton();

  if (tpu_manager == nullptr) {
    // Return a null context when running on NNAPI
    return std::unique_ptr<edgetpu::EdgeTpuContext>(nullptr);
  }

  edgetpu::DeviceType device_type_enum;
  if (device_type == kDeviceTypeDefault) {
    if (device_path != kDevicePathDefault) {
      return InvalidArgumentError(
          StringPrintf("device_path must be %s when device_type is %s",
                       kDevicePathDefault, kDeviceTypeDefault));
    }

    if (performance_expectation != kPerformanceExpectationDefault) {
      return InvalidArgumentError(StringPrintf(
          "performance_expectation has no effect when device_type is %s",
          kDeviceTypeDefault));
    }

    auto result = tpu_manager->NewEdgeTpuContext();
    if (!result) {
      return NotFoundError("Failed opening default Edge TPU.");
    }
    return std::move(result);
  } else if (device_type == kDeviceTypeApexUsb) {
    device_type_enum = edgetpu::DeviceType::kApexUsb;
  } else if (device_type == kDeviceTypeApexPci) {
    device_type_enum = edgetpu::DeviceType::kApexPci;
  } else if (device_type == kDeviceTypeApexReference) {
    device_type_enum =
        static_cast<edgetpu::DeviceType>(DeviceTypeExtended::kApexReference);
  } else {
    return InvalidArgumentError("Unrecognized device type.");
  }

  std::string performance_expectation_str;
  switch (performance_expectation) {
    case 0:
      performance_expectation_str = "Low";
      break;
    case 1:
      performance_expectation_str = "Medium";
      break;
    case 2:
      performance_expectation_str = "High";
      break;
    case 3:
      performance_expectation_str = "Max";
      break;
    default:
      LOG(FATAL) << "Unrecognized performance expectation.";
  }

  auto result = tpu_manager->NewEdgeTpuContext(
      device_type_enum, device_path,
      {{"Performance", performance_expectation_str}});
  if (!result) {
    return NotFoundError("Failed opening specified Edge TPU.");
  }

  return std::move(result);
}

StatusOr<std::shared_ptr<edgetpu::EdgeTpuContext>>
EdgeTpuContextFactory::OpenEdgeTpuContext(const std::string& device_type,
                                          const std::string& device_path,
                                          int performance_expectation) {
  auto tpu_manager = edgetpu::EdgeTpuManager::GetSingleton();

  if (tpu_manager == nullptr) {
    // Return a null context when running on NNAPI
    return std::shared_ptr<edgetpu::EdgeTpuContext>(nullptr);
  }

  edgetpu::DeviceType device_type_enum;
  if (device_type == kDeviceTypeDefault) {
    if (device_path != kDevicePathDefault) {
      return InvalidArgumentError(
          StringPrintf("device_path must be %s when device_type is %s",
                       kDevicePathDefault, kDeviceTypeDefault));
    }

    if (performance_expectation != kPerformanceExpectationDefault) {
      return InvalidArgumentError(StringPrintf(
          "performance_expectation has no effect when device_type is %s",
          kDeviceTypeDefault));
    }

    auto result = tpu_manager->OpenDevice();
    if (!result) {
      return NotFoundError("Failed opening default Edge TPU.");
    }
    return std::move(result);
  } else if (device_type == kDeviceTypeApexUsb) {
    device_type_enum = edgetpu::DeviceType::kApexUsb;
  } else if (device_type == kDeviceTypeApexPci) {
    device_type_enum = edgetpu::DeviceType::kApexPci;
  } else if (device_type == kDeviceTypeApexReference) {
    device_type_enum =
        static_cast<edgetpu::DeviceType>(DeviceTypeExtended::kApexReference);
  } else {
    return InvalidArgumentError("Unrecognized device type.");
  }

  std::string performance_expectation_str;
  switch (performance_expectation) {
    case 0:
      performance_expectation_str = "Low";
      break;
    case 1:
      performance_expectation_str = "Medium";
      break;
    case 2:
      performance_expectation_str = "High";
      break;
    case 3:
      performance_expectation_str = "Max";
      break;
    default:
      LOG(FATAL) << "Unrecognized performance expectation.";
  }

  auto result =
      tpu_manager->OpenDevice(device_type_enum, device_path,
                              {{"Performance", performance_expectation_str}});
  if (!result) {
    return NotFoundError("Failed opening specified Edge TPU.");
  }

  return std::move(result);
}

StatusOr<std::vector<edgetpu::EdgeTpuManager::DeviceEnumerationRecord>>
EdgeTpuContextFactory::EnumerateEdgeTpu(const std::string& device_type) {
  auto tpu_manager = edgetpu::EdgeTpuManager::GetSingleton();

  if (tpu_manager == nullptr) {
    return FailedPreconditionError("Cannot enumerate NNAPI devices.");
  }

  auto devices = tpu_manager->EnumerateEdgeTpu();

  if (device_type != kDeviceTypeDefault) {
    edgetpu::DeviceType device_type_enum;

    if (device_type == kDeviceTypeApexUsb) {
      device_type_enum = edgetpu::DeviceType::kApexUsb;
    } else if (device_type == kDeviceTypeApexPci) {
      device_type_enum = edgetpu::DeviceType::kApexPci;
    } else if (device_type == kDeviceTypeApexReference) {
      device_type_enum =
          static_cast<edgetpu::DeviceType>(DeviceTypeExtended::kApexReference);
    } else {
      return InvalidArgumentError("Unrecognized device type.");
    }

    // Filter out all devices not of the specified type.
    for (auto it = devices.begin(); it != devices.end();) {
      if (it->type != device_type_enum) {
        it = devices.erase(it);
      } else {
        ++it;
      }
    }
  }

  if (devices.empty()) {
    return NotFoundError("Failed finding any Edge TPU of specified type.");
  }

  return devices;
}

}  // namespace tflite
}  // namespace darwinn
}  // namespace platforms
