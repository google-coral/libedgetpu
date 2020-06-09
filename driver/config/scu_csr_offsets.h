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

#ifndef DARWINN_DRIVER_CONFIG_SCU_CSR_OFFSETS_H_
#define DARWINN_DRIVER_CONFIG_SCU_CSR_OFFSETS_H_

#include "port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for programming the SCU in Beagle
// (the block containing state machines to control boot and power sequences)
// Members are intentionally named to match the GCSR register names.
struct ScuCsrOffsets {
  // The SCU control registers have generic names but each contain many small
  // fields which are reflected in the spec (should use csr_helper to access)
  uint64 scu_ctrl_0;
  uint64 scu_ctrl_1;
  uint64 scu_ctrl_2;
  uint64 scu_ctrl_3;
  uint64 scu_ctrl_4;
  uint64 scu_ctrl_5;
  uint64 scu_ctr_6;
  uint64 scu_ctr_7;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_CONFIG_SCU_CSR_OFFSETS_H_
