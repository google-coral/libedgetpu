# Edge TPU runtime library (libedgetpu)

This repo contains the source code for the userspace
level runtime driver for [Coral devices](https://coral.ai/products).
This software is distributed in the binary form at [coral.ai/software](https://coral.ai/software/).

## Building

At present only Bazel build system is supported, but it can be invoked from the Makefile. 

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
