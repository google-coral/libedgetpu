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
#  Memory management related functionality.

load("//:libedgetpu_cc_rules.bzl", "libedgetpu_cc_library")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

libedgetpu_cc_library(
    name = "address_utilities",
    hdrs = ["address_utilities.h"],
    deps = [
        "//driver:hardware_structures",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "mmu_mapper",
    srcs = ["mmu_mapper.cc"],
    hdrs = ["mmu_mapper.h"],
    deps = [
        ":address_utilities",
        "//api:buffer",
        "//driver:device_buffer",
        "//driver/memory:dma_direction",
        "//port",
        "//port:tracing",
    ],
)

libedgetpu_cc_library(
    name = "address_space",
    hdrs = ["address_space.h"],
    deps = [
        ":dma_direction",
        "//api:buffer",
        "//driver:device_buffer",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "mmio_address_space",
    srcs = ["mmio_address_space.cc"],
    hdrs = ["mmio_address_space.h"],
    deps = [
        ":address_space",
        ":address_utilities",
        ":dma_direction",
        ":mmu_mapper",
        "//api:buffer",
        "//port",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
        "//port:tracing",
    ],
)

libedgetpu_cc_library(
    name = "buddy_address_space",
    srcs = ["buddy_address_space.cc"],
    hdrs = ["buddy_address_space.h"],
    deps = [
        ":address_utilities",
        ":buddy_allocator",
        ":dma_direction",
        ":mmio_address_space",
        ":mmu_mapper",
        "//api:buffer",
        "//port",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
        "//port:tracing",
    ],
)

libedgetpu_cc_library(
    name = "address_space_allocator",
    hdrs = ["address_space_allocator.h"],
    deps = ["//port"],
)

libedgetpu_cc_library(
    name = "buddy_allocator",
    srcs = ["buddy_allocator.cc"],
    hdrs = ["buddy_allocator.h"],
    deps = [
        ":address_space_allocator",
        ":address_utilities",
        "@com_google_absl//absl/strings:str_format",
        "//port",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
    ],
)

libedgetpu_cc_library(
    name = "fake_mmu_mapper",
    srcs = ["fake_mmu_mapper.cc"],
    hdrs = ["fake_mmu_mapper.h"],
    deps = [
        ":address_utilities",
        ":dma_direction",
        ":mmu_mapper",
        "//driver:hardware_structures",
        "//port",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
    ],
)

libedgetpu_cc_library(
    name = "dma_direction",
    hdrs = ["dma_direction.h"],
    deps = [
    ],
)

libedgetpu_cc_library(
    name = "nop_address_space",
    srcs = ["nop_address_space.cc"],
    hdrs = ["nop_address_space.h"],
    deps = [
        ":address_space",
        ":dma_direction",
        "//api:buffer",
        "//driver:device_buffer",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "dual_address_space",
    srcs = ["dual_address_space.cc"],
    hdrs = ["dual_address_space.h"],
    deps = [
        ":address_space",
        ":buddy_address_space",
        ":mmu_mapper",
        "//driver:hardware_structures",
        "//driver/config",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "dram_allocator",
    hdrs = ["dram_allocator.h"],
    deps = [
        "//api:dram_buffer",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "null_dram_allocator",
    hdrs = ["null_dram_allocator.h"],
    deps = [
        ":dram_allocator",
        "//api:dram_buffer",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "fake_dram_allocator",
    srcs = ["fake_dram_allocator.cc"],
    hdrs = ["fake_dram_allocator.h"],
    deps = [
        ":dram_allocator",
        "//api:dram_buffer",
        "//port",
    ],
)
