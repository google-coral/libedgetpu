"""
This module contains workspace definitions for building and using libedgetpu.
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

TENSORFLOW_COMMIT = "0b06f2927be226ffe44f47bfa9e03e4ea649d7f3"
TENSORFLOW_SHA256 = "098733e15e227a6a55997295ca761a9a8e7169b64104ea86feb952e4e2ea0bc9"

IO_BAZEL_RULES_CLOSURE_COMMIT = "308b05b2419edb5c8ee0471b67a40403df940149"
IO_BAZEL_RULES_CLOSURE_SHA256 = "5b00383d08dd71f28503736db0500b6fb4dda47489ff5fc6bed42557c07c6ba9"

CORAL_CROSSTOOL_COMMIT = "9e00d5be43bf001f883b5700f5d04882fea00229"
CORAL_CROSSTOOL_SHA256 = "cb31b1417ccdcf7dd9fca5ec63e1571672372c30427730255997a547569d2feb"

def libedgetpu_dependencies(
        tensorflow_commit = TENSORFLOW_COMMIT,
        tensorflow_sha256 = TENSORFLOW_SHA256,
        io_bazel_rules_closure_commit = IO_BAZEL_RULES_CLOSURE_COMMIT,
        io_bazel_rules_closure_sha256 = IO_BAZEL_RULES_CLOSURE_SHA256,
        coral_crosstool_commit = CORAL_CROSSTOOL_COMMIT,
        coral_crosstool_sha256 = CORAL_CROSSTOOL_SHA256):
    maybe(
        http_archive,
        name = "org_tensorflow",
        urls = [
            "https://github.com/tensorflow/tensorflow/archive/" + tensorflow_commit + ".tar.gz",
        ],
        sha256 = tensorflow_sha256,
        strip_prefix = "tensorflow-" + tensorflow_commit,
    )

    maybe(
        http_archive,
        name = "io_bazel_rules_closure",
        urls = [
            "https://github.com/bazelbuild/rules_closure/archive/" + io_bazel_rules_closure_commit + ".tar.gz",  # 2019-06-13
        ],
        sha256 = io_bazel_rules_closure_sha256,
        strip_prefix = "rules_closure-" + io_bazel_rules_closure_commit,
    )

    maybe(
        http_archive,
        name = "coral_crosstool",
        urls = [
            "https://github.com/google-coral/crosstool/archive/" + coral_crosstool_commit + ".tar.gz",
        ],
        sha256 = coral_crosstool_sha256,
        strip_prefix = "crosstool-" + coral_crosstool_commit,
    )

    maybe(
        libusb_repository,
        name = "libusb",
    )

def _libusb_impl(ctx):
    lower_name = ctx.os.name.lower()
    if lower_name.startswith("linux"):
        path = "/usr/include"
        build_file_content = """
cc_library(
  name = "headers",
  includes = ["root"],
  hdrs = ["root/libusb-1.0/libusb.h"],
  visibility = ["//visibility:public"],
)
"""
    elif lower_name.startswith("windows"):
        path = str(ctx.path(Label("@//:WORKSPACE"))) + "/../../libusb-1.0.22"
        build_file_content = """
cc_library(
  name = "headers",
  includes = ["root/include"],
  hdrs = ["root/include/libusb-1.0/libusb.h"],
  visibility = ["//visibility:public"],
)
cc_import(
  name = "shared",
  interface_library = "root/MS64/dll/libusb-1.0.lib",
  shared_library = "root/MS64/dll/libusb-1.0.dll",
  visibility = ["//visibility:public"],
)
"""
    elif lower_name.startswith("mac os x"):
        path = "/opt/local/include/"
        build_file_content = """
cc_library(
  name = "headers",
  includes = ["root"],
  hdrs = ["root/libusb-1.0/libusb.h"],
  visibility = ["//visibility:public"],
)
"""
    else:
        fail("Unsupported operating system.")

    ctx.symlink(path, "root")
    ctx.file(
        "BUILD",
        content = build_file_content,
        executable = False,
    )

libusb_repository = repository_rule(
    implementation = _libusb_impl,
)
