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

#include "port/logging.h"

namespace platforms {
namespace darwinn {
namespace driver {

std::vector<api::Device> DriverProvider::EnumerateByClass(
    const std::string& class_name, const std::string& device_name,
    api::Chip chip, api::Device::Type type) {
  std::vector<api::Device> device_list;
  LOG(FATAL) << "EnumerateByClass is not supported on macOS at this time.";
  return device_list;
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
