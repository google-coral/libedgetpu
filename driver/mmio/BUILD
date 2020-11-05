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
#  Memory Mapped IO (ex: PCIe.)

load("//:libedgetpu_cc_rules.bzl", "libedgetpu_cc_library")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

libedgetpu_cc_library(
    name = "coherent_allocator",
    srcs = ["coherent_allocator.cc"],
    hdrs = ["coherent_allocator.h"],
    deps = [
        "//api:buffer",
        "//port",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
    ],
)

libedgetpu_cc_library(
    name = "host_queue",
    hdrs = ["host_queue.h"],
    deps = [
        ":coherent_allocator",
        "//api:buffer",
        "//driver:device_buffer",
        "//driver:hardware_structures",
        "//driver/config",
        "//driver/memory:address_space",
        "//driver/memory:dma_direction",
        "//driver/registers",
        "//port",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
        "//port:tracing",
    ],
)
