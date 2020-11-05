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
#   Contains definitions for the Darwinn Executable format that serves as the
#   compiler / runtime and firmware contract.

# Keep the following two load calls seperate, otherwise copybara can not
# clean it up properly.
load(
    "@flatbuffers//:build_defs.bzl",
    "flatbuffer_cc_library",
    "flatbuffer_py_library",
)

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

flatbuffer_cc_library(
    name = "executable_fbs",
    srcs = ["executable.fbs"],
    flatc_args = [
        "--gen-object-api",  # Adds 46KB to the generated code.
        "--force-empty",
        "--gen-mutable",  # No size increase to the generated code.
    ],
)
