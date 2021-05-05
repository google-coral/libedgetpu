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

#ifndef DARWINN_DRIVER_PACKAGE_VERIFIER_H_
#define DARWINN_DRIVER_PACKAGE_VERIFIER_H_

#include <fstream>
#include <string>

#include "port/defs.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/openssl.h"
#include "port/status.h"
#include "port/statusor.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

// ExecutableVerifier is a class to verify executable packages using digital
// signatures.
class PackageVerifier {
 public:
  virtual ~PackageVerifier() = default;

  // Verifies the executable package provided its buffer.
  virtual Status VerifySignature(const void* package_buffer) const = 0;
};

// A noop implementation of ExecutableVerifier that errors out on all calls.
class NoopPackageVerifier : public PackageVerifier {
 public:
  NoopPackageVerifier() = default;
  ~NoopPackageVerifier() override = default;
  Status VerifySignature(const void* package_buffer) const override;
};

// Makes an ExecutableVerifier provided a public key. If the key is empty a noop
// verifier will be returned that errors on Verify.
StatusOr<std::unique_ptr<PackageVerifier>> MakeExecutableVerifier(
    const std::string& public_key);

// Makes an ExecutableVerifier provided a file path to the public key. If the
// path is empty a noop verifier will be returned that errors on Verify.
StatusOr<std::unique_ptr<PackageVerifier>> MakeExecutableVerifierFromFile(
    const std::string& public_key_path);

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_PACKAGE_VERIFIER_H_
