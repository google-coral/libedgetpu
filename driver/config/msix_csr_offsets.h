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

#ifndef DARWINN_DRIVER_CONFIG_MSIX_CSR_OFFSETS_H_
#define DARWINN_DRIVER_CONFIG_MSIX_CSR_OFFSETS_H_

#include "port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for programming MSIX interrupts.
// Members are intentionally named to match the GCSR register names.
struct MsixCsrOffsets {
  // Interrupt vectors for descriptor queues.
  uint64 instruction_queue_int_vector;
  uint64 input_actv_queue_int_vector;
  uint64 param_queue_int_vector;
  uint64 output_actv_queue_int_vector;

  // top_level_int_vector contains indices for four interrupts.
  // [27:21] | [20:14] | [13:7] | [6:0]
  // thermal | mbist   | PCIe   | thermalShutdown
  uint64 top_level_int_vector;

  // sc_host_int_vector contains indices for four interrupts.
  // [27:21] | [20:14] | [13:7] | [6:0]
  // INT_3   | INT_2   | INT_1  | INT_0
  uint64 sc_host_int_vector;

  // HIB fatal error.
  uint64 fatal_err_int_vector;

  // Points to the first bit array. Subsequent bit arrays can be accessed with
  // increasing offsets if they exist.
  uint64 msix_pending_bit_array0;

  // Points to the first entry in the table. Subsequent entries can be accessed
  // with increasing offsets if they exist.
  uint64 msix_table;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_CONFIG_MSIX_CSR_OFFSETS_H_
