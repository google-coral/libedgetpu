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

#include "tflite/public/edgetpu_c.h"

#include <cstring>

#include "port/logging.h"
#include "tflite/edgetpu_delegate_for_custom_op.h"
#include "tflite/public/edgetpu.h"

using edgetpu::DeviceType;
using edgetpu::EdgeTpuContext;
using edgetpu::EdgeTpuManager;
using DeviceOptions = EdgeTpuManager::DeviceOptions;

extern "C" {

struct edgetpu_device* edgetpu_list_devices(size_t* num_devices) {
  CHECK(num_devices);

  auto records = EdgeTpuManager::GetSingleton()->EnumerateEdgeTpu();
  if (records.empty()) {
    *num_devices = 0;
    return nullptr;
  }

  const auto devs_size = sizeof(edgetpu_device) * records.size();

  size_t size = devs_size;
  for (const auto& record : records) size += record.path.size() + 1;

  char* memory = new char[size];
  edgetpu_device* devs = reinterpret_cast<edgetpu_device*>(memory);
  char* paths = memory + devs_size;

  int i = 0;
  for (const auto& record : records) {
    edgetpu_device* dev = &devs[i++];
    dev->type = static_cast<edgetpu_device_type>(record.type);
    dev->path = paths;

    const auto len = record.path.size() + 1;
    std::memcpy(paths, record.path.c_str(), len);
    paths += len;
  }

  *num_devices = records.size();
  return devs;
}

void edgetpu_free_devices(struct edgetpu_device* dev) {
  delete[] reinterpret_cast<char*>(dev);
}

TfLiteDelegate* edgetpu_create_delegate(enum edgetpu_device_type type,
                                        const char* name,
                                        const struct edgetpu_option* options,
                                        size_t num_options) {
  auto* manager = EdgeTpuManager::GetSingleton();
  const auto device_type = static_cast<DeviceType>(type);

  std::shared_ptr<EdgeTpuContext> context;
  if (num_options > 0) {
    CHECK(options);
    CHECK(name);
    EdgeTpuManager::DeviceOptions device_options;
    for (size_t i = 0; i < num_options; ++i) {
      const edgetpu_option* option = &options[i];
      device_options.insert({option->name, option->value});
    }
    context = manager->OpenDevice(device_type, name, device_options);
  } else {
    context = (name == nullptr) ? manager->OpenDevice(device_type)
                                : manager->OpenDevice(device_type, name);
  }

  if (!context) return nullptr;
  return platforms::darwinn::tflite::CreateEdgeTpuDelegateForCustomOp(context);
}

void edgetpu_free_delegate(TfLiteDelegate* delegate) {
  platforms::darwinn::tflite::FreeEdgeTpuDelegateForCustomOp(delegate);
}

void edgetpu_verbosity(int verbosity) {
  EdgeTpuManager::GetSingleton()->SetVerbosity(verbosity);
}

const char* edgetpu_version() {
  static auto* version =
      new std::string(EdgeTpuManager::GetSingleton()->Version());
  return version->c_str();
}

}  // extern "C"
