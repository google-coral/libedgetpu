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

#ifndef DARWINN_API_DRIVER_OPTIONS_HELPER_H_
#define DARWINN_API_DRIVER_OPTIONS_HELPER_H_

#include "api/driver.h"

namespace platforms {
namespace darwinn {
namespace api {

// A simpler wrapper around several static helper functions.
class DriverOptionsHelper {
 public:
  // Returns driver options for default.
  static Driver::Options Defaults();

  // Returns driver options for maximum performance.
  static Driver::Options MaxPerformance();
};

}  // namespace api
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_API_DRIVER_OPTIONS_HELPER_H_
