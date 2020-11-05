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

load("//:libedgetpu_cc_rules.bzl", "libedgetpu_cc_library")

package(default_visibility = ["//visibility:public"])

# Files are originally ported from TensorFlow.
licenses(["notice"])

exports_files(["logging.h"])

# Test helpers
libedgetpu_cc_library(
    name = "status_test_util",
    testonly = 1,
    hdrs = [
        "status_test_util.h",
    ],
    deps = [
        ":port_from_tf",
        "//port/default:port",
    ],
)

#  Port of various google3 libraries and utilities.
libedgetpu_cc_library(
    name = "port_from_tf",
    srcs = [
        "logging.cc",
        "status.cc",
        "statusor.cc",
    ],
    hdrs = [
        "array_slice.h",
        "array_slice_internal.h",
        "casts.h",
        "errors.h",
        "logging.h",
        "macros.h",
        "math_util.h",
        "ptr_util.h",
        "status.h",
        "statusor.h",
        "statusor_internals.h",
    ],
    deps = [
        "//port/default:port_base",
    ],
)

libedgetpu_cc_library(
    name = "thread_annotations",
    hdrs = [
        "thread_annotations.h",
    ],
)
