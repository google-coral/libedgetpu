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

# Description: time related libraries used by DarwiNN drivers.

load("//:libedgetpu_cc_rules.bzl", "libedgetpu_cc_library")

package(
    default_visibility = ["//visibility:public"],
)

licenses(["notice"])

libedgetpu_cc_library(
    name = "driver_time_stamper",
    srcs = ["driver_time_stamper.cc"],
    hdrs = ["driver_time_stamper.h"],
    deps = [
        ":time_stamper",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "mock_time_stamper",
    testonly = 1,
    hdrs = ["mock_time_stamper.h"],
    deps = [
        ":time_stamper",
        "//port:lock_guard",
        "//port:port_test_helper",
    ],
)

libedgetpu_cc_library(
    name = "mock_time_stamper_no_config",
    testonly = 1,
    hdrs = ["mock_time_stamper_no_config.h"],
    deps = [
        ":time_stamper",
        "//testing/base/public:gunit_main",
    ],
)

libedgetpu_cc_library(
    name = "time_stamper",
    hdrs = ["time_stamper.h"],
    deps = ["//port"],
)

libedgetpu_cc_library(
    name = "mock_time_stamper_factory",
    testonly = 1,
    hdrs = ["mock_time_stamper_factory.h"],
    deps = [
        ":mock_time_stamper",
        ":time_stamper",
        ":time_stamper_factory",
        "//port",
        "//port:port_test_helper",
    ],
)

libedgetpu_cc_library(
    name = "driver_time_stamper_factory",
    hdrs = ["driver_time_stamper_factory.h"],
    deps = [
        ":driver_time_stamper",
        ":time_stamper",
        ":time_stamper_factory",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "time_stamper_factory",
    hdrs = ["time_stamper_factory.h"],
    deps = [
        ":time_stamper",
        "//port",
    ],
)
