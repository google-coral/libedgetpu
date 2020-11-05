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
#  Port of various google3 libraries and utilities. In google3, blaze allows
#  building two variants controlled by the --define=darwinn_portable flag.
#   --define=darwinn_portable=0 (default) :
#    Port directory points to google3. (DARWINN_PORT_GOOGLE3 is defined.)
#   --define=darwinn_portable=1 :
#    Port directory points to default/ (DARWINN_PORT_DEFAULT is defined.)
# Android:
#  For compiling against Android NDK (with google3 blaze) both above configurations
#  will work.
#  For compiling as part of Android system however, DARWINN_PORT_ANDROID
#  must be defined as part of the Android build system.

load("//:libedgetpu_cc_rules.bzl", "libedgetpu_cc_library")
load("//:build_defs.bzl", "darwinn_port_defines")

package(default_visibility = ["//visibility:public"])

# All Google Owned Code except :
# - certain files in port/default/ that are under Apache 2.0 license.
licenses(["notice"])

libedgetpu_cc_library(
    name = "port",
    hdrs = [
        "aligned_malloc.h",
        "array_slice.h",
        "builddata.h",
        "casts.h",
        "cleanup.h",
        "defs.h",
        "errors.h",
        "gflags.h",
        "integral_types.h",
        "logging.h",
        "macros.h",
        "math_util.h",
        "openssl.h",
        "ptr_util.h",
        "status.h",
        "status_macros.h",
        "statusor.h",
        "stringprintf.h",
        "time.h",
        "unreachable.h",
    ],
    defines = darwinn_port_defines(),
    deps = select({
        "//:darwinn_portable": [
            "//port/default:port",
        ],
        "//conditions:default": [
            "//base",
            "//base:port",
            "//util/gtl:ptr_util",
            "//util/math:mathutil",
            "//util/task:status",
            "@com_google_absl//absl/time",
            "@com_google_absl//absl/types:span",
            "@com_google_absl//absl/status:statusor",
        ],
    }) + [
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:flag",
        "//port/default:cleanup",
        "//port/default:unreachable",
    ],
)

libedgetpu_cc_library(
    name = "aligned_malloc",
    hdrs = [
        "aligned_malloc.h",
        "defs.h",
    ],
    defines = darwinn_port_defines(),
    deps = select({
        "//:darwinn_portable": [
            "//port/default:port",
        ],
        "//conditions:default": [
            "//base",
            "//base:port",
        ],
    }),
)

libedgetpu_cc_library(
    name = "math_util",
    hdrs = [
        "defs.h",
        "math_util.h",
    ],
    defines = darwinn_port_defines(),
    deps = select({
        "//:darwinn_portable": [
            "//port/default:port",
        ],
        "//conditions:default": [
            "//util/math:mathutil",
        ],
    }),
)

libedgetpu_cc_library(
    name = "unreachable",
    hdrs = [
        "unreachable.h",
    ],
    defines = darwinn_port_defines(),
    deps = [
        "//port/default:unreachable",
    ],
)

libedgetpu_cc_library(
    name = "macros",
    hdrs = [
        "defs.h",
        "macros.h",
    ],
    defines = darwinn_port_defines(),
    deps = select({
        "//:darwinn_portable": [
            "//port/default:port",
        ],
        "//:darwinn_firmware": [
            "//port/default:port",
        ],
        "//conditions:default": [
            "//base",
        ],
    }),
)

libedgetpu_cc_library(
    name = "logging",
    hdrs = [
        "defs.h",
        "logging.h",
    ],
    defines = darwinn_port_defines(),
    deps = select({
        "//:darwinn_portable": [
            "//port/default:port",
        ],
        "//:darwinn_firmware": [
            "//port/firmware:logging",
        ],
        "//conditions:default": [
            "//base",
        ],
    }),
)

# This library is used for Darwinn 1.0 runtime only (Android/Google3).
libedgetpu_cc_library(
    name = "thread_annotations",
    hdrs = [
        "defs.h",
        "thread_annotations.h",
    ],
    defines = darwinn_port_defines(),
    deps = [
        "//port/default:thread_annotations",
    ],
)

libedgetpu_cc_library(
    name = "integral_types",
    hdrs = [
        "defs.h",
        "integral_types.h",
    ],
    defines = darwinn_port_defines(),
    deps = select({
        "//:darwinn_portable": [
            "//port/default:port",
        ],
        "//:darwinn_firmware": [
            "//port/default:port",
        ],
        "//conditions:default": [
            "//base",
        ],
    }),
)

libedgetpu_cc_library(
    name = "std_mutex_lock",
    hdrs = ["std_mutex_lock.h"],
    deps = [
        ":port",
        "//port:thread_annotations",
    ],
)

libedgetpu_cc_library(
    name = "shared_mutex",
    srcs = [
        "shared_mutex.cc",
    ],
    hdrs = ["shared_mutex.h"],
    deps = [
        "port",
        "//port:thread_annotations",
    ],
)

libedgetpu_cc_library(
    name = "mutex",
    hdrs = [
        "defs.h",
        "mutex.h",
    ],
    defines = darwinn_port_defines(),
    deps = select({
        "//:darwinn_portable": [
            "//port/default:port",
            "//port:thread_annotations",
        ],
        "//:darwinn_firmware": [
            "//third_party/safertos_addons",
        ],
        "//conditions:default": [
            "@com_google_absl//absl/synchronization",
        ],
    }),
    alwayslink = 1,
)

libedgetpu_cc_library(
    name = "dma330",
    hdrs = [
        "defs.h",
        "dma330.h",
    ],
    defines = darwinn_port_defines(),
    deps = select({
        "//:darwinn_firmware": [
            "//firmware/driver/dma:dma330",
        ],
        "//conditions:default": [
            "//port/default:dma330",
        ],
    }),
    alwayslink = 1,
)

libedgetpu_cc_library(
    name = "semaphore",
    hdrs = [
        "defs.h",
        "semaphore.h",
    ],
    defines = darwinn_port_defines(),
    deps = select({
        "//:darwinn_firmware": [
            "//third_party/safertos_addons",
        ],
        "//conditions:default": [
            "//port/default:semaphore",
        ],
    }),
    alwayslink = 1,
)

libedgetpu_cc_library(
    name = "condition_variable",
    hdrs = [
        "condition_variable.h",
        "defs.h",
    ],
    defines = darwinn_port_defines(),
    deps = select({
        "//:darwinn_firmware": [
            "//firmware/common:condition_variable",
        ],
        "//conditions:default": [
            "@com_google_absl//absl/synchronization",
            "@com_google_absl//absl/time",
        ],
    }),
    alwayslink = 1,
)

libedgetpu_cc_library(
    name = "restartable_thread",
    hdrs = ["restartable_thread.h"],
    defines = darwinn_port_defines(),
    deps = select({
        "//:darwinn_firmware": ["//port/firmware:restartable_thread"],
        "//conditions:default": ["//port/default:restartable_thread"],
    }),
    alwayslink = 1,
)

libedgetpu_cc_library(
    name = "blocking_queue",
    hdrs = ["blocking_queue.h"],
    defines = darwinn_port_defines(),
    deps = select({
        "//:darwinn_firmware": ["//port/firmware:blocking_queue"],
        "//conditions:default": ["//port/default:blocking_queue"],
    }),
    alwayslink = 1,
)

libedgetpu_cc_library(
    name = "lock_guard",
    srcs = ["lock_guard.cc"],
    hdrs = ["lock_guard.h"],
    defines = darwinn_port_defines(),
    deps = [
        "//port:mutex",
        "//port:thread_annotations_firmware",
    ],
    alwayslink = 1,
)

libedgetpu_cc_library(
    name = "blocking_counter",
    srcs = ["blocking_counter.cc"],
    hdrs = ["blocking_counter.h"],
    deps = [
        ":port",
        ":std_mutex_lock",
        "//port:thread_annotations",
    ],
)

# If --define darwinn_xprof_enabled=1, links with xprof libraries.
config_setting(
    name = "darwinn_xprof_enabled",
    values = {
        "define": "darwinn_xprof_enabled=1",
    },
)

# If --define darwinn_android_google3_trace_enabled=1, enable traces for
# android binaries built in google3.
config_setting(
    name = "darwinn_android_google3_trace_enabled",
    values = {
        "define": "darwinn_android_google3_trace_enabled=1",
    },
)

# If --define darwinn_csv_trace_enabled=1, generates trace outputs in csv.
config_setting(
    name = "darwinn_csv_trace_enabled",
    values = {
        "define": "darwinn_csv_trace_enabled=1",
    },
)

# If --define darwinn_perfetto_trace_enabled=1, generates trace outputs in perfetto proto file.
config_setting(
    name = "darwinn_perfetto_trace_enabled",
    values = {
        "define": "darwinn_perfetto_trace_enabled=1",
    },
)

# If --define pnp_benchmarking=1, control what trace points are generated to minimize intrusion.
# (used by Silicon Software PnP team)
config_setting(
    name = "pnp_benchmarking",
    values = {
        "define": "pnp_benchmarking=1",
    },
)

libedgetpu_cc_library(
    name = "tracing",
    hdrs = ["tracing.h"],
    defines = darwinn_port_defines(),
)

libedgetpu_cc_library(
    name = "string_util",
    hdrs = ["string_util.h"],
    deps = [
        "//port/default:strcat",
    ],
)

libedgetpu_cc_library(
    name = "timer",
    srcs = select({
        "//:windows": ["timer_windows.cc"],
        "//:darwin": ["timer_darwin.cc"],
        "//conditions:default": ["timer_linux.cc"],
    }),
    hdrs = ["timer.h"] + select({
        "//:windows": ["timer_windows.h"],
        "//:darwin": ["timer_darwin.h"],
        "//conditions:default": ["timer_linux.h"],
    }),
    deps = [
        ":port",
    ],
)

libedgetpu_cc_library(
    name = "posix_time",
    srcs = ["posix_time.cc"],
    hdrs = ["posix_time.h"],
    deps = [
        ":integral_types",
    ],
)

libedgetpu_cc_library(
    name = "fileio",
    srcs = select({
        "//:windows": ["fileio_windows.cc"],
        "//conditions:default": [],
    }),
    hdrs = ["fileio.h"] + select({
        "//:windows": ["fileio_windows.h"],
        "//conditions:default": ["fileio_linux.h"],
    }),
    deps = [
        ":port",
    ],
)

libedgetpu_cc_library(
    name = "cpu",
    hdrs = ["cpu.h"],
    defines = darwinn_port_defines(),
    deps = select({
        "//:darwinn_firmware": ["//port/firmware:cpu"],
        "//conditions:default": ["//port/default:cpu"],
    }),
)

libedgetpu_cc_library(
    name = "demangle",
    hdrs = [
        "defs.h",
        "demangle.h",
    ],
    defines = darwinn_port_defines(),
    deps = select({
        "//conditions:default": [
            "//base",
        ],
        "//:darwinn_portable": [
        ],
        "//:darwinn_firmware": [
        ],
    }),
)

libedgetpu_cc_library(
    name = "bsp",
    hdrs = [
        "bsp.h",
        "defs.h",
    ],
    defines = darwinn_port_defines(),
    deps = select({
        "//:darwinn_firmware": [
            "//firmware:bsp",
        ],
        "//conditions:default": [
        ],
    }),
    alwayslink = 1,
)

libedgetpu_cc_library(
    name = "status_macros",
    hdrs = [
        "defs.h",
        "status_macros.h",
    ],
    defines = darwinn_port_defines(),
    deps = select({
        "//:darwinn_portable": [
            "//port/default:port",
        ],
        "//conditions:default": [
            "//util/task:status",
        ],
    }),
)

libedgetpu_cc_library(
    name = "dma_manager",
    hdrs = [
        "defs.h",
        "dma_manager.h",
    ],
    defines = darwinn_port_defines(),
    deps = select({
        "//:darwinn_firmware": [
            "//firmware/driver/dma:dma_manager",
        ],
        "//conditions:default": [
            "//port/default:dma_manager",
        ],
    }),
    alwayslink = 1,
)
