"""
This module contains workspace definitions for building and using libedgetpu.
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

TENSORFLOW_COMMIT = "855c4c0ee34257b98ce2d01121940efb5423a059"
TENSORFLOW_SHA256 = "855ded5cae5915c6f232d27342fa4fb666922798e02e15379b4fa67265695685"

CORAL_CROSSTOOL_COMMIT = "6bcc2261d9fc60dff386b557428d98917f0af491"
CORAL_CROSSTOOL_SHA256 = "38cb4da13009d07ebc2fed4a9d055b0f914191b344dd2d1ca5803096343958b4"

def libedgetpu_dependencies(
        tensorflow_commit = TENSORFLOW_COMMIT,
        tensorflow_sha256 = TENSORFLOW_SHA256,
        coral_crosstool_commit = CORAL_CROSSTOOL_COMMIT,
        coral_crosstool_sha256 = CORAL_CROSSTOOL_SHA256):
    """Sets up libedgetpu dependencies.

    Args:
      tensorflow_commit: https://github.com/tensorflow/tensorflow commit ID
      tensorflow_sha256: corresponding sha256 of the source archive
      io_bazel_rules_closure_commit: https://github.com/bazelbuild/rules_closure commit ID
      io_bazel_rules_closure_sha256: corresponding sha256 of the source archive
      coral_crosstool_commit: https://github.com/google-coral/crosstool commit ID
      coral_crosstool_sha256: corresponding sha256 of the source archive
    """
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
