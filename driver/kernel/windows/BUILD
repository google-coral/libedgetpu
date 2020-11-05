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

load("//:libedgetpu_cc_rules.bzl", "libedgetpu_cc_library")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

libedgetpu_cc_library(
    name = "kernel_event_windows",
    srcs = ["kernel_event_windows.cc"],
    hdrs = ["kernel_event_windows.h"],
    tags = [
        "manual",
        "nobuilder",
        "notap",
    ],
    deps = [
        ":gasket_ioctl_windows",
        "//driver/kernel:gasket_ioctl",
        "//driver/kernel:kernel_event",
        "//port",
        "//port:fileio",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
        "//port:tracing",
    ],
)

libedgetpu_cc_library(
    name = "kernel_event_handler_windows",
    srcs = ["kernel_event_handler_windows.cc"],
    hdrs = ["kernel_event_handler_windows.h"],
    tags = [
        "manual",
        "nobuilder",
        "notap",
    ],
    deps = [
        ":gasket_ioctl_windows",
        "//driver/kernel:gasket_ioctl",
        "//driver/kernel:kernel_event",
        "//driver/kernel:kernel_event_handler",
        "//driver/kernel/windows:kernel_event_windows",
        "//port",
        "//port:fileio",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
        "//port:tracing",
    ],
)

libedgetpu_cc_library(
    name = "kernel_coherent_allocator_windows",
    srcs = ["kernel_coherent_allocator_windows.cc"],
    hdrs = ["kernel_coherent_allocator_windows.h"],
    tags = [
        "manual",
        "nobuilder",
        "notap",
    ],
    deps = [
        ":gasket_ioctl_windows",
        "//driver:hardware_structures",
        "//driver/kernel:gasket_ioctl",
        "//driver/kernel:kernel_coherent_allocator",
        "//driver/mmio:coherent_allocator",
        "//port",
        "//port:fileio",
    ],
)

libedgetpu_cc_library(
    name = "kernel_registers_windows",
    srcs = ["kernel_registers_windows.cc"],
    hdrs = ["kernel_registers_windows.h"],
    tags = [
        "manual",
        "nobuilder",
        "notap",
    ],
    deps = [
        ":gasket_ioctl_windows",
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

libedgetpu_cc_library(
    name = "gasket_ioctl_windows",
    hdrs = ["windows_gasket_ioctl.inc"],
)
