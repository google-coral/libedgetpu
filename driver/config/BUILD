# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Description:
#   Chip specific configuration and CSR Layouts.
#
# When compiled in google3 environment, gen_rule generated headers in
# platforms/... is used directly.
# When compiled in non-google3 environment, pregenerated headers in
# ${PROJECT}/...  is used.

load("//:libedgetpu_cc_rules.bzl", "libedgetpu_cc_library")

DEFAULT_VISIBILITY = [
    "//:internal",
]

package(default_visibility = ["//visibility:public"])

# All Google Owned Code except :
# - certain files in port/default/ that are under Apache 2.0 license.
licenses(["notice"])

# Configuration structures.
libedgetpu_cc_library(
    name = "config",
    hdrs = [
        "apex_csr_offsets.h",
        "beagle_csr_helper.h",
        "breakpoint_csr_offsets.h",
        "cb_bridge_csr_offsets.h",
        "chip_config.h",
        "chip_structures.h",
        "common_csr_helper.h",
        "debug_hib_user_csr_offsets.h",
        "debug_scalar_core_csr_offsets.h",
        "debug_tile_csr_offsets.h",
        "hib_kernel_csr_offsets.h",
        "hib_user_csr_offsets.h",
        "interrupt_csr_offsets.h",
        "lpm_csr_offsets.h",
        "memory_csr_offsets.h",
        "misc_csr_offsets.h",
        "msix_csr_offsets.h",
        "power_throttle_csr_helper.h",
        "queue_csr_offsets.h",
        "register_file_csr_offsets.h",
        "scalar_core_csr_offsets.h",
        "scu_csr_offsets.h",
        "sync_flag_csr_offsets.h",
        "tile_config_csr_offsets.h",
        "tile_csr_offsets.h",
        "tile_thread_csr_offsets.h",
        "tile_thread_trace_csr_offsets.h",
        "trace_csr_offsets.h",
        "usb_csr_offsets.h",
        "wire_csr_offsets.h",
    ],
    deps = [
        "//api:chip",
        "//driver:util",
        "//port:integral_types",
        "//port:logging",
        "//port:unreachable",
    ],
)

libedgetpu_cc_library(
    name = "register_constants",
    hdrs = ["register_constants.h"],
    deps = [
        "//port:integral_types",
    ],
)

libedgetpu_cc_library(
    name = "scalar_core_csr_offsets_helper",
    hdrs = ["scalar_core_csr_offsets_helper.h"],
    deps = [
        "//base",
        "//driver:util",
        "//util/bits",
    ],
)
