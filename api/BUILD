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

load("//:libedgetpu_cc_rules.bzl", "libedgetpu_cc_library")

# Description:
#   Darwinn API headers
load(
    "@flatbuffers//:build_defs.bzl",
    "flatbuffer_cc_library",
)

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

libedgetpu_cc_library(
    name = "chip",
    hdrs = ["chip.h"],
)

libedgetpu_cc_library(
    name = "tensor_util",
    srcs = ["tensor_util.cc"],
    hdrs = ["tensor_util.h"],
    deps = [
        "//executable:executable_fbs",
        "//port",
        "//port:string_util",
    ],
)

libedgetpu_cc_library(
    name = "layer_information",
    srcs = ["layer_information.cc"],
    hdrs = ["layer_information.h"],
    deps = [
        ":buffer",
        ":tensor_util",
        "//executable:executable_fbs",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "request",
    hdrs = ["request.h"],
    deps = [
        ":buffer",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "driver",
    hdrs = ["driver.h"],
    deps = [
        ":buffer",
        ":package_reference",
        ":request",
        ":timing",
        "//api:telemeter_interface",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "driver_options_helper",
    srcs = ["driver_options_helper.cc"],
    hdrs = ["driver_options_helper.h"],
    deps = [
        ":driver",
        ":driver_options_fbs",
    ],
)

libedgetpu_cc_library(
    name = "allocated_buffer",
    srcs = ["allocated_buffer.cc"],
    hdrs = ["allocated_buffer.h"],
    deps = ["//port"],
)

libedgetpu_cc_library(
    name = "dram_buffer",
    hdrs = ["dram_buffer.h"],
    deps = ["//port"],
)

libedgetpu_cc_library(
    name = "buffer",
    srcs = ["buffer.cc"],
    hdrs = ["buffer.h"],
    deps = [
        ":allocated_buffer",
        ":dram_buffer",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "driver_factory",
    srcs = ["driver_factory.cc"],
    hdrs = ["driver_factory.h"],
    deps = [
        ":chip",
        ":driver",
        ":driver_options_fbs",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "package_reference",
    hdrs = ["package_reference.h"],
    deps = [
        ":execution_context_interface",
        ":layer_information",
        "//executable:executable_fbs",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "runtime_version",
    hdrs = ["runtime_version.h"],
)

flatbuffer_cc_library(
    name = "driver_options_fbs",
    srcs = ["driver_options.fbs"],
    flatc_args = [""],
)

libedgetpu_cc_library(
    name = "timing",
    hdrs = ["timing.h"],
    deps = [
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "watchdog",
    srcs = ["watchdog.cc"],
    hdrs = ["watchdog.h"],
    deps = [
        "//port",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
        "//port:timer",
    ],
)

libedgetpu_cc_library(
    name = "telemeter_interface",
    hdrs = [
        "telemeter_interface.h",
    ],
    deps = [
        ":execution_context_interface",
    ],
)

libedgetpu_cc_library(
    name = "execution_context_interface",
    hdrs = [
        "execution_context_interface.h",
    ],
)
