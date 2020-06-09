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

#ifndef DARWINN_API_CHIP_H_
#define DARWINN_API_CHIP_H_

#include <string.h>

#include <string>

namespace platforms {
namespace darwinn {
namespace api {

// Target chip for runtime stack.
enum class Chip {
  kBeagle,
  kUnknown,
};

static const struct {
  Chip chip;
  const char* names[2];
} kChipNames[] = {
    {Chip::kBeagle, {"beagle", "beagle_fpga"}},
};

// Returns correct Chip for given |chip_name|.
static inline Chip GetChipByName(const char* chip_name) {
  for (auto& pair : kChipNames)
    for (auto name : pair.names)
      if (name != nullptr && strcmp(chip_name, name) == 0) return pair.chip;
  return Chip::kUnknown;
}

// Returns correct Chip for given |chip_name|.
static inline Chip GetChipByName(const std::string& chip_name) {
  return GetChipByName(chip_name.c_str());
}

// Returns the name of the given |chip|.
static inline std::string GetChipName(Chip chip) {
  for (auto& pair : kChipNames)
    if (pair.chip == chip) return pair.names[0];
  return "unknown";
}

}  // namespace api
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_API_CHIP_H_
