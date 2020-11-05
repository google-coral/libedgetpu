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
#   Beagle-specific configuration and CSR Layouts.

load("//:libedgetpu_cc_rules.bzl", "libedgetpu_cc_library")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

exports_files(glob(["**/*"]))

libedgetpu_cc_library(
    name = "beagle_config",
    hdrs = [
        "beagle_chip_structures.h",
        "beagle_csr_offsets.h",
    ],
    deps = [
        "//driver/config",
        "//driver/config:register_constants",
    ],
)

libedgetpu_cc_library(
    name = "beagle_chip_config",
    hdrs = ["beagle_chip_config.h"],
    deps = [
        ":beagle_config",
        "//driver/config",
        "//port:logging",
        "//port:unreachable",
    ],
)
