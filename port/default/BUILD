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
#  Port of various google3 libraries and utilities.

package(default_visibility = ["//visibility:public"])

# All Google Owned Code except :
# - certain files in port/default/ that are under Apache 2.0 license.
licenses(["notice"])

config_setting(
    name = "windows",
    values = {
        "cpu": "x64_windows",
    },
)

#  Independent headers that can be included by port_from_tf.
cc_library(
    name = "port_base",
    hdrs = [
        "error_codes.h",
        "strcat.h",
        "unreachable.h",
    ] + select({
        "//:windows": ["unreachable_windows.h"],
        "//conditions:default": ["unreachable_default.h"],
    }),
)

#  Port of various google3 libraries and utilities.
cc_library(
    name = "port",
    srcs = [
        "status_macros.cc",
        "stringprintf.cc",
    ],
    hdrs = [
        "aligned_malloc.h",
        "array_slice.h",
        "builddata.h",
        "casts.h",
        "error_codes.h",
        "errors.h",
        "integral_types.h",
        "logging.h",
        "macros.h",
        "mutex.h",
        "math_util.h",
        "ptr_util.h",
        "status.h",
        "status_macros.h",
        "statusor.h",
        "strcat.h",
        "stringprintf.h",
    ] + select({
        "//:windows": ["aligned_malloc_windows.h"],
        "//conditions:default": ["aligned_malloc_default.h"],
    }),
    linkstamp = select({
        ":windows": None,  # Linkstamp doesn't work on Windows, https://github.com/bazelbuild/bazel/issues/6997
        "//conditions:default": "builddata.cc",
    }),
    deps = [
        ":thread_annotations",
        "//port/default/port_from_tf",
    ],
)

cc_library(
    name = "thread_annotations",
    hdrs = [
        "thread_annotations.h",
    ],
    deps = [
        "//port/default/port_from_tf:thread_annotations",
    ],
)

cc_library(
    name = "semaphore",
    hdrs = [
        "semaphore.h",
    ],
    deps = [
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "//thread/fiber",
        "//thread/fiber/semaphore:fifo_semaphore",
    ],
)

cc_library(
    name = "strcat",
    hdrs = [
        "strcat.h",
    ],
)

cc_library(
    name = "unreachable",
    hdrs = [
        "unreachable.h",
    ] + select({
        "//:windows": ["unreachable_windows.h"],
        "//conditions:default": ["unreachable_default.h"],
    }),
)

cc_library(
    name = "cleanup",
    hdrs = ["cleanup.h"],
    deps = [
        ":port",
    ],
)

cc_library(
    name = "cpu",
    hdrs = ["cpu.h"],
)

cc_library(
    name = "memory_barriers",
    hdrs = ["memory_barriers.h"],
)
