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

#ifndef DARWINN_API_RUNTIME_VERSION_H_
#define DARWINN_API_RUNTIME_VERSION_H_

namespace platforms {
namespace darwinn {
namespace api {

enum RuntimeVersion {
  // A DariwiNN package carrying runtime version less than or equal to this
  // value would trigger a warning from driver, during model registration.
  // This is the lower bound of kCurrent, and shall not be modified.
  kMinValidRuntimeVersion = 10,

  // Increase this number everytime a change involving both compiler and runtime
  // happens. This number is used in binary compatibility checks for DarwiNN
  // packages.
  kCurrent = 14,
  // This is the runtime version that has native batch support.
  kWithNativeBatchSupport = 11,
  // This is the runtime version that has support for int8 as host data type.
  kWithInt8HostDataTypeSupport = 12,
  // This is the runtime version that has support for 16-bit floating point.
  kWithFp16Support = 13,
  // Coral release codename Grouper.
  // Support for variable tensors and possibly other changes.
  kCoralGrouperRelease = 14,
};

}  // namespace api
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_API_RUNTIME_VERSION_H_
