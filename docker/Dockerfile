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
ARG IMAGE
FROM ${IMAGE}

COPY update_sources.sh /
RUN /update_sources.sh

RUN dpkg --add-architecture armhf
RUN dpkg --add-architecture arm64
RUN apt-get update && apt-get install -y \
  python3-all \
  python3-numpy \
  build-essential \
  crossbuild-essential-armhf \
  crossbuild-essential-arm64 \
  libusb-1.0-0-dev \
  libusb-1.0-0-dev:arm64 \
  libusb-1.0-0-dev:armhf \
  zlib1g-dev \
  zlib1g-dev:armhf \
  zlib1g-dev:arm64 \
  sudo \
  pkg-config \
  zip \
  unzip \
  curl \
  wget \
  git \
  software-properties-common \
  $(grep Ubuntu /etc/os-release > /dev/null && echo vim-common || echo xxd)

# Bionic Beaver == Ubuntu 18.04
RUN if grep 'Bionic Beaver' /etc/os-release > /dev/null; then \
      add-apt-repository ppa:ubuntu-toolchain-r/test \
        && DEBIAN_FRONTEND=noninteractive apt-get install -y gcc-9 g++-9; \
    fi

RUN git clone https://github.com/raspberrypi/tools.git && \
    cd tools && \
    git reset --hard 4a335520900ce55e251ac4f420f52bf0b2ab6b1f

ARG BAZEL_VERSION=4.0.0
RUN wget -O /bazel https://github.com/bazelbuild/bazel/releases/download/${BAZEL_VERSION}/bazel-${BAZEL_VERSION}-installer-linux-x86_64.sh && \
    bash /bazel && \
    rm -f /bazel
