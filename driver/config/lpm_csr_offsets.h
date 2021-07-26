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

#ifndef DARWINN_DRIVER_CONFIG_LPM_CSR_OFFSETS_H_
#define DARWINN_DRIVER_CONFIG_LPM_CSR_OFFSETS_H_

#include "port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for lpm programming. Members are
// intentionally named to match the GCSR register names.
struct LpmCsrOffsets {
  uint64 psm_0_dmem_cfg;
  uint64 psm_0_dmem_start;
  uint64 psm_0_dmem_status;
  uint64 psm_0_state_table_3_trans_3_enable_state;
  uint64 psm_1_dmem_data_28;
  uint64 psm_1_dmem_data_29;
  uint64 psm_1_dmem_data_30;
  uint64 psm_1_dmem_cfg;
  uint64 psm_1_dmem_start;
  uint64 psm_1_dmem_status;
  uint64 psm_1_state_table_3_trans_3_enable_state;
  uint64 psm_2_dmem_cfg;
  uint64 psm_2_dmem_start;
  uint64 psm_2_dmem_status;
  uint64 psm_2_state_table_3_trans_3_enable_state;
  uint64 psm_3_dmem_cfg;
  uint64 psm_3_dmem_start;
  uint64 psm_3_dmem_status;
  uint64 psm_3_state_table_3_trans_3_enable_state;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_CONFIG_LPM_CSR_OFFSETS_H_
