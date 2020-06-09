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

#ifndef DARWINN_DRIVER_DRIVER_FACTORY_H_
#define DARWINN_DRIVER_DRIVER_FACTORY_H_

#include <memory>
#include <mutex>  // NOLINT
#include <string>
#include <type_traits>
#include <vector>

#include "api/chip.h"
#include "api/driver.h"
#include "api/driver_factory.h"
#include "api/driver_options_generated.h"
#include "port/integral_types.h"
#include "port/statusor.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Interface for a class that can provide a Driver implementation.
//
// Once implemented, driver providers needs to be registered with the
// DriverFactory using the following function in driver_factory.h
//
// REGISTER_DRIVER_PROVIDER(..).
//
// The subclasses of DriverProvider must implement a static CreateDriverProvider
// function with following signature. The driver provider cannot be registered
// without this function.
//
// static std::unique_ptr<DriverProvider> CreateDriverProvider();
class DriverProvider {
 public:
  DriverProvider() = default;
  virtual ~DriverProvider() = default;

  // This class is neither copyable nor movable.
  DriverProvider(const DriverProvider&) = delete;
  DriverProvider& operator=(const DriverProvider&) = delete;

  // Enumerates all devices available through this provider.
  virtual std::vector<api::Device> Enumerate() = 0;

  // Returns true, if the factory can create driver for given device.
  virtual bool CanCreate(const api::Device& device) = 0;

  // Returns a driver instance that interfaces with specified device.
  // Custom options specified here would override default ones. The exact set of
  // possible key-value pairs is provider-specific.
  virtual util::StatusOr<std::unique_ptr<api::Driver>> CreateDriver(
      const api::Device& device, const api::DriverOptions& options) = 0;

 protected:
  // Helper function that looks for devices by iterating over directory entries
  // in /sys/class/<class_name>/<class_name>* and matching them against files
  // in /dev.
  std::vector<api::Device> EnumerateSysfs(const std::string& class_name,
                                          api::Chip chip,
                                          api::Device::Type type);
  // Same as above but specifying class name and device name separately:
  // /sys/class/<class_name>/<device_name>*
  std::vector<api::Device> EnumerateByClass(const std::string& class_name,
                                            const std::string& device_name,
                                            api::Chip chip,
                                            api::Device::Type type);
};

// Enumerates devices and creates drivers for those devices.
class DriverFactory : public api::DriverFactory {
 public:
  // Creates or returns the singleton instance of the driver factory.
  static DriverFactory* GetOrCreate();

  // This class is neither copyable nor movable.
  DriverFactory(const DriverFactory&) = delete;
  DriverFactory& operator=(const DriverFactory&) = delete;

  ~DriverFactory() = default;

  // Enumerates all available devices.
  std::vector<api::Device> Enumerate() override LOCKS_EXCLUDED(mutex_);

  // Creates a driver instance that interfaces to the specified device.
  util::StatusOr<std::unique_ptr<api::Driver>> CreateDriver(
      const api::Device& device) override LOCKS_EXCLUDED(mutex_);

  // Creates a driver instance that interfaces to the specified device with
  // custom options.
  util::StatusOr<std::unique_ptr<api::Driver>> CreateDriver(
      const api::Device& device, const api::Driver::Options& options) override
      LOCKS_EXCLUDED(mutex_);

  // Registers a new driver provider.
  void RegisterDriverProvider(std::unique_ptr<DriverProvider> provider)
      LOCKS_EXCLUDED(mutex_);

 private:
  // Constructor.
  DriverFactory() = default;

  // Container for all registered driver providers.
  std::vector<std::unique_ptr<DriverProvider>> providers_ GUARDED_BY(mutex_);

  // Maintains integrity of providers_.
  mutable std::mutex mutex_;
};

namespace internal {

// Functions for checking that the DriverProvider has the required
// CreateDriverProvider function.
template <class T>
constexpr bool DriverProviderHasCreateDriverProvider() {
  typedef std::unique_ptr<DriverProvider> (*CreateDriverProviderType)();
  return std::is_same<decltype(&T::CreateDriverProvider),
                      CreateDriverProviderType>::value;
}

// Provides access to the static functions within a specific subclass
// of DriverProvider.
template <typename DriverProviderSubclass>
class StaticAccessToDriverProvider {
 public:
  static_assert(std::is_base_of<::platforms::darwinn::driver::DriverProvider,
                                DriverProviderSubclass>::value,
                "Classes registered with REGISTER_DRIVER_PROVIDER must be "
                "subclasses of ::platforms::darwinn::driver::DriverProvider.");

  static_assert(
      DriverProviderHasCreateDriverProvider<DriverProviderSubclass>(),
      "CreateDriverProvider() must be defined with the correct signature "
      "in every DriverProvider.");

  // Provides access to the static function CreateDriverProvider within a
  // specific subclass of DriverProvider.
  static std::unique_ptr<DriverProvider> CreateDriverProvider() {
    // DriverProviderSubclass must implement this function, since it is not
    // implemented in the parent class.
    return DriverProviderSubclass::CreateDriverProvider();
  }
};

// Registrar that registers an instance of DriverProviderSubclass during
// construction.
template <typename DriverProviderSubclass>
class DriverProviderRegistrar {
 public:
  DriverProviderRegistrar() {
    auto provider = StaticAccessToDriverProvider<
        DriverProviderSubclass>::CreateDriverProvider();
    DriverFactory::GetOrCreate()->RegisterDriverProvider(std::move(provider));
  }
};

}  // namespace internal

// Macro for registering DriverProviders.
#define REGISTER_DRIVER_PROVIDER(name)                                         \
  static ::platforms::darwinn::driver::internal::DriverProviderRegistrar<name> \
      DriverProviderRegistrar##name

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_DRIVER_FACTORY_H_
