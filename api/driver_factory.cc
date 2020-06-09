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

#include "api/driver_factory.h"

#include "port/unreachable.h"

namespace platforms {
namespace darwinn {
namespace api {

namespace {

Driver* NewDriver(const Device& device, const Driver::Options& options) {
  // Build Driver.
  auto factory = DriverFactory::GetOrCreate();
  auto backing_driver = factory->CreateDriver(device, options).ValueOrDie();
  return backing_driver.release();
}

}  // namespace

Driver* DriverFactory::CreateDriverAsSingleton(const Device& device,
                                               const Driver::Options& options) {
  static api::Driver* driver = NewDriver(device, options);
  return driver;
}

Device::Type GetTypeByName(const std::string& device_type) {
  if (device_type == "PCI" || device_type == "pci") {
    return Device::Type::PCI;
  } else if (device_type == "USB" || device_type == "usb") {
    return Device::Type::USB;
  } else if (device_type == "PLATFORM" || device_type == "platform") {
    return Device::Type::PLATFORM;
  } else if (device_type == "REFERENCE" || device_type == "reference") {
    return Device::Type::REFERENCE;
  } else if (device_type == "SIMULATOR" || device_type == "simulator") {
    return Device::Type::SIMULATOR;
  } else if (device_type == "REMOTE_PCI" || device_type == "remote_pci") {
    return Device::Type::REMOTE_PCI;
  } else if (device_type == "REMOTE_USB" || device_type == "remote_usb") {
    return Device::Type::REMOTE_USB;
  } else if (device_type == "REMOTE_PLATFORM" ||
             device_type == "remote_platform") {
    return Device::Type::REMOTE_PLATFORM;
  } else {
    LOG(FATAL) << "Unknown device type: " << device_type
               << R"error(, which should be either "PCI", "USB", "PLATFORM", "REFERENCE", "REMOTE_PCI", "REMOTE_USB", "REMOTE_PLATFORM", or "SIMULATOR")error";
    unreachable();  // NOLINT
  }
}

std::string GetTypeName(Device::Type device_type) {
  switch (device_type) {
    case Device::Type::PCI:
      return "pci";
    case Device::Type::USB:
      return "usb";
    case Device::Type::PLATFORM:
      return "platform";
    case Device::Type::REMOTE_PCI:
      return "remote_pci";
    case Device::Type::REMOTE_USB:
      return "remote_usb";
    case Device::Type::REMOTE_PLATFORM:
      return "remote_platform";
    case Device::Type::REFERENCE:
      return "reference";
    case Device::Type::SIMULATOR:
      return "simulator";
    default:
      return "unknown";
  }
}

}  // namespace api
}  // namespace darwinn
}  // namespace platforms
