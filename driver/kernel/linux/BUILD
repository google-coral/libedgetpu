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
# Kernel driver specific functionality.

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

cc_library(
    name = "kernel_event_linux",
    srcs = ["kernel_event_linux.cc"],
    hdrs = ["kernel_event_linux.h"],
    deps = [
        "//driver/kernel:kernel_event",
        "//port",
        "//port:fileio",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
        "//port:tracing",
    ],
)

cc_library(
    name = "kernel_event_handler_linux",
    srcs = ["kernel_event_handler_linux.cc"],
    hdrs = ["kernel_event_handler_linux.h"],
    deps = [
        "//driver/kernel:gasket_ioctl",
        "//driver/kernel:kernel_event",
        "//driver/kernel:kernel_event_handler",
        "//driver/kernel/linux:kernel_event_linux",
        "//port",
        "//port:fileio",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
        "//port:tracing",
    ],
)

cc_library(
    name = "kernel_coherent_allocator_linux",
    srcs = ["kernel_coherent_allocator_linux.cc"],
    hdrs = ["kernel_coherent_allocator_linux.h"],
    deps = [
        "//driver:hardware_structures",
        "//driver/kernel:gasket_ioctl",
        "//driver/kernel:kernel_coherent_allocator",
        "//driver/mmio:coherent_allocator",
        "//port",
        "//port:fileio",
    ],
)

cc_library(
    name = "kernel_registers_linux",
    srcs = ["kernel_registers_linux.cc"],
    hdrs = ["kernel_registers_linux.h"],
    deps = [
        "@com_google_absl//absl/strings:str_format",
        "//driver/kernel:gasket_ioctl",
        "//driver/kernel:kernel_registers",
        "//driver/registers",
        "//port",
        "//port:fileio",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
    ],
)
