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

#include "driver/package_verifier.h"

#include <string>

#include "executable/executable_generated.h"
#include "port/ptr_util.h"

namespace platforms {
namespace darwinn {
namespace driver {

Status NoopPackageVerifier::VerifySignature(const void*) const {
  return FailedPreconditionError(
      "No verifier was created yet verification was requested.");
}

StatusOr<std::unique_ptr<PackageVerifier>> MakeExecutableVerifier(
    const std::string& public_key_path) {

  return {gtl::MakeUnique<NoopPackageVerifier>()};
}

StatusOr<std::unique_ptr<PackageVerifier>> MakeExecutableVerifierFromFile(
    const std::string& public_key_path) {

  return {gtl::MakeUnique<NoopPackageVerifier>()};
}
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
