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

// AUTO GENERATED FILE.
// See http://go/darwinn-chip-structure for more info.

#ifndef DARWINN_DRIVER_CONFIG_BEAGLE_BEAGLE_CHIP_STRUCTURES_H_
#define DARWINN_DRIVER_CONFIG_BEAGLE_BEAGLE_CHIP_STRUCTURES_H_

#include "driver/config/chip_structures.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

const ChipStructures kBeagleChipStructures = {
    8ULL,        // NOLINT: minimum_alignment_bytes
    4096ULL,     // NOLINT: allocation_alignment_bytes
    0ULL,        // NOLINT: axi_dma_burst_limiter
    0ULL,        // NOLINT: num_wire_interrupts
    8192ULL,     // NOLINT: num_page_table_entries
    64ULL,       // NOLINT: physical_address_bits
    0ULL,        // NOLINT: tpu_dram_size_bytes
    196608ULL,   // NOLINT: narrow_memory_capacity
    262144ULL,   // NOLINT: external_narrow_memory_translate_entry_size_bytes
    4ULL,        // NOLINT: number_x_tiles
    4ULL,        // NOLINT: number_y_tiles
    1ULL,        // NOLINT: number_compute_threads
    0ULL,        // NOLINT: number_of_ring_virtual_networks
    0ULL,        // NOLINT: last_z_out_cell_disable_incompatible_with_sparsity
    0ULL,        // NOLINT: nlu_buffer_backpressure_causes_assertion
    0ULL,        // NOLINT: mesh_rx_queue_depth
    0ULL,        // NOLINT: default_vn_buffer_memory_lines
    6291456ULL,  // NOLINT: csr_region_base_offset
    2097152ULL,  // NOLINT: csr_region_size_bytes
    0ULL,        // NOLINT: support_trace_arch_registers
    0ULL,        // NOLINT: base_and_bound_unit_size_bytes
    1ULL,        // NOLINT: number_of_scalar_core_contexts
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_CONFIG_BEAGLE_BEAGLE_CHIP_STRUCTURES_H_
