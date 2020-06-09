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

package(
    default_visibility = ["//visibility:public"],
)

licenses(["notice"])

cc_library(
    name = "driver_time_stamper",
    srcs = ["driver_time_stamper.cc"],
    hdrs = ["driver_time_stamper.h"],
    deps = [
        ":time_stamper",
        "//port",
    ],
)

cc_library(
    name = "time_stamper",
    hdrs = ["time_stamper.h"],
    deps = ["//port"],
)

cc_library(
    name = "driver_time_stamper_factory",
    hdrs = ["driver_time_stamper_factory.h"],
    deps = [
        ":driver_time_stamper",
        ":time_stamper",
        ":time_stamper_factory",
        "//port",
    ],
)

cc_library(
    name = "time_stamper_factory",
    hdrs = ["time_stamper_factory.h"],
    deps = [
        ":time_stamper",
        "//port",
    ],
)
