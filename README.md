# Edge TPU runtime library (libedgetpu)

This repo contains the source code for the userspace
level runtime driver for [Coral devices](https://coral.ai/products).
This software is distributed in the binary form at [coral.ai/software](https://coral.ai/software/).

## Building

At present only [Bazel](https://bazel.build/) build system is supported. For
proper environment setup check `docker` directory.

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

Build Linux binaries inside Docker container (works on Linux and macOS):
```
$ DOCKER_CPUS="k8" DOCKER_IMAGE="ubuntu:18.04" DOCKER_TARGETS=libedgetpu make docker-build
$ DOCKER_CPUS="armv7a aarch64" DOCKER_IMAGE="debian:stretch" DOCKER_TARGETS=libedgetpu make docker-build
```

All built binaries go to the `out` directory.

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
