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

#ifndef DARWINN_API_DRIVER_FACTORY_H_
#define DARWINN_API_DRIVER_FACTORY_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "api/chip.h"
#include "api/driver.h"
#include "api/driver_options_generated.h"
#include "port/integral_types.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {
namespace api {

// A type for uniquely identifying a DarwiNN device.
struct Device {
  // Device type.
  enum class Type {
    // PCI device.
    // Path format: "/dev/<name>"
    // Example: /dev/apex_0
    PCI = 0,

    // USB device.
    // Path format: "/sys/bus/usb/devices/<bus>-<port>"
    // Example: /sys/bus/usb/devices/3-5.1.2.1.2
    USB = 1,

    // Platform (integrated) device.
    // Path format: "/dev/<name>"
    PLATFORM = 2,

    // Remote PCI device (for testing.)
    // Path format: "<ip address>:<port>"
    REMOTE_PCI = 10,

    // Remote USB device (for testing.)
    // Path format: "<ip address>:<port>"
    REMOTE_USB = 11,

    // Remote Platform device (for testing.)
    // Path format: "<ip address>:<port>"
    REMOTE_PLATFORM = 12,

    // Reference driver (for testing.)
    REFERENCE = 30,

    // Simulator driver (for testing.)  Path is ignored,
    // Chip determines which simulator is instantiated.
    SIMULATOR = 31,
  };

  // TODO: Replace map of strings with something better.
  using Attributes = std::unordered_map<std::string, std::string>;

  // Chip
  Chip chip;

  // Device type.
  Type type;

  // String that uniquely identifies the device.
  // Set this to DriverFactory::kDefaultDevicePath for default device picked by
  // the factory.
  std::string path;

  // Device attributes discovered through enumeration.
  // The exact set of possible key-value pairs is provider-specific.
  Attributes attributes;
};

// Returns correct Device::Type for given |device_type|.
Device::Type GetTypeByName(const std::string& device_type);

// Returns the name of the given |device_type|.
std::string GetTypeName(Device::Type device_type);

// Enumerates devices and creates drivers for those devices.
class DriverFactory {
 public:
  static constexpr const char* kDefaultDevicePath = "default";

  // Creates a singleton driver.
  static Driver* CreateDriverAsSingleton(const Device& device,
                                         const Driver::Options& options);

  // Creates or returns the singleton instance of the driver factory.
  static DriverFactory* GetOrCreate();

  virtual ~DriverFactory() = default;

  // Enumerates all available devices.
  virtual std::vector<Device> Enumerate() = 0;

  // Creates a driver instance that interfaces to the specified device.
  virtual util::StatusOr<std::unique_ptr<Driver>> CreateDriver(
      const Device& device) = 0;

  // Creates a driver instance that interfaces to the specified device with
  // custom options.
  virtual util::StatusOr<std::unique_ptr<Driver>> CreateDriver(
      const Device& device, const Driver::Options& options) = 0;

 protected:
  // Constructor.
  DriverFactory() = default;
};

}  // namespace api
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_API_DRIVER_FACTORY_H_
