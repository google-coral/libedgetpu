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

cc_library(
    name = "export_graph_fbs",
    data = [
        "graph.fbs",
    ],
)

flatbuffer_cc_library(
    name = "executable_fbs",
    srcs = ["executable.fbs"],
    flatc_args = [
        "--gen-object-api",  # Adds 46KB to the generated code.
        "--force-empty",
        "--gen-mutable",  # No size increase to the generated code.
    ],
)

# The extended version has a larger code size and is meant for tools/tests.
# Make sure it does not overlap with executable_fbs.
flatbuffer_cc_library(
    name = "executable_fbs_extended",
    srcs = ["executable.fbs"],
    flatc_args = [
        "--gen-object-api",
        "--force-empty",
        "--reflect-names",
    ],
    out_prefix = "extended/",
)

# Flatbuffer library for graph that would be interpreted by the runtime.
flatbuffer_cc_library(
    name = "graph_fbs",
    srcs = ["graph.fbs"],
    flatc_args = [
        "--gen-object-api",
        "--force-empty",
        "--gen-mutable",
    ],
    include_paths = [
        ".",
        "third_party/darwinn/executable",
    ],
)

# Flatbuffer library for graph that would be interpreted by the runtime.
# The extended version has a larger code size and is meant for tools/tests.
# Make sure it does not overlap with graph_fbs.
flatbuffer_cc_library(
    name = "graph_fbs_extended",
    srcs = ["graph.fbs"],
    flatc_args = [
        "--gen-object-api",
        "--force-empty",
        "--reflect-names",
    ],
    include_paths = [
        ".",
        "third_party/darwinn/executable",
    ],
    out_prefix = "extended/",
)

filegroup(
    name = "graph_schema",
    srcs = [
        "graph.fbs",
    ],
)

flatbuffer_cc_library(
    name = "vii_commands_fbs",
    srcs = ["vii_commands.fbs"],
    flatc_args = [
        "--gen-object-api",
        "--force-empty",
        "--gen-mutable",
    ],
)
