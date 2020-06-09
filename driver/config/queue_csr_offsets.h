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

#ifndef DARWINN_DRIVER_CONFIG_QUEUE_CSR_OFFSETS_H_
#define DARWINN_DRIVER_CONFIG_QUEUE_CSR_OFFSETS_H_

#include <stddef.h>

#include "port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds the various CSR offsets for programming queue behaviors.
// Members are intentionally named to match the GCSR register names.
struct QueueCsrOffsets {
  uint64 queue_control;
  uint64 queue_status;
  uint64 queue_descriptor_size;
  uint64 queue_base;
  uint64 queue_status_block_base;
  uint64 queue_size;
  uint64 queue_tail;
  uint64 queue_fetched_head;
  uint64 queue_completed_head;
  uint64 queue_int_control;
  uint64 queue_int_status;
  uint64 queue_minimum_size;
  uint64 queue_maximum_size;
  uint64 queue_int_vector;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_CONFIG_QUEUE_CSR_OFFSETS_H_
