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

#include "tflite/edgetpu_manager_direct.h"

#include <memory>

#include "absl/strings/str_format.h"
#include "api/driver_factory.h"
#include "api/driver_options_generated.h"
#include "api/runtime_version.h"
#include "port/builddata.h"
#include "port/defs.h"
#include "port/logging.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"
#include "tflite/edgetpu_context_direct.h"

namespace platforms {
namespace darwinn {
namespace tflite {

using edgetpu::EdgeTpuContext;

EdgeTpuManagerDirect* EdgeTpuManagerDirect::GetSingleton() {
  // Static objects with non-trivial destructors shouldn't be deleted,
  // according to coding style requirement.
  static auto* const impl =
      new platforms::darwinn::tflite::EdgeTpuManagerDirect();
  return impl;
}

std::unique_ptr<EdgeTpuContext> EdgeTpuManagerDirect::NewEdgeTpuContext() {
  StdMutexLock lock(&mutex_);

  return NewEdgeTpuContextInternal(DeviceTypeExtended::kApexAny, std::string(),
                                   EdgeTpuManager::DeviceOptions());
}

std::unique_ptr<edgetpu::EdgeTpuContext>
EdgeTpuManagerDirect::NewEdgeTpuContext(edgetpu::DeviceType device_type) {
  StdMutexLock lock(&mutex_);

  return NewEdgeTpuContextInternal(static_cast<DeviceTypeExtended>(device_type),
                                   std::string(),
                                   EdgeTpuManager::DeviceOptions());
}

std::unique_ptr<edgetpu::EdgeTpuContext>
EdgeTpuManagerDirect::NewEdgeTpuContext(edgetpu::DeviceType device_type,
                                        const std::string& device_path) {
  StdMutexLock lock(&mutex_);

#if defined(THROTTLE_EDGE_TPU) && THROTTLE_EDGE_TPU != 0
  // In some cases, we need to throttle edgetpu, see b/119426047 for more
  // context. Throttling only applies when EdgeTpu is connected through USB.
  // THROTTLE_EDGE_TPU = undefined/0: Max; 1: High; 2: Med; 3: Low; others: High
  if (device_type == edgetpu::DeviceType::kApexUsb) {
    VLOG(2) << "EdgeTpu is throttled.";
    std::string performance_str = "High";
#if THROTTLE_EDGE_TPU == 2
    performance_str = "Medium";
#elif THROTTLE_EDGE_TPU == 3
    performance_str = "Low";
#endif
    return NewEdgeTpuContextInternal(
        static_cast<DeviceTypeExtended>(device_type), device_path,
        {{"Performance", performance_str}});
  }
#endif

  return NewEdgeTpuContextInternal(static_cast<DeviceTypeExtended>(device_type),
                                   device_path,
                                   EdgeTpuManager::DeviceOptions());
}

std::unique_ptr<edgetpu::EdgeTpuContext>
EdgeTpuManagerDirect::NewEdgeTpuContext(
    edgetpu::DeviceType device_type, const std::string& device_path,
    const EdgeTpuManager::DeviceOptions& options) {
  StdMutexLock lock(&mutex_);

  return NewEdgeTpuContextInternal(static_cast<DeviceTypeExtended>(device_type),
                                   device_path, options);
}

std::vector<EdgeTpuManager::DeviceEnumerationRecord>
EdgeTpuManagerDirect::EnumerateEdgeTpu() const {
  StdMutexLock lock(&mutex_);

  return EnumerateEdgeTpuInternal();
}

std::shared_ptr<EdgeTpuContext> EdgeTpuManagerDirect::OpenDevice() {
  StdMutexLock lock(&mutex_);

  return OpenDeviceInternal(DeviceTypeExtended::kApexAny, std::string(),
                            EdgeTpuManager::DeviceOptions());
}

std::shared_ptr<edgetpu::EdgeTpuContext> EdgeTpuManagerDirect::OpenDevice(
    edgetpu::DeviceType device_type) {
  StdMutexLock lock(&mutex_);

  return OpenDeviceInternal(static_cast<DeviceTypeExtended>(device_type),
                            std::string(), EdgeTpuManager::DeviceOptions());
}

std::shared_ptr<edgetpu::EdgeTpuContext> EdgeTpuManagerDirect::OpenDevice(
    edgetpu::DeviceType device_type, const std::string& device_path) {
  StdMutexLock lock(&mutex_);

#if defined(THROTTLE_EDGE_TPU) && THROTTLE_EDGE_TPU != 0
  // In some cases, we need to throttle edgetpu, see b/119426047 for more
  // context. Throttling only applies when EdgeTpu is connected through USB.
  // THROTTLE_EDGE_TPU = undefined/0: Max; 1: High; 2: Med; 3: Low; others: High
  if (device_type == edgetpu::DeviceType::kApexUsb) {
    VLOG(2) << "EdgeTpu is throttled.";
    std::string performance_str = "High";
#if THROTTLE_EDGE_TPU == 2
    performance_str = "Medium";
#elif THROTTLE_EDGE_TPU == 3
    performance_str = "Low";
#endif
    return OpenDeviceInternal(static_cast<DeviceTypeExtended>(device_type),
                              device_path,
                              {{"Performance", performance_str}});
  }
#endif

  return OpenDeviceInternal(static_cast<DeviceTypeExtended>(device_type),
                            device_path, EdgeTpuManager::DeviceOptions());
}

std::shared_ptr<edgetpu::EdgeTpuContext> EdgeTpuManagerDirect::OpenDevice(
    edgetpu::DeviceType device_type, const std::string& device_path,
    const EdgeTpuManager::DeviceOptions& options) {
  StdMutexLock lock(&mutex_);

  return OpenDeviceInternal(static_cast<DeviceTypeExtended>(device_type),
                            device_path, options);
}

std::vector<std::shared_ptr<EdgeTpuContext>>
EdgeTpuManagerDirect::GetOpenedDevices() const {
  StdMutexLock lock(&mutex_);

  std::vector<std::shared_ptr<EdgeTpuContext>> results;

  for (auto& drvier_wrapper : opened_devices_) {
    if (drvier_wrapper->IsExclusivelyOwned()) {
      // Skips devices that are not sharable
      continue;
    }

    auto shared_context =
        std::make_shared<EdgeTpuContextDirect>(drvier_wrapper.get());

    // Note that we only keeps the weak pointer, as usually it is
    results.push_back(shared_context);
  }

  return results;
}

TfLiteStatus EdgeTpuManagerDirect::SetVerbosity(int verbosity) {
  StdMutexLock lock(&mutex_);

  if (verbosity < 0 || verbosity > 10) {
    return kTfLiteError;
  }

  // Update verbosity level.
  // TODO: Verbosity level should be of per driver instance.
#if !DARWINN_PORT_USE_GOOGLE3
  ::platforms::darwinn::internal::SetLoggingLevel(verbosity);
  return kTfLiteOk;
#else
  // Assume FLAGS_v is defined.
  absl::SetFlag(&FLAGS_v, verbosity);
  return kTfLiteOk;
#endif
}

std::string EdgeTpuManagerDirect::Version() const {
  StdMutexLock lock(&mutex_);

  absl::string_view build_label = BuildData::BuildLabel();
  // Note that runtime version reported here is correct only if all driver
  // providers are built at the same time with this compile unit.
  if (build_label.empty()) {
    return StringPrintf("BuildLabel(N/A), RuntimeVersion(%d)",
                        api::RuntimeVersion::kCurrent);
  } else {
    return absl::StrFormat("BuildLabel(%s), RuntimeVersion(%d)", build_label,
                           api::RuntimeVersion::kCurrent);
  }
}

void EdgeTpuManagerDirect::ReleaseEdgeTpuContext(
    EdgeTpuDriverWrapper* driver_wrapper) {
  StdMutexLock lock(&mutex_);

  for (auto it = opened_devices_.begin(); it != opened_devices_.end(); ++it) {
    if (it->get() == driver_wrapper) {
      // Perform weak_ptr::lock here and create a new instance of shared_ptr to
      // the context object.
      auto use_count = (*it)->Release();
      if (use_count > 0) {
        // This could happen when the last shared pointer to the context object
        // is
        VLOG(1) << "Edge TPU device at " << (*it)->GetDeviceEnumRecord().path
                << " is still in use.";
      } else {
        VLOG(4) << "Releasing Edge TPU device at "
                << (*it)->GetDeviceEnumRecord().path;
        opened_devices_.erase(it);
      }
      return;
    }
  }

  LOG(FATAL) << "Could not find specified Edge TPU context to close.";
}

std::vector<EdgeTpuManager::DeviceEnumerationRecord>
EdgeTpuManagerDirect::EnumerateEdgeTpuInternal() const {
  std::vector<edgetpu::EdgeTpuManager::DeviceEnumerationRecord> result;

  auto factory = api::DriverFactory::GetOrCreate();
  if (!factory) {
    VLOG(1) << "Failed to create driver factory.";
    return result;
  }

  auto devices = factory->Enumerate();

  for (const auto& device : devices) {
    edgetpu::DeviceType device_type =
        static_cast<edgetpu::DeviceType>(DeviceTypeExtended::kUnknown);
    if (device.chip == api::Chip::kBeagle) {
      switch (device.type) {
        case api::Device::Type::PCI:
          device_type = edgetpu::DeviceType::kApexPci;
          break;
        case api::Device::Type::USB:
          device_type = edgetpu::DeviceType::kApexUsb;
          break;
        case api::Device::Type::REFERENCE:
          device_type = static_cast<edgetpu::DeviceType>(
              DeviceTypeExtended::kApexReference);
          break;
        default:
          VLOG(7) << "Skipping unrecognized device type: "
                  << static_cast<int>(device.type);
          continue;
      }
    } else {
      VLOG(7) << "Skipping unrecognized Edge TPU type: "
              << static_cast<int>(device.chip);
      continue;
    }

    result.push_back(edgetpu::EdgeTpuManager::DeviceEnumerationRecord{
        device_type, device.path});
  }
  return result;
}

std::string EdgeTpuManagerDirect::FindPathToFirstUnopenedDevice(
    const std::vector<EdgeTpuManager::DeviceEnumerationRecord>& candidates,
    edgetpu::DeviceType request_device_type) {
  for (auto& device : candidates) {
    if (request_device_type != device.type) {
      // Skips devices of different type.
      continue;
    }

    bool is_opened = false;

    // So this candidate is of the same type as requested.
    // Check if it's already opened.
    for (auto& drvier_wrapper : opened_devices_) {
      const auto& enum_record = drvier_wrapper->GetDeviceEnumRecord();

      if ((device.type == enum_record.type) &&
          (device.path == enum_record.path)) {
        // A device of the same type and path is already registereed as open.
        is_opened = true;
        break;
      }
    }

    if (!is_opened) {
      // We just found a device of the requested type and is not opened yet.
      return device.path;
    }
  }

  // We couldn't find any suitable device.
  return std::string();
}

std::shared_ptr<EdgeTpuContext> EdgeTpuManagerDirect::TryMatchDriverWrapper(
    const std::vector<edgetpu::DeviceType>& extended_device_types,
    const std::string& extended_device_path) {
  // Iterates through all requested device types.
  for (auto request_device_type : extended_device_types) {
    for (auto& drvier_wrapper : opened_devices_) {
      const auto& enum_record = drvier_wrapper->GetDeviceEnumRecord();
      if (request_device_type != enum_record.type) {
        // Skips devices of different type.
        continue;
      }

      // Only check the path if it's not empty.
      if (!extended_device_path.empty()) {
        if (extended_device_path != enum_record.path) {
          // Skips devices of different path.
          continue;
        }
      }

      if (drvier_wrapper->IsExclusivelyOwned()) {
        // Skips devices that are not sharable
        continue;
      }

      // Now we finally find a device to be shared.
      // Note we don't check for compatibility in options.
      return std::make_shared<EdgeTpuContextDirect>(drvier_wrapper.get());
    }
  }

  // Returns null pointer in case we cannot find a matching device.
  return std::shared_ptr<EdgeTpuContext>();
}

std::unique_ptr<EdgeTpuDriverWrapper> EdgeTpuManagerDirect::MakeDriverWrapper(
    edgetpu::DeviceType request_device_type,
    const std::string& extended_device_path,
    const EdgeTpuManager::DeviceOptions& options, bool exclusive_ownership) {
  auto driver = EdgeTpuDriverWrapper::MakeOpenedDriver(
      request_device_type, extended_device_path, options);

  if (driver) {
    EdgeTpuManager::DeviceEnumerationRecord enum_record;
    enum_record.path = extended_device_path;
    enum_record.type = request_device_type;
    return gtl::MakeUnique<EdgeTpuDriverWrapper>(std::move(driver), enum_record,
                                                 options, exclusive_ownership);
  }

  // In case we cannot create a new driver with its wrapper, return a null
  // pointer.
  return std::unique_ptr<EdgeTpuDriverWrapper>();
}

std::unique_ptr<EdgeTpuContext> EdgeTpuManagerDirect::NewEdgeTpuContextInternal(
    DeviceTypeExtended device_type, const std::string& device_path,
    const EdgeTpuManager::DeviceOptions& options) {
  bool allow_any_path = false;
  auto extended_device_types = ExtendRequestDeviceType(device_type);
  auto extended_device_path = device_path;
  if (extended_device_path.empty() ||
      (extended_device_path == api::DriverFactory::kDefaultDevicePath)) {
    allow_any_path = true;
  }

  // Get all connected devices. Note this enumeration results do not consider if
  // the devices have been opened or not.
  auto candidates = EnumerateEdgeTpuInternal();

  // Iterates through all requested device types.
  for (auto request_device_type : extended_device_types) {
    if (allow_any_path) {
      // Overwrites extended_device_path here, as driver factory
      // doesn't actually report back the exact path of the opened device.
      extended_device_path =
          FindPathToFirstUnopenedDevice(candidates, request_device_type);

      if (extended_device_path.empty()) {
        // There is no un-opened device of this particular type.
        // It's okay to leave extended_device_path as empty, for we already have
        // allow_any_path set to true.
        VLOG(5) << "No device of type "
                << EdgeTpuDriverWrapper::GetDeviceTypeName(
                       static_cast<DeviceType>(request_device_type))
                << " is available.";
        continue;
      }
    }

    // We have a path. Now try to open and creates a wrapper for it.
    std::unique_ptr<EdgeTpuDriverWrapper> driver_wrapper =
        MakeDriverWrapper(request_device_type, extended_device_path, options,
                          /*exclusive_ownership=*/true);

    if (!driver_wrapper) {
      VLOG(1)
          << "Failed creating new Edge TPU context for exclusive ownership.";

      // Returns a null pointer on error.
      return std::unique_ptr<EdgeTpuContext>();
    }

    EdgeTpuDriverWrapper* wrapper_pointer = driver_wrapper.get();

    // Commits the driver wrapper into opened devices.
    opened_devices_.push_back(std::move(driver_wrapper));

    return gtl::MakeUnique<EdgeTpuContextDirect>(wrapper_pointer);
  }

  VLOG(1) << "Failed allocating Edge TPU device for exclusive ownership.";

  return std::unique_ptr<EdgeTpuContextDirect>();
}

std::shared_ptr<EdgeTpuContext> EdgeTpuManagerDirect::OpenDeviceInternal(
    DeviceTypeExtended device_type, const std::string& device_path,
    const EdgeTpuManager::DeviceOptions& options) {
  bool allow_any_path = false;
  auto extended_device_types = ExtendRequestDeviceType(device_type);
  auto extended_device_path = device_path;
  if (extended_device_path.empty() ||
      (extended_device_path == api::DriverFactory::kDefaultDevicePath)) {
    allow_any_path = true;
    extended_device_path.clear();
  }

  // Tries to find a match in all the device types.
  // The returned shared_ptr would prevent the device from being closed.
  auto context =
      TryMatchDriverWrapper(extended_device_types, extended_device_path);

  if (context) {
    // Returns the opened device.
    return context;
  }

  VLOG(5) << "No matching device is already opened for shared ownership.";

  // Get all connected devices. Note this enumeration results do not consider if
  // the devices have been opened or not.
  auto candidates = EnumerateEdgeTpuInternal();

  // Iterates through all requested device types.
  for (auto request_device_type : extended_device_types) {
    if (allow_any_path) {
      // We have to overwrite extended_device_path here, as driver factory
      // doesn't actually report back the exact path of the opened device.
      extended_device_path =
          FindPathToFirstUnopenedDevice(candidates, request_device_type);

      if (extended_device_path.empty()) {
        // There is no un-opened device of this particular type.
        // It's okay to leave extended_device_path as empty, for we already have
        // allow_any_path set to true.
        VLOG(5) << "No device of type "
                << EdgeTpuDriverWrapper::GetDeviceTypeName(
                       static_cast<DeviceType>(request_device_type))
                << " is available.";
        continue;
      }
    }

    EdgeTpuManager::DeviceOptions device_options(options);
    if (request_device_type == edgetpu::DeviceType::kApexUsb &&
        device_options.find("Usb.MaxBulkInQueueLength") ==
            device_options.end()) {
      device_options["Usb.MaxBulkInQueueLength"] = "8";
    }
    std::unique_ptr<EdgeTpuDriverWrapper> driver_wrapper = MakeDriverWrapper(
        request_device_type, extended_device_path, device_options,
        /*exclusive_ownership=*/false);

    if (!driver_wrapper) {
      // Returns a null pointer on error.
      return std::shared_ptr<EdgeTpuContext>();
    }

    EdgeTpuDriverWrapper* wrapper_pointer = driver_wrapper.get();

    // Commits the driver wrapper into opened devices.
    opened_devices_.push_back(std::move(driver_wrapper));

    return std::make_shared<EdgeTpuContextDirect>(wrapper_pointer);
  }

  VLOG(1) << "Failed allocating Edge TPU device for shared ownership.";

  return std::shared_ptr<EdgeTpuContext>();
}

std::vector<edgetpu::DeviceType> EdgeTpuManagerDirect::ExtendRequestDeviceType(
    DeviceTypeExtended device_type) {
  std::vector<edgetpu::DeviceType> request_device_types;

  if (device_type == DeviceTypeExtended::kApexAny) {
    // If the device type is Apex Any, try all supported types
    // 1st priority: PCIe
    request_device_types.push_back(edgetpu::DeviceType::kApexPci);

    // 2nd priority: USB
    request_device_types.push_back(edgetpu::DeviceType::kApexUsb);

    // 3rd priority: Reference device
    request_device_types.push_back(
        static_cast<edgetpu::DeviceType>(DeviceTypeExtended::kApexReference));
  } else {
    request_device_types.push_back(
        static_cast<edgetpu::DeviceType>(device_type));
  }

  return request_device_types;
}

}  // namespace tflite
}  // namespace darwinn
}  // namespace platforms
