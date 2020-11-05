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

#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/types/optional.h"
#include "tflite/edgetpu_delegate_for_custom_op.h"
#include "tflite/public/edgetpu.h"
#include "tensorflow/lite/builtin_op_data.h"

namespace {

using edgetpu::DeviceType;
using edgetpu::EdgeTpuContext;
using edgetpu::EdgeTpuManager;
using DeviceOptions = EdgeTpuManager::DeviceOptions;

typedef void (*ErrorHandler)(const char*);

constexpr char kUsb[] = "usb";
constexpr char kPci[] = "pci";
constexpr char kOptionDevice[] = "device";

bool MatchDevice(const std::string& s, const std::string& type, int* index) {
  const auto prefix(type + ":");
  if (!absl::StartsWith(s, prefix)) return false;
  if (!absl::SimpleAtoi(s.substr(prefix.size()), index)) return false;
  if (*index < 0) return false;
  return true;
}

std::shared_ptr<edgetpu::EdgeTpuContext> GetEdgeTpuContext(
    absl::optional<DeviceType> device_type, absl::optional<int> device_index,
    const edgetpu::EdgeTpuManager::DeviceOptions& options) {
  auto* manager = edgetpu::EdgeTpuManager::GetSingleton();
  if (!device_index.has_value()) {
    return device_type.has_value() ? manager->OpenDevice(device_type.value())
                                   : manager->OpenDevice();
  } else {
    const int index = device_index.value();
    auto tpus = manager->EnumerateEdgeTpu();
    if (device_type.has_value()) {
      int i = 0;
      for (auto& record : tpus)
        if (record.type == device_type.value() && i++ == index)
          return manager->OpenDevice(record.type, record.path, options);
    } else {
      if (index < tpus.size())
        return manager->OpenDevice(tpus[index].type, tpus[index].path, options);
    }
    return nullptr;
  }
}

std::shared_ptr<EdgeTpuContext> GetEdgeTpuContext(
    const DeviceOptions& options) {
  auto it = options.find(kOptionDevice);
  if (it == options.end()) {
    return EdgeTpuManager::GetSingleton()->OpenDevice();
  } else {
    const auto& device = it->second;
    if (device.empty()) {
      return GetEdgeTpuContext(absl::nullopt, absl::nullopt, options);
    } else if (device == kUsb) {
      return GetEdgeTpuContext(DeviceType::kApexUsb, absl::nullopt, options);
    } else if (device == kPci) {
      return GetEdgeTpuContext(DeviceType::kApexPci, absl::nullopt, options);
    } else {
      int index;
      if (MatchDevice(device, "", &index)) {
        return GetEdgeTpuContext(absl::nullopt, index, options);
      } else if (MatchDevice(device, kUsb, &index)) {
        return GetEdgeTpuContext(DeviceType::kApexUsb, index, options);
      } else if (MatchDevice(device, kPci, &index)) {
        return GetEdgeTpuContext(DeviceType::kApexPci, index, options);
      } else {
        return nullptr;
      }
    }
  }
}
}  // namespace

extern "C" {

// Recognized input options:
//   "device", possible values:
//     ""             -- any TPU device
//     ":<index>"     -- TPU device of any type according to enumeration order
//     "usb"          -- any TPU device of USB type
//     "usb:<index>"  -- TPU device of USB type according to enumeration order
//     "pci"          -- any TPU device of PCIe type
//     "pci:<index>"  -- TPU device of PCIe type according to enumeration order
//
// Enumeration order is defined by `edgetpu::EdgeTpuManager::EnumerateEdgeTpu`
// call.
//
// All options are forwarded to `edgetpu::EdgeTpuManager::OpenDevice`
// call when "device" has a form of ":<index>", "usb:<index>", or "pci:<index>",
// i.e. the following are supported as well:
//   "Performance": ["Low", "Medium", "High", "Max"] (Default is "Max")
//   "Usb.AlwaysDfu": ["True", "False"] (Default is "False")
//   "Usb.MaxBulkInQueueLength": ["0",.., "255"] (Default is "32")
//
// Any availabe TPU device is used if "device" option is not specified.
EDGETPU_EXPORT TfLiteDelegate* tflite_plugin_create_delegate(
    char** options_keys, char** options_values, size_t num_options,
    ErrorHandler error_handler) {
  DeviceOptions options;
  for (size_t i = 0; i < num_options; ++i)
    options[options_keys[i]] = options_values[i];

  auto context = GetEdgeTpuContext(options);
  if (!context) return nullptr;
  return platforms::darwinn::tflite::CreateEdgeTpuDelegateForCustomOp(context);
}

EDGETPU_EXPORT void tflite_plugin_destroy_delegate(TfLiteDelegate* delegate) {
  platforms::darwinn::tflite::FreeEdgeTpuDelegateForCustomOp(delegate);
}

}  // extern "C"
