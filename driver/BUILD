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
#  Portable DarwiNN driver implementation.

load("//:libedgetpu_cc_rules.bzl", "libedgetpu_cc_library")

package(
    default_visibility = ["//visibility:public"],
)

licenses(["notice"])

exports_files(["libdarwinn_driver.lds"])

# Utility functions.
libedgetpu_cc_library(
    name = "util",
    hdrs = [
        "bitfield.h",
    ],
    deps = [
        "//port:integral_types",
        "//port:logging",
    ],
)

libedgetpu_cc_library(
    name = "allocator",
    srcs = [
        "aligned_allocator.cc",
        "allocator.cc",
    ],
    hdrs = [
        "aligned_allocator.h",
        "allocator.h",
    ],
    deps = [
        "//api:allocated_buffer",
        "//api:buffer",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "device_buffer",
    srcs = ["device_buffer.cc"],
    hdrs = ["device_buffer.h"],
    deps = [
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "executable_util",
    srcs = ["executable_util.cc"],
    hdrs = ["executable_util.h"],
    deps = [
        "//api:buffer",
        "//executable:executable_fbs",
        "//port",
    ],
)

# Driver Factory.
libedgetpu_cc_library(
    name = "driver_factory",
    srcs = ["driver_factory.cc"] + select({
        "//:windows": ["driver_factory_windows.cc"],
        "//:darwin": ["driver_factory_darwin.cc"],
        "//conditions:default": ["driver_factory_default.cc"],
    }),
    hdrs = [
        "driver_factory.h",
    ],
    deps = [
        ":driver",
        "//api:chip",
        "//api:driver",
        "//api:driver_factory",
        "//api:driver_options_fbs",
        "//api:driver_options_helper",
        "//driver/config",
        "//driver/kernel:gasket_ioctl",
        "//port",
        "//port:fileio",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
    ],
)

libedgetpu_cc_library(
    name = "driver_helper",
    srcs = ["driver_helper.cc"],
    hdrs = ["driver_helper.h"],
    deps = [
        ":executable_util",
        ":package_registry",
        ":test_vector",
        "//api:buffer",
        "//api:chip",
        "//api:driver",
        "//api:package_reference",
        "//api:request",
        "//api:telemeter_interface",
        "//api:timing",
        "//executable:executable_fbs",
        "//port",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
    ],
)

libedgetpu_cc_library(
    name = "instruction_buffers",
    srcs = ["instruction_buffers.cc"],
    hdrs = ["instruction_buffers.h"],
    deps = [
        ":allocator",
        ":device_buffer_mapper",
        ":executable_util",
        "//api:buffer",
        "//executable:executable_fbs",
        "//port",
        "//port:tracing",
    ],
)

libedgetpu_cc_library(
    name = "run_controller",
    srcs = ["run_controller.cc"],
    hdrs = ["run_controller.h"],
    deps = [
        ":hardware_structures",
        "//driver/config",
        "//driver/config:register_constants",
        "//driver/registers",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "scalar_core_controller",
    srcs = ["scalar_core_controller.cc"],
    hdrs = ["scalar_core_controller.h"],
    deps = [
        "//driver/config",
        "//driver/interrupt:interrupt_controller",
        "//driver/registers",
        "//port",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
    ],
)

libedgetpu_cc_library(
    name = "hardware_structures",
    hdrs = ["hardware_structures.h"],
    deps = [
        "//port:integral_types",
        "//port:macros",
    ],
)

libedgetpu_cc_library(
    name = "driver",
    srcs = ["driver.cc"],
    hdrs = ["driver.h"],
    deps = [
        ":default_telemeter",
        ":device_buffer_mapper",
        ":package_registry",
        ":request",
        ":tpu_request",
        "@com_google_absl//absl/strings:str_format",
        "//api:buffer",
        "//api:chip",
        "//api:driver",
        "//api:execution_context_interface",
        "//api:package_reference",
        "//api:request",
        "//api:telemeter_interface",
        "//driver/memory:dma_direction",
        "//driver_shared/time_stamper",
        "//executable:executable_fbs",
        "//port",
        "//port:blocking_counter",
        "//port:shared_mutex",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
        "//port:tracing",
    ],
)

libedgetpu_cc_library(
    name = "default_telemeter",
    hdrs = ["default_telemeter.h"],
    deps = [
        "//api:telemeter_interface",
    ],
)

libedgetpu_cc_library(
    name = "mmio_driver",
    srcs = ["mmio_driver.cc"],
    hdrs = ["mmio_driver.h"],
    deps = [
        ":allocator",
        ":device_buffer",
        ":device_buffer_mapper",
        ":dma_info_extractor",
        ":driver",
        ":hardware_structures",
        ":package_registry",
        ":real_time_dma_scheduler",
        ":run_controller",
        ":scalar_core_controller",
        ":single_tpu_request",
        ":top_level_handler",
        ":tpu_request",
        "//api:allocated_buffer",
        "//api:buffer",
        "//api:watchdog",
        "//driver/config",
        "//driver/config:register_constants",
        "//driver/interrupt:interrupt_controller_interface",
        "//driver/interrupt:interrupt_handler",
        "//driver/interrupt:top_level_interrupt_manager",
        "//driver/memory:address_space",
        "//driver/memory:address_utilities",
        "//driver/memory:dma_direction",
        "//driver/memory:dram_allocator",
        "//driver/memory:mmu_mapper",
        "//driver/mmio:host_queue",
        "//driver/registers",
        "//driver_shared/time_stamper",
        "//driver_shared/time_stamper:driver_time_stamper",
        "//executable:executable_fbs",
        "//port",
        "//port:fileio",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
        "//port:tracing",
    ],
)

libedgetpu_cc_library(
    name = "dma_info",
    srcs = ["dma_info.cc"],
    hdrs = ["dma_info.h"],
    deps = [
        ":device_buffer",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "device_buffer_mapper",
    srcs = ["device_buffer_mapper.cc"],
    hdrs = ["device_buffer_mapper.h"],
    deps = [
        ":device_buffer",
        ":hardware_structures",
        "//api:buffer",
        "//driver/memory:address_space",
        "//driver/memory:address_utilities",
        "//driver/memory:dma_direction",
        "//port",
        "//port:tracing",
    ],
)

libedgetpu_cc_library(
    name = "dma_info_extractor",
    srcs = ["dma_info_extractor.cc"],
    hdrs = ["dma_info_extractor.h"],
    deps = [
        ":device_buffer_mapper",
        ":dma_info",
        ":package_registry",
        "//driver/memory:address_utilities",
        "//executable:executable_fbs",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "dma_scheduler",
    hdrs = ["dma_scheduler.h"],
    deps = [
        ":dma_info",
        ":tpu_request",
        "//api:driver",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "single_queue_dma_scheduler",
    srcs = ["single_queue_dma_scheduler.cc"],
    hdrs = ["single_queue_dma_scheduler.h"],
    deps = [
        ":dma_info",
        ":dma_scheduler",
        ":tpu_request",
        "//api:driver",
        "//api:watchdog",
        "//port",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
        "//port:tracing",
    ],
)

libedgetpu_cc_library(
    name = "real_time_dma_scheduler",
    srcs = ["real_time_dma_scheduler.cc"],
    hdrs = ["real_time_dma_scheduler.h"],
    deps = [
        ":dma_info",
        ":dma_scheduler",
        ":package_registry",
        ":single_queue_dma_scheduler",
        ":tpu_request",
        "@com_google_absl//absl/strings:str_format",
        "//api:driver",
        "//api:package_reference",
        "//api:timing",
        "//api:watchdog",
        "//driver_shared/time_stamper",
        "//port",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
    ],
)

libedgetpu_cc_library(
    name = "dma_chunker",
    srcs = ["dma_chunker.cc"],
    hdrs = ["dma_chunker.h"],
    deps = [
        ":device_buffer",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "top_level_handler",
    hdrs = ["top_level_handler.h"],
    deps = ["//port"],
)

libedgetpu_cc_library(
    name = "package_registry",
    srcs = ["package_registry.cc"],
    hdrs = ["package_registry.h"],
    deps = [
        ":allocator",
        ":device_buffer_mapper",
        ":instruction_buffers",
        ":package_verifier",
        "//api:buffer",
        "//api:chip",
        "//api:driver_options_fbs",
        "//api:execution_context_interface",
        "//api:layer_information",
        "//api:package_reference",
        "//api:runtime_version",
        "//driver/memory:dram_allocator",
        "//executable:executable_fbs",
        "//port",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
        "//port:tracing",
    ],
)

libedgetpu_cc_library(
    name = "package_verifier",
    srcs = ["package_verifier.cc"],
    hdrs = ["package_verifier.h"],
    deps = [
        "//executable:executable_fbs",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "tpu_request",
    hdrs = ["tpu_request.h"],
    deps = [
        ":dma_info",
        ":package_registry",
        "//api:buffer",
        "//api:request",
        "//port",
    ],
)

libedgetpu_cc_library(
    name = "single_tpu_request",
    srcs = ["single_tpu_request.cc"],
    hdrs = ["single_tpu_request.h"],
    deps = [
        ":allocator",
        ":device_buffer",
        ":device_buffer_mapper",
        ":dma_info",
        ":dma_info_extractor",
        ":executable_util",
        ":hardware_structures",
        ":instruction_buffers",
        ":package_registry",
        ":request",
        ":tpu_request",
        "//api:allocated_buffer",
        "//api:buffer",
        "//driver/memory:address_space",
        "//driver/memory:dram_allocator",
        "//executable:executable_fbs",
        "//port",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
        "//port:tracing",
    ],
)

libedgetpu_cc_library(
    name = "request",
    srcs = ["request.cc"],
    hdrs = ["request.h"],
    deps = [
        ":tpu_request",
        "//api:request",
        "//driver_shared/time_stamper",
        "//driver_shared/time_stamper:driver_time_stamper_factory",
        "//port",
        "//port:std_mutex_lock",
        "//port:thread_annotations",
        "//port:tracing",
    ],
)

filegroup(
    name = "linker_script",
    srcs = ["libdarwinn_driver.lds"],
)
