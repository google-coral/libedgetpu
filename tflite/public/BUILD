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

licenses(["notice"])

# Version number used in the soname of shared libraries.
VERSION = "1"

SHARED_LIBRARY_LINKOPTS = [
    "-Wl,-soname,libedgetpu.so." + VERSION,
    "-Wl,--version-script=$(location libedgetpu.lds)",
]

DLL_ADDITIONAL_LINKOPTS = [
    ":edgetpu.res",
]

DLL_LINKOPTS = [
    "$(location edgetpu.res)",
]

config_setting(
    name = "dbg",
    values = {
        "compilation_mode": "dbg",
    },
)

genrule(
    name = "dll_res_gen",
    srcs = [
        ":edgetpu.rc",
    ],
    outs = [
        "edgetpu.res",
    ],
    cmd_bat = "rc.exe " +
              select({
                  ":dbg": "/D_DEBUG",
                  "//conditions:default": "",
              }) +
              " /nologo /fo $(location edgetpu.res) $(location edgetpu.rc)",
)

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
        "@org_tensorflow//tensorflow/lite/c:common",
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
        ":edgetpu_internal_direct_all",
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
        ":edgetpu_internal_direct_pci",
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
        ":edgetpu_internal_direct_usb",
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
        ":edgetpu_internal_direct_usb",
    ],
)

cc_binary(
    name = "edgetpu_direct_usb.dll",
    additional_linker_inputs = DLL_ADDITIONAL_LINKOPTS,
    linkopts = DLL_LINKOPTS,
    linkshared = 1,
    tags = [
        "manual",
        "nobuilder",
        "notap",
    ],
    deps = [
        ":edgetpu_internal_direct_usb",
        "@libusb//:shared",
    ],
)

cc_binary(
    name = "edgetpu_direct_pci.dll",
    additional_linker_inputs = DLL_ADDITIONAL_LINKOPTS,
    linkopts = DLL_LINKOPTS,
    linkshared = 1,
    tags = [
        "manual",
        "nobuilder",
        "notap",
    ],
    deps = [
        ":edgetpu_internal_direct_pci_windows",
    ],
)

filegroup(
    name = "edgetpu_direct_pci_interface_library",
    srcs = [":edgetpu_direct_pci.dll"],
    output_group = "interface_library",
)

cc_import(
    name = "edgetpu_direct_pci",
    hdrs = [
        "edgetpu.h",
    ],
    interface_library = ":edgetpu_direct_pci_interface_library",
    shared_library = ":edgetpu_direct_pci.dll",
)

cc_binary(
    name = "edgetpu_direct_all.dll",
    additional_linker_inputs = DLL_ADDITIONAL_LINKOPTS,
    linkopts = DLL_LINKOPTS,
    linkshared = 1,
    tags = [
        "manual",
        "nobuilder",
        "notap",
    ],
    deps = [
        ":edgetpu_internal_direct_all_windows",
        "@libusb//:shared",
    ],
)

filegroup(
    name = "edgetpu_direct_all_interface_library",
    srcs = [":edgetpu_direct_all.dll"],
    output_group = "interface_library",
)

cc_import(
    name = "edgetpu_direct_all",
    hdrs = [
        "edgetpu.h",
    ],
    interface_library = ":edgetpu_direct_all_interface_library",
    shared_library = ":edgetpu_direct_all.dll",
)

cc_library(
    name = "edgetpu_internal_direct_all",
    deps = [
        "//driver/beagle:beagle_all_driver_provider_linux",
        "//tflite:custom_op_direct",
    ],
)

cc_library(
    name = "edgetpu_internal_direct_all_windows",
    deps = [
        "//driver/beagle:beagle_all_driver_provider_windows",
        "//tflite:custom_op_direct",
    ],
)

cc_library(
    name = "edgetpu_internal_direct_pci",
    deps = [
        "//driver/beagle:beagle_pci_driver_provider_linux",
        "//tflite:custom_op_direct",
    ],
)

cc_library(
    name = "edgetpu_internal_direct_pci_windows",
    deps = [
        "//driver/beagle:beagle_pci_driver_provider_windows",
        "//tflite:custom_op_direct",
    ],
)

cc_library(
    name = "edgetpu_internal_direct_usb",
    deps = [
        "//driver/beagle:beagle_usb_driver_provider",
        "//tflite:custom_op_direct",
    ],
)

cc_library(
    name = "oss_edgetpu_direct_usb",
    deps = [":edgetpu_internal_direct_usb"],
)

cc_library(
    name = "oss_edgetpu_direct_pci",
    deps = select({
        "//:windows": [":edgetpu_internal_direct_pci_windows"],
        "//conditions:default": [":edgetpu_internal_direct_pci"],
    }),
)

cc_library(
    name = "oss_edgetpu_direct_all",
    deps = select({
        "//:windows": [":edgetpu_internal_direct_all_windows"],
        "//:darwin": [":edgetpu_internal_direct_usb"],
        "//conditions:default": [":edgetpu_internal_direct_all"],
    }),
)
