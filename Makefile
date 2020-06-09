# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
SHELL := /bin/bash
MAKEFILE_DIR := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
OUT_DIR := $(MAKEFILE_DIR)/out
OS := $(shell uname -s)

ifeq ($(OS),Linux)
CPU ?= k8
else ifeq ($(OS),Darwin)
CPU ?= darwin
else
$(error $(OS) is not supported)
endif

ifeq ($(filter $(CPU),k8 armv6 armv7a aarch64 darwin),)
$(error CPU must be k8, armv7a, armv6, aarch64, or darwin)
endif

COMPILATION_MODE ?= opt
ifeq ($(filter $(COMPILATION_MODE),opt dbg),)
$(error COMPILATION_MODE must be opt or dbg)
endif

BAZEL_OUT_DIR := $(MAKEFILE_DIR)/bazel-out/$(CPU)-$(COMPILATION_MODE)/bin

# Linux-specific parameters
BAZEL_BUILD_TARGET_Linux := //tflite/public:libedgetpu_direct_all.so
# --experimental_repo_remote_exec for remotable parameter used in
# --`repository_rule` from TF.
BAZEL_BUILD_FLAGS_Linux := --crosstool_top=@crosstool//:toolchains \
                           --compiler=gcc \
                           --linkopt=-l:libusb-1.0.so \
                           --experimental_repo_remote_exec
BAZEL_BUILD_OUTPUT_FILE_Linux := libedgetpu.so.1.0
BAZEL_BUILD_OUTPUT_SYMLINK_Linux := libedgetpu.so.1

ifeq ($(COMPILATION_MODE), opt)
BAZEL_BUILD_FLAGS_Linux += --linkopt=-Wl,--strip-all
endif
ifeq ($(CPU), armv6)
BAZEL_BUILD_FLAGS_Linux += --linkopt=-L/usr/lib/arm-linux-gnueabihf/
endif

# Darwin-specific parameters
BAZEL_BUILD_TARGET_Darwin := //tflite/public:libedgetpu_direct_usb.dylib
BAZEL_BUILD_FLAGS_Darwin := --linkopt=-L/opt/local/lib \
                            --linkopt=-lusb-1.0 \
                            --copt=-fvisibility=hidden
BAZEL_BUILD_OUTPUT_FILE_Darwin := libedgetpu.1.0.dylib
BAZEL_BUILD_OUTPUT_SYMLINK_Darwin := libedgetpu.1.dylib

# Common parameters
BAZEL_BUILD_FLAGS := --sandbox_debug --subcommands \
  --compilation_mode=$(COMPILATION_MODE) \
  --define darwinn_portable=1 \
  --copt=-DSTRIP_LOG=1 \
  --copt=-DEDGETPU_EXTERNAL_RELEASE_RUNTIME \
  --copt=-fno-rtti \
  --copt=-fno-exceptions \
  --copt='-D__FILE__=""' \
  --cpu=$(CPU)
BAZEL_BUILD_FLAGS += $(BAZEL_BUILD_FLAGS_$(OS))
BAZEL_BUILD_TARGET := $(BAZEL_BUILD_TARGET_$(OS))
BAZEL_BUILD_OUTPUT_FILE := $(BAZEL_BUILD_OUTPUT_FILE_$(OS))
BAZEL_BUILD_OUTPUT_SYMLINK := $(BAZEL_BUILD_OUTPUT_SYMLINK_$(OS))

define copy_out
mkdir -p $(OUT_DIR)/$(1)/$(CPU) && \
cp -f $(BAZEL_OUT_DIR)/tflite/public/*$(suffix $(BAZEL_BUILD_TARGET)) \
      $(OUT_DIR)/$(1)/$(CPU)/$(BAZEL_BUILD_OUTPUT_FILE) && \
ln -fs $(BAZEL_BUILD_OUTPUT_FILE) \
       $(OUT_DIR)/$(1)/$(CPU)/$(BAZEL_BUILD_OUTPUT_SYMLINK)
endef

ifeq ($(OS),Darwin)
ifeq ($(COMPILATION_MODE),opt)
define strip_out
strip -x -S -o $(OUT_DIR)/$(1)/$(CPU)/$(BAZEL_BUILD_OUTPUT_FILE) \
               $(OUT_DIR)/$(1)/$(CPU)/$(BAZEL_BUILD_OUTPUT_FILE)
endef
endif
endif

libedgetpu: libedgetpu-direct libedgetpu-throttled

libedgetpu-direct:
	bazel build $(BAZEL_BUILD_FLAGS) $(BAZEL_BUILD_TARGET)
	$(call copy_out,direct)
	$(call strip_out,direct)

libedgetpu-throttled:
	bazel build $(BAZEL_BUILD_FLAGS) --copt=-DTHROTTLE_EDGE_TPU $(BAZEL_BUILD_TARGET)
	$(call copy_out,throttled)
	$(call strip_out,throttled)

clean:
	rm -rf $(OUT_DIR)

ifdef DOCKER_MK
DOCKER_WORKSPACE := $(MAKEFILE_DIR)
DOCKER_TAG_BASE=coral-libedgetpu
include $(DOCKER_MK)
endif
