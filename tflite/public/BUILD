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

# BUILD rules for DarwiNN TfLite Custom-op public interface.

package(
    default_visibility = ["//visibility:public"],
)

# This group is meant for users of USB TPUs who also have a dependency to
# libcap (ex: CNS file support depends on libcap). Using the default USB
# driver wouldn't work in that case as libUSB is dynamically linked and
# thus the global _cap_names in libcap ends up twice in your binary,
# causing an ODR violation. Also to comply with libUSB LGPL license, this
# version only exist for internal usage (no external release, this is why
# the user are limited to this group).
package_group(
    name = "libusb_statically_linked_users",
    packages = [
        "//lifescience/cad/ophthalmology/arda_camera/dagon/...",
        "//vr/stargate/...",
    ],
)

licenses(["notice"])

# Version number used in the soname of shared libraries.
VERSION = "1"

SHARED_LIBRARY_LINKOPTS = [
    "-Wl,-soname,libedgetpu.so." + VERSION,
    "-Wl,--version-script=$(location libedgetpu.lds)",
]

# Header for external use.
cc_library(
    name = "edgetpu",
    hdrs = [
        "edgetpu.h",
    ],
    defines = select({
        "//:windows": ["EDGETPU_COMPILE_LIBRARY"],
        "//conditions:default": [],
    }),
    deps = [
        "@org_tensorflow//tensorflow/lite:context",
    ],
)

cc_library(
    name = "edgetpu_c",
    hdrs = [
        "edgetpu_c.h",
    ],
    defines = select({
        "//:windows": ["EDGETPU_COMPILE_LIBRARY"],
        "//conditions:default": [],
    }),
    deps = [
        "@org_tensorflow//tensorflow/lite:context",
    ],
)

# Shared library for external use.
# Explicit variant for all(pci/usb).
cc_binary(
    name = "libedgetpu_direct_all.so",
    linkopts = SHARED_LIBRARY_LINKOPTS,
    linkshared = 1,
    linkstatic = 1,
    deps = [
        "libedgetpu.lds",
        "//driver/beagle:beagle_all_driver_provider",
        "//tflite:custom_op_direct",
    ],
)

# Shared library for external use.
# Explicit variant for Beagle PCIe.
cc_binary(
    name = "libedgetpu_direct_pci.so",
    linkopts = SHARED_LIBRARY_LINKOPTS,
    linkshared = 1,
    linkstatic = 1,
    deps = [
        "libedgetpu.lds",
        "//driver/beagle:beagle_pci_driver_provider",
        "//tflite:custom_op_direct",
    ],
)

# Shared library for linking of applications not depending on a particular driver provider.
cc_binary(
    name = "libedgetpu_bare.so",
    linkopts = SHARED_LIBRARY_LINKOPTS,
    linkshared = 1,
    linkstatic = 1,
    deps = [
        "libedgetpu.lds",
        "//tflite:custom_op_direct",
    ],
)

# Shared library for external use.
# Explicit variant for Beagle USB.
cc_binary(
    name = "libedgetpu_direct_usb.so",
    linkopts = SHARED_LIBRARY_LINKOPTS,
    linkshared = 1,
    linkstatic = 1,
    deps = [
        "libedgetpu.lds",
        "//driver/beagle:beagle_usb_driver_provider",
        "//tflite:custom_op_direct",
    ],
)

cc_binary(
    name = "libedgetpu_direct_usb.dylib",
    linkopts = [
        "-Wl,-install_name,@rpath/libedgetpu." + VERSION + ".dylib",
    ],
    linkshared = 1,
    linkstatic = 1,
    tags = [
        "manual",
        "nobuilder",
        "notap",
    ],
    deps = [
        "//driver/beagle:beagle_usb_driver_provider",
        "//tflite:custom_op_direct",
    ],
)

cc_binary(
    name = "edgetpu_direct_usb.dll",
    linkshared = 1,
    tags = [
        "manual",
        "nobuilder",
        "notap",
    ],
    deps = [
        "//driver/beagle:beagle_usb_driver_provider",
        "//tflite:custom_op_direct",
        "@libusb//:shared",
    ],
)
