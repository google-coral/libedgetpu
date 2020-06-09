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

#ifndef DARWINN_DRIVER_CONFIG_TILE_THREAD_TRACE_CSR_OFFSETS_H_
#define DARWINN_DRIVER_CONFIG_TILE_THREAD_TRACE_CSR_OFFSETS_H_

#include "port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for tilethreads.
// Members are intentionally named to match the GCSR register names.
struct TileThreadTraceCsrOffsets {
  // RunControls to change run state.
  uint64 TimeStampUnit;
  uint64 StallCauseSelect;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_CONFIG_TILE_THREAD_TRACE_CSR_OFFSETS_H_
