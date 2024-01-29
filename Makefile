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
OUT_DIR ?= $(MAKEFILE_DIR)/out
DIST_DIR ?= $(MAKEFILE_DIR)/dist
OS := $(shell uname -s)
ARCH := $(shell uname -p)

ifeq ($(OS),Linux)
CPU ?= k8
sha256_check = echo "$(1)  $(2)" | sha256sum --check -
else ifeq ($(OS),Darwin)
ifeq ($(ARCH),arm)
CPU ?= darwin_arm64
else
CPU ?= darwin_x86_64
endif
sha256_check = echo "$(1) *$(2)" | shasum --algorithm 256 --check -
else
$(error $(OS) is not supported)
endif

ifeq ($(filter $(CPU),k8 armv7a aarch64 darwin_arm64 darwin_x86_64),)
$(error CPU must be k8, armv7a, aarch64, darwin_arm64, or darwin_x86_64)
endif

COMPILATION_MODE ?= opt
ifeq ($(filter $(COMPILATION_MODE),opt dbg),)
$(error COMPILATION_MODE must be opt or dbg)
endif

ifeq ($(COMPILATION_MODE),opt)
STRIPPED_SUFFIX = .stripped
endif

EDGETPU_RUNTIME_DIR := /tmp/edgetpu_runtime
LIBUSB_URL := https://github.com/libusb/libusb/releases/download/v1.0.24/libusb-1.0.24.7z
LIBUSB_SHA256 := 620cec4dbe4868202949294157da5adb75c9fbb4f04266146fc833eef85f90fb
USBDK_URL := https://github.com/daynix/UsbDk/releases/download/v1.00-22/UsbDk_1.0.22_x64.msi
USBDK_SHA256 := 91f6f695e1e13c656024e6d3b55620bf08d8835ef05ee0496935ba6bb62466a5
LIBEDGETPU_BIN ?= $(MAKEFILE_DIR)/out/

BAZEL_OUT_DIR := $(MAKEFILE_DIR)/bazel-out/$(CPU)-$(COMPILATION_MODE)/bin

# Linux-specific parameters
BAZEL_BUILD_TARGET_Linux := //tflite/public:libedgetpu_direct_all.so$(STRIPPED_SUFFIX)
BAZEL_BUILD_OUTPUT_FILE_Linux := libedgetpu.so.1.0
BAZEL_BUILD_OUTPUT_SYMLINK_Linux := libedgetpu.so.1

# Darwin-specific parameters
BAZEL_BUILD_TARGET_Darwin := //tflite/public:libedgetpu_direct_usb.dylib$(STRIPPED_SUFFIX)
BAZEL_BUILD_OUTPUT_FILE_Darwin := libedgetpu.1.0.dylib
BAZEL_BUILD_OUTPUT_SYMLINK_Darwin := libedgetpu.1.dylib

# Common parameters
BAZEL_BUILD_FLAGS = \
  --stripopt=-x \
  --compilation_mode=$(COMPILATION_MODE) \
  --cpu=$(CPU) \
  --embed_label='TENSORFLOW_COMMIT=$(shell bazel query "@libedgetpu_properties//..." | grep tensorflow_commit | cut -d\# -f2)' \
  --stamp

BAZEL_BUILD_TARGET := $(BAZEL_BUILD_TARGET_$(OS))
BAZEL_BUILD_OUTPUT_FILE := $(BAZEL_BUILD_OUTPUT_FILE_$(OS))
BAZEL_BUILD_OUTPUT_SYMLINK := $(BAZEL_BUILD_OUTPUT_SYMLINK_$(OS))

define copy_out
mkdir -p $(1) && \
cp -f $(BAZEL_OUT_DIR)/tflite/public/*$(suffix $(BAZEL_BUILD_TARGET)) \
      $(1)/$(BAZEL_BUILD_OUTPUT_FILE) && \
ln -fs $(BAZEL_BUILD_OUTPUT_FILE) \
       $(1)/$(BAZEL_BUILD_OUTPUT_SYMLINK)
endef

.PHONY: libedgetpu \
        libedgetpu-direct \
        libedgetpu-throttled \
        deb \
        clean

libedgetpu: libedgetpu-direct libedgetpu-throttled

libedgetpu-direct:
	bazel build $(BAZEL_BUILD_FLAGS) $(BAZEL_BUILD_TARGET)
	$(call copy_out,$(OUT_DIR)/direct/$(CPU))
	$(call strip_out,$(OUT_DIR)/direct/$(CPU))

libedgetpu-throttled:
	bazel build $(BAZEL_BUILD_FLAGS) --copt=-DTHROTTLE_EDGE_TPU $(BAZEL_BUILD_TARGET)
	$(call copy_out,$(OUT_DIR)/throttled/$(CPU))
	$(call strip_out,$(OUT_DIR)/throttled/$(CPU))

deb:
	dpkg-buildpackage -rfakeroot -us -uc -tc -b
	dpkg-buildpackage -rfakeroot -us -uc -tc -b -a armhf -d
	dpkg-buildpackage -rfakeroot -us -uc -tc -b -a arm64 -d
	mkdir -p $(DIST_DIR)
	mv $(MAKEFILE_DIR)/../*.{deb,changes,buildinfo} $(DIST_DIR)

runtime:
	rm -rf $(EDGETPU_RUNTIME_DIR) && mkdir -p $(EDGETPU_RUNTIME_DIR)/{libedgetpu,third_party/libusb_win,third_party/usbdk}
	cp -a $(LIBEDGETPU_BIN)/{direct,throttled} \
	      $(MAKEFILE_DIR)/tflite/public/{edgetpu.h,edgetpu_c.h} \
	      $(MAKEFILE_DIR)/debian/edgetpu-accelerator.rules \
	      $(EDGETPU_RUNTIME_DIR)/libedgetpu
	cp -r $(MAKEFILE_DIR)/coral_accelerator_windows \
	      $(EDGETPU_RUNTIME_DIR)/third_party
	(cd $(EDGETPU_RUNTIME_DIR)/libedgetpu && \
	 echo "Copyright 2021 Google LLC. This software is provided as-is, without warranty" >> LICENSE && \
	 echo "or representation for any use or purpose. Your use of it is subject to your"  >> LICENSE && \
	 echo "agreements with Google covering this software, or if no such agreement"       >> LICENSE && \
	 echo "applies, your use is subject to a limited, non-transferable, non-exclusive"   >> LICENSE && \
	 echo "license solely to run the software for your testing use, unless and until"    >> LICENSE && \
	 echo "revoked by Google."                                                           >> LICENSE)
	(cd $(EDGETPU_RUNTIME_DIR)/third_party/libusb_win && \
	 wget $(LIBUSB_URL) && \
	 $(call sha256_check,$(LIBUSB_SHA256),$(notdir $(LIBUSB_URL))) && \
	 7z e *.7z -o. "VS2019/MS64/dll/*.dll" && \
	 rm -f *.7z && \
	 echo "This folder contains an unmodified $(basename $(notdir $(LIBUSB_URL))) for 64-bit Windows." >> README && \
	 echo "It is extracted from the archive available at $(LIBUSB_URL)"                                >> README && \
	 echo "The library is licensed under LGPL 2.1."                                                    >> README)
	(cd $(EDGETPU_RUNTIME_DIR)/third_party/usbdk && \
	 wget $(USBDK_URL) && \
	 $(call sha256_check,$(USBDK_SHA256),$(notdir $(USBDK_URL))) && \
	 wget https://raw.githubusercontent.com/daynix/UsbDk/master/LICENSE)
	cp $(MAKEFILE_DIR)/scripts/{install.sh,uninstall.sh,install.bat,uninstall.bat} \
	   $(EDGETPU_RUNTIME_DIR)
	mkdir -p $(DIST_DIR)
	(cd $(shell dirname $(EDGETPU_RUNTIME_DIR)) && \
	 zip --symlinks -r $(DIST_DIR)/edgetpu_runtime_$(shell date '+%Y%m%d').zip \
	     $(shell basename $(EDGETPU_RUNTIME_DIR)))

clean:
	rm -rf $(OUT_DIR) $(DIST_DIR) bazel-*

################################################################################
# Docker commands
################################################################################
DOCKER_CONTEXT_DIR := $(MAKEFILE_DIR)/docker
DOCKER_WORKSPACE := $(MAKEFILE_DIR)
DOCKER_CONTAINER_WORKSPACE := /workspace
DOCKER_CPUS ?= k8 armv7a aarch64
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
