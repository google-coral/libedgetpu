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

#ifndef DARWINN_DRIVER_CONFIG_HIB_USER_CSR_OFFSETS_H_
#define DARWINN_DRIVER_CONFIG_HIB_USER_CSR_OFFSETS_H_

#include "port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for user space in HIB. Members are
// intentionally named to match the GCSR register names.
struct HibUserCsrOffsets {
  // Interrupt control and status for top level.
  uint64 top_level_int_control;
  uint64 top_level_int_status;

  // Interrupt count for scalar core.
  uint64 sc_host_int_count;

  // DMA pauses.
  uint64 dma_pause;
  uint64 dma_paused;

  // Enable/disable status block update.
  uint64 status_block_update;

  // HIB errors.
  uint64 hib_error_status;
  uint64 hib_error_mask;
  uint64 hib_first_error_status;
  uint64 hib_first_error_timestamp;
  uint64 hib_inject_error;

  // Limits AXI DMA burst.
  uint64 dma_burst_limiter;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_CONFIG_HIB_USER_CSR_OFFSETS_H_
