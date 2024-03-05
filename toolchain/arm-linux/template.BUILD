load(":cc_config.bzl", "cc_toolchain_config")

package(default_visibility = ["//visibility:public"])

licenses(["restricted"])  # GPLv3

cc_toolchain_suite(
    name = "toolchain",
    toolchains = {
        "armv6": ":cc-compiler-armv6",
    },
)

filegroup(
    name = "empty",
    srcs = [],
)

filegroup(
    name = "my_armv6_toolchain_all_files",
    srcs = [
        "@my_armv6_linux_toolchain//:compiler_pieces",
    ],
)

cc_toolchain_config(
    name = "my_armv6_toolchain_config",
    cpu = "armv6",
)

cc_toolchain(
    name = "cc-compiler-armv6",
    all_files = ":my_armv6_toolchain_all_files",
    compiler_files = ":my_armv6_toolchain_all_files",
    dwp_files = ":empty",
    linker_files = ":my_armv6_toolchain_all_files",
    objcopy_files = "my_armv6_toolchain_all_files",
    strip_files = "my_armv6_toolchain_all_files",
    supports_param_files = 1,
    toolchain_config = ":my_armv6_toolchain_config",
)
