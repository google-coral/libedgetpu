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

#include "driver/driver_factory.h"

#include <memory>
#include <utility>

#include "api/driver_options_generated.h"
#include "api/driver_options_helper.h"
#include "driver/config/chip_config.h"
#include "driver/driver.h"
#include "port/defs.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/ptr_util.h"
#include "port/statusor.h"
#include "port/std_mutex_lock.h"

namespace platforms {
namespace darwinn {
namespace driver {

std::vector<api::Device> DriverProvider::EnumerateSysfs(
    const std::string& class_name, api::Chip chip, api::Device::Type type) {
  // Look through sysfs for devices of the given class.
  // Sysfs paths look like this: /sys/class/<class_name>/<class_name>_<n>
  // For example, the first beagle device is: /sys/class/apex/apex_0
  // The corresponding device file is assumed to be: /dev/<class_name>_<n>
  // For example, if /sys/class/apex/apex_0 exists, we look for /dev/apex_0.
  // This convention is used for DarwiNN 1.0 devices.
  return EnumerateByClass(class_name, class_name, chip, type);
}

std::vector<api::Device> DriverFactory::Enumerate() {
  StdMutexLock lock(&mutex_);

  std::vector<api::Device> device_list;
  for (auto& provider : providers_) {
    auto provider_supported_devices = provider->Enumerate();
    for (auto& device : provider_supported_devices) {
      device_list.push_back(device);
    }
  }

  return device_list;
}

StatusOr<std::unique_ptr<api::Driver>> DriverFactory::CreateDriver(
    const api::Device& device) {
  return CreateDriver(device, api::DriverOptionsHelper::Defaults());
}

StatusOr<std::unique_ptr<api::Driver>> DriverFactory::CreateDriver(
    const api::Device& device, const api::Driver::Options& opaque_options) {
  StdMutexLock lock(&mutex_);

  // Deserialize options.
  const api::DriverOptions* options =
      api::GetDriverOptions(opaque_options.data());
  if (options == nullptr) {
    return InvalidArgumentError("Invalid Driver::Options instance.");
  }

  if (options->version() != Driver::kOptionsVersion) {
    return InvalidArgumentError("Invalid Driver::Options version.");
  }

  // Update verbosity level.
  // TODO: Verbosity level should be of per driver instance.
#if !DARWINN_PORT_USE_GOOGLE3
  if (options->verbosity() >= 0) {
    ::platforms::darwinn::internal::SetLoggingLevel(options->verbosity());
  }
#endif  // !DARWINN_PORT_USE_GOOGLE3

  for (auto& provider : providers_) {
    // Skip if the provider cannot create driver for this device spec.
    if (!provider->CanCreate(device)) {
      continue;
    }

    // Always invoke only the first provider which claims the ability.
    if (device.path != kDefaultDevicePath) {
      return provider->CreateDriver(device, *options);
    }

    // Try to enumerate with this provider.
    std::vector<api::Device> device_list = provider->Enumerate();
    // Skip this provider if there is no any device.
    if (device_list.empty()) {
      continue;
    }

    // Create driver associated with the device in the resulting list.
    for (const auto& provider_device : device_list) {
      if (device.chip == provider_device.chip &&
          device.type == provider_device.type) {
        return provider->CreateDriver(provider_device, *options);
      }
    }
  }

  return NotFoundError("Unable to construct driver for device.");
}

void DriverFactory::RegisterDriverProvider(
    std::unique_ptr<DriverProvider> provider) {
  StdMutexLock lock(&mutex_);
  providers_.push_back(std::move(provider));
}

DriverFactory* DriverFactory::GetOrCreate() {
  static std::unique_ptr<DriverFactory> singleton =
      gtl::WrapUnique<DriverFactory>(new driver::DriverFactory());
  return singleton.get();
}

}  // namespace driver

namespace api {

DriverFactory* DriverFactory::GetOrCreate() {
  return driver::DriverFactory::GetOrCreate();
}

}  // namespace api
}  // namespace darwinn
}  // namespace platforms
