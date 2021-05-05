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

#ifndef DARWINN_TFLITE_EDGETPU_CONTEXT_DIRECT_H_
#define DARWINN_TFLITE_EDGETPU_CONTEXT_DIRECT_H_

#include <mutex>  // NOLINT

#include "api/driver.h"
#include "api/package_reference.h"
#include "port/errors.h"
#include "port/ptr_util.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/statusor.h"
#include "port/thread_annotations.h"
#include "tflite/custom_op_user_data_direct.h"
#include "tflite/public/edgetpu.h"

namespace platforms {
namespace darwinn {
namespace tflite {

using edgetpu::DeviceType;
using edgetpu::EdgeTpuContext;
using edgetpu::EdgeTpuManager;

// Internal-only extension to edgetpu::DeviceType
enum class DeviceTypeExtended {
  kExtendedBegin = 1000,
  kUnknown = kExtendedBegin + 0,
  kApexReference = kExtendedBegin + 1,
  kApexAny = kExtendedBegin + 2,
};

// Holds opened device through api::Driver interface.
class EdgeTpuDriverWrapper {
 public:
  // Constructs EdgeTpuDriverWrapper with an opened instance of the driver.
  EdgeTpuDriverWrapper(
      std::unique_ptr<api::Driver> driver,
      const EdgeTpuManager::DeviceEnumerationRecord& enum_record,
      const EdgeTpuManager::DeviceOptions options, bool exclusive_ownership);

  ~EdgeTpuDriverWrapper();

  // Returns the pointer to the driver.
  api::Driver* GetDriver() const LOCKS_EXCLUDED(mutex_);

  // Synchronously executes executables for this node, with the context object
  // locked.
  Status InvokeExecutable(TfLiteContext* context, TfLiteNode* node)
      LOCKS_EXCLUDED(mutex_);

  // Returns constant reference to enumeration record for this device.
  const EdgeTpuManager::DeviceEnumerationRecord& GetDeviceEnumRecord() const
      LOCKS_EXCLUDED(mutex_);

  // Returns a snapshot of device options and attributes.
  EdgeTpuManager::DeviceOptions GetDeviceOptions() const LOCKS_EXCLUDED(mutex_);

  // Intended to be used by #EdgeTpuContextDirect
  Status AddRef() LOCKS_EXCLUDED(mutex_);

  // Intended to be used by #EdgeTpuManagerDirect
  int Release() LOCKS_EXCLUDED(mutex_);

  // Returns true if the device is most likely ready to accept requests.
  // When there are fatal errors, including unplugging of an USB device, the
  // state of this device would be changed.
  bool IsReady() const LOCKS_EXCLUDED(mutex_);

  // Returns true if the device is exclusively owned by an unique_ptr.
  bool IsExclusivelyOwned() const LOCKS_EXCLUDED(mutex_);

  // Makes an new api::Driver and opens it, or nullptr on failure.
  static std::unique_ptr<api::Driver> MakeOpenedDriver(
      DeviceType device_type, const std::string& device_path,
      const EdgeTpuManager::DeviceOptions& options);

  // Returns name in string for device types.
  static const char* GetDeviceTypeName(edgetpu::DeviceType device_type);

 private:
  static const char* STATUS_IS_READY;
  static const char* STATUS_EXCLUSIVE_OWNERSHIP;

  // Serializes access to this device.
  mutable std::mutex mutex_;

  int use_count_ GUARDED_BY(mutex_){0};
  bool is_ready_ GUARDED_BY(mutex_){false};
  bool is_exclusively_owned_ GUARDED_BY(mutex_){false};
  std::unique_ptr<api::Driver> driver_ GUARDED_BY(mutex_);
  const EdgeTpuManager::DeviceEnumerationRecord enum_record_ GUARDED_BY(mutex_);
  const EdgeTpuManager::DeviceOptions options_ GUARDED_BY(mutex_);
};

class EdgeTpuContextDirect : public EdgeTpuContext {
 public:
  explicit EdgeTpuContextDirect(EdgeTpuDriverWrapper* driver_wrapper);

  ~EdgeTpuContextDirect();

  const EdgeTpuManager::DeviceEnumerationRecord& GetDeviceEnumRecord()
      const final {
    return driver_wrapper_->GetDeviceEnumRecord();
  }

  EdgeTpuManager::DeviceOptions GetDeviceOptions() const final {
    return driver_wrapper_->GetDeviceOptions();
  }

  bool IsReady() const final { return driver_wrapper_->IsReady(); }

  EdgeTpuDriverWrapper* GetDriverWrapper() const { return driver_wrapper_; }

 private:
  EdgeTpuDriverWrapper* driver_wrapper_{nullptr};
};

}  // namespace tflite
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_TFLITE_EDGETPU_CONTEXT_DIRECT_H_
