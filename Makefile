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
PYTHON3 ?= python3
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
BAZEL_BUILD_FLAGS_Linux := --crosstool_top=@crosstool//:toolchains \
                           --compiler=gcc
BAZEL_BUILD_OUTPUT_FILE_Linux := libedgetpu.so.1.0
BAZEL_BUILD_OUTPUT_SYMLINK_Linux := libedgetpu.so.1

ifeq ($(CPU), armv6)
BAZEL_BUILD_FLAGS_Linux += --linkopt=-L/usr/lib/arm-linux-gnueabihf/
endif

# Darwin-specific parameters
BAZEL_BUILD_TARGET_Darwin := //tflite/public:libedgetpu_direct_usb.dylib
BAZEL_BUILD_OUTPUT_FILE_Darwin := libedgetpu.1.0.dylib
BAZEL_BUILD_OUTPUT_SYMLINK_Darwin := libedgetpu.1.dylib

# Common parameters
BAZEL_BUILD_FLAGS := --sandbox_debug --subcommands \
  --experimental_repo_remote_exec \
  --compilation_mode=$(COMPILATION_MODE) \
  --define darwinn_portable=1 \
  --action_env PYTHON_BIN_PATH=$(shell which $(PYTHON3)) \
  --cpu=$(CPU) \
  --embed_label='TENSORFLOW_COMMIT=$(shell grep "TENSORFLOW_COMMIT =" $(MAKEFILE_DIR)/workspace.bzl | grep -o '[0-9a-f]\{40\}')' \
  --stamp
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

.PHONY: libedgetpu \
        libedgetpu-direct \
        libedgetpu-throttled \
        deb \
        deb-armhf \
        deb-arm64 \
        clean

libedgetpu: libedgetpu-direct libedgetpu-throttled

libedgetpu-direct:
	bazel build $(BAZEL_BUILD_FLAGS) $(BAZEL_BUILD_TARGET)
	$(call copy_out,direct)
	$(call strip_out,direct)

libedgetpu-throttled:
	bazel build $(BAZEL_BUILD_FLAGS) --copt=-DTHROTTLE_EDGE_TPU $(BAZEL_BUILD_TARGET)
	$(call copy_out,throttled)
	$(call strip_out,throttled)

deb:
	dpkg-buildpackage -rfakeroot -us -uc -tc -b

deb-armhf:
	dpkg-buildpackage -rfakeroot -us -uc -tc -b -a armhf -d

deb-arm64:
	dpkg-buildpackage -rfakeroot -us -uc -tc -b -a arm64 -d

clean:
	rm -rf $(OUT_DIR)

################################################################################
# Docker commands
################################################################################
DOCKER_CONTEXT_DIR := $(MAKEFILE_DIR)/docker
DOCKER_WORKSPACE := $(MAKEFILE_DIR)
DOCKER_CONTAINER_WORKSPACE := /workspace
DOCKER_CPUS ?= k8 armv7a armv6 aarch64
DOCKER_TARGETS ?=
DOCKER_IMAGE ?= debian:stretch
DOCKER_TAG_BASE ?= libedgetpu-cross
DOCKER_TAG := "$(DOCKER_TAG_BASE)-$(subst :,-,$(DOCKER_IMAGE))"
DOCKER_SHELL_COMMAND ?=

DOCKER_MAKE_COMMAND := \
for cpu in $(DOCKER_CPUS); do \
    make CPU=\$${cpu} -C $(DOCKER_CONTAINER_WORKSPACE) $(DOCKER_TARGETS) || exit 1; \
done

define docker_run_command
chmod a+w /; \
groupadd --gid $(shell id -g) $(shell id -g -n); \
useradd -m -e '' -s /bin/bash --gid $(shell id -g) --uid $(shell id -u) $(shell id -u -n); \
echo '$(shell id -u -n) ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers; \
su $(shell id -u -n) $(if $(1),-c '$(1)',)
endef

docker-image:
	docker build $(DOCKER_IMAGE_OPTIONS) -t $(DOCKER_TAG) \
	    --build-arg IMAGE=$(DOCKER_IMAGE) $(DOCKER_CONTEXT_DIR)

docker-shell: docker-image
	docker run --rm -i --tty --workdir $(DOCKER_CONTAINER_WORKSPACE) \
	    -v $(DOCKER_WORKSPACE):$(DOCKER_CONTAINER_WORKSPACE) \
	    $(DOCKER_TAG) /bin/bash -c "$(call docker_run_command,$(DOCKER_SHELL_COMMAND))"

docker-build: docker-image
	docker run --rm -i $(shell tty -s && echo --tty) \
	    -v $(DOCKER_WORKSPACE):$(DOCKER_CONTAINER_WORKSPACE) \
	    $(DOCKER_TAG) /bin/bash -c "$(call docker_run_command,$(DOCKER_MAKE_COMMAND))"
