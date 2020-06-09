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

#ifndef DARWINN_TFLITE_EDGETPU_MANAGER_DIRECT_H_
#define DARWINN_TFLITE_EDGETPU_MANAGER_DIRECT_H_

#include <mutex>  // NOLINT

#include "port/thread_annotations.h"
#include "tflite/edgetpu_context_direct.h"
#include "tflite/public/edgetpu.h"

namespace platforms {
namespace darwinn {
namespace tflite {

// TPU Manager implementation for direct API.
// This class is threadsafe, as multiple TPU contexts, driven by multiple
// Interpreter threads, could access this singleton at the same time.
class EdgeTpuManagerDirect : public edgetpu::EdgeTpuManager {
 public:
  virtual ~EdgeTpuManagerDirect() = default;

  static EdgeTpuManagerDirect* GetSingleton();

  std::unique_ptr<EdgeTpuContext> NewEdgeTpuContext() final
      LOCKS_EXCLUDED(mutex_);

  std::unique_ptr<EdgeTpuContext> NewEdgeTpuContext(
      DeviceType device_type) final LOCKS_EXCLUDED(mutex_);

  std::unique_ptr<EdgeTpuContext> NewEdgeTpuContext(
      DeviceType device_type, const std::string& device_path) final
      LOCKS_EXCLUDED(mutex_);

  std::unique_ptr<EdgeTpuContext> NewEdgeTpuContext(
      DeviceType device_type, const std::string& device_path,
      const EdgeTpuManager::DeviceOptions& options) final
      LOCKS_EXCLUDED(mutex_);

  std::vector<DeviceEnumerationRecord> EnumerateEdgeTpu() const final
      LOCKS_EXCLUDED(mutex_);

  std::shared_ptr<EdgeTpuContext> OpenDevice() final LOCKS_EXCLUDED(mutex_);

  std::shared_ptr<EdgeTpuContext> OpenDevice(DeviceType device_type) final
      LOCKS_EXCLUDED(mutex_);

  std::shared_ptr<EdgeTpuContext> OpenDevice(
      DeviceType device_type, const std::string& device_path) final
      LOCKS_EXCLUDED(mutex_);

  std::shared_ptr<EdgeTpuContext> OpenDevice(DeviceType device_type,
                                             const std::string& device_path,
                                             const DeviceOptions& options) final
      LOCKS_EXCLUDED(mutex_);

  std::vector<std::shared_ptr<EdgeTpuContext>> GetOpenedDevices() const final
      LOCKS_EXCLUDED(mutex_);

  TfLiteStatus SetVerbosity(int verbosity) final LOCKS_EXCLUDED(mutex_);

  std::string Version() const final LOCKS_EXCLUDED(mutex_);

  // Intended to be used by #~EdgeTpuContextDirect()
  // When EdgeTpuContextDirect is destructed, it would release reference to the
  // underlying EdgeTpuDriverWrapper with this call.
  void ReleaseEdgeTpuContext(EdgeTpuDriverWrapper* driver_wrapper)
      LOCKS_EXCLUDED(mutex_);

 private:
  EdgeTpuManagerDirect() = default;

  // Enumerates connected TPU devices.
  std::vector<DeviceEnumerationRecord> EnumerateEdgeTpuInternal() const
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Returns path to the first device of requested type which is not yet opened.
  // Reutnrs empty string if no such device can be found.
  std::string FindPathToFirstUnopenedDevice(
      const std::vector<DeviceEnumerationRecord>& candidates,
      edgetpu::DeviceType request_device_type) EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Returns a shared pointer to existing/opened driver wrapper of the specified
  // device types and path. The path argument can be empty
  // string so it matches with any device path.
  std::shared_ptr<EdgeTpuContext> TryMatchDriverWrapper(
      const std::vector<edgetpu::DeviceType>& extended_device_types,
      const std::string& extended_device_path) EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Constructs device driver instance, opens it, and then returns an unique
  // pointer to the driver wrapper.
  std::unique_ptr<EdgeTpuDriverWrapper> MakeDriverWrapper(
      edgetpu::DeviceType request_device_type,
      const std::string& extended_device_path,
      const EdgeTpuManager::DeviceOptions& options, bool exclusive_ownership)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Returns an unique pointer to EdgeTpuContext, which is opened for exclusive
  // ownership.
  //
  // #device_type can be kApexAny, as it's further extended by
  // ExtendRequestDeviceType. #device_path can be either empty or
  // api::DriverFactory::kDefaultDevicePath, both match to any device path.
  std::unique_ptr<EdgeTpuContext> NewEdgeTpuContextInternal(
      DeviceTypeExtended device_type, const std::string& device_path,
      const EdgeTpuManager::DeviceOptions& options)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Returns a shared pointer to EdgeTpuContext, which is opened for shared
  // ownership.
  //
  // #device_type can be kApexAny, as it's further extended by
  // ExtendRequestDeviceType. #device_path can be either empty or
  // api::DriverFactory::kDefaultDevicePath, both match to any device path.
  std::shared_ptr<EdgeTpuContext> OpenDeviceInternal(
      DeviceTypeExtended device_type, const std::string& device_path,
      const EdgeTpuManager::DeviceOptions& options)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Extends wildcard device type to a list of types.
  static std::vector<edgetpu::DeviceType> ExtendRequestDeviceType(
      DeviceTypeExtended device_type);

  // Serializes access to this singleton.
  mutable std::mutex mutex_;

  std::vector<std::unique_ptr<EdgeTpuDriverWrapper>> opened_devices_
      GUARDED_BY(mutex_);
};

}  // namespace tflite
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_TFLITE_EDGETPU_MANAGER_DIRECT_H_
