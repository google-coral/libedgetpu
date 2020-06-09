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
#  DarwiNN Runtime Libaries.

package(default_visibility = ["//visibility:public"])

# All Google Owned Code except :
# - certain files in port/default/ that are under Apache 2.0 license.
licenses(["notice"])

exports_files([
    "LICENSE",
])

# If --define darwinn_portable=1, compile without google3 deps.
config_setting(
    name = "darwinn_portable",
    values = {
        "define": "darwinn_portable=1",
    },
)

# If --define darwinn_portable=1 AND this is an otherwise non-portable config.
config_setting(
    name = "darwinn_portable_with_non_portable_os",
    flag_values = {"//tools/cpp:cc_target_os": "linux-google"},
    values = {"define": "darwinn_portable=1"},
)

# If --define darwinn_firmware=1, compile with minimal deps.
config_setting(
    name = "darwinn_firmware",
    values = {
        "define": "darwinn_firmware=1",
    },
)

config_setting(
    name = "windows",
    values = {
        "cpu": "x64_windows",
    },
)

config_setting(
    name = "darwin",
    values = {
        "cpu": "darwin",
    },
)
