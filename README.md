# Edge TPU runtime library (libedgetpu)

This repo contains the source code for the userspace
level runtime driver for [Coral devices](https://coral.ai/products).
This software is distributed in the binary form at [coral.ai/software](https://coral.ai/software/).

## Building

There are three ways to build libedgetpu:

* Docker + Bazel: Compatible with Linux, MacOS and Windows (via Dockerfile.windows and build.bat), this method ensures a known-good build enviroment and pulls all external depedencies needed.
* Bazel: Supports Linux, macOS, and Windows (via build.bat). A proper enviroment setup is required before using this technique.
* Makefile: Supporting only Linux and Native builds, this strategy is pure Makefile and doesn't require Bazel or external dependencies to be pulled at runtime.

### Bazel + Docker

Build Linux binaries inside Docker container (works on Linux and macOS):
```
$ DOCKER_CPUS="k8" DOCKER_IMAGE="ubuntu:18.04" DOCKER_TARGETS=libedgetpu make docker-build
$ DOCKER_CPUS="armv7a aarch64" DOCKER_IMAGE="debian:stretch" DOCKER_TARGETS=libedgetpu make docker-build
```

All built binaries go to the `out` directory. Note that the bazel-* are not copied to the host from the Docker container.

### Bazel [Recommended]

[outdated] For proper environment setup check `docker` directory, although the setup is outdated. 

This fork requires the use of `bazel` in the version recommended for the corresponding version of tensorflow. For example, it requires `Bazel 5.3.0` to compile TF 2.13.1.

Current version of tensorflow supported is 2.13.1.

Build native binaries on Linux and macOS:
```
$ make
```

Build native binaries on Windows:
```
$ build.bat
```

Cross-compile for ARMv7-A (32 bit), and ARMv8-A (64 bit) on Linux:
```
$ CPU=armv7a make
$ CPU=aarch64 make
```

To package a Debian deb:
```
debuild -us -uc -tc -b
```

### Makefile

If only building for native systems, it is possible to significantly reduce the complexity of the build by removing Bazel (and Docker). This simple approach builds only what is needed, removes build-time depenency fetching, increases the speed, and uses upstream Debian packages.

To prepare your system, you'll need the following packages (both available on Debian Bullseye or Buster-Backports):
```
sudo apt install libabsl-dev libflatbuffers-dev
```

Next, you'll need to clone the [Tensorflow Repo](https://github.com/tensorflow/tensorflow) at the desired checkout (using TF head isn't advised). If you are planning to use libcoral or pycoral libraries, this should match the ones in those repos' WORKSPACE files. For example, if you are using TF2.5, we can check that [tag in the TF Repo](https://github.com/tensorflow/tensorflow/commit/a4dfb8d1a71385bd6d122e4f27f86dcebb96712d) and then checkout that address:
```
git clone https://github.com/tensorflow/tensorflow
git checkout a4dfb8d1a71385bd6d122e4f27f86dcebb96712d -b tf2.5
```

To build the library:
```
TFROOT=<Directory of Tensorflow> make -f makefile_build/Makefile -j$(nproc) libedgetpu
```

## Support

If you have question, comments or requests concerning this library, please
reach out to coral-support@google.com.

## License

[Apache License 2.0](LICENSE)

## Warning

If you're using the Coral USB Accelerator, it may heat up during operation, depending
on the computation workloads and operating frequency. Touching the metal part of the USB
Accelerator after it has been operating for an extended period of time may lead to discomfort
and/or skin burns. As such, if you enable the Edge TPU runtime using the maximum operating
frequency, the USB Accelerator should be operated at an ambient temperature of 25°C or less.
Alternatively, if you enable the Edge TPU runtime using the reduced operating frequency, then
the device is intended to safely operate at an ambient temperature of 35°C or less.

Google does not accept any responsibility for any loss or damage if the device
is operated outside of the recommended ambient temperature range.

Note: This issue affects only USB-based Coral devices, and is irrelevant for PCIe devices.
