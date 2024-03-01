"""
This module contains workspace definitions for building and using libedgetpu.
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

# TF release 2.15.0 as of Nov 15, 2023
#
# TENSORFLOW_SHA256=$(curl -fSL "https://github.com/tensorflow/tensorflow/archive/${TENSORFLOW_COMMIT}.tar.gz" | sha256sum)
TENSORFLOW_COMMIT = "6887368d6d46223f460358323c4b76d61d1558a8"
TENSORFLOW_SHA256 = "bb25fa4574e42ea4d452979e1d2ba3b86b39569d6b8106a846a238b880d73652"

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
      coral_crosstool_commit: https://github.com/cocoa-xu/crosstool commit ID
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

    # Use bazel to query values defined here, e.g.:
    #     bazel query "@libedgetpu_properties//..." | grep tensorflow_commit | cut -d\# -f2
    _properties_repository(
        name = "libedgetpu_properties",
        properties = {
            "tensorflow_commit": tensorflow_commit,
            "coral_crosstool_commit": coral_crosstool_commit,
        },
    )

def _brew_libusb_path(ctx):
    res = ctx.execute(["/bin/bash", "-c", "command -v brew"])
    if res.return_code != 0:
        return None
    brew = ctx.path(res.stdout.strip())

    res = ctx.execute([brew, "ls", "--versions", "libusb"])
    if res.return_code != 0:
        return None

    res = ctx.execute([brew, "--prefix", "libusb"])
    if res.return_code != 0:
        return None

    return res.stdout.strip()

def _port_libusb_path(ctx):
    res = ctx.execute(["/bin/bash", "-c", "command -v port"])
    if res.return_code != 0:
        return None
    port = ctx.path(res.stdout.strip())

    res = ctx.execute([port, "info", "libusb"])
    if res.return_code != 0:
        return None

    return port.dirname.dirname

def _pkg_config_libusb_path(ctx):
    res = ctx.execute(["/bin/bash", "-c", "which pkg-config"])
    if res.return_code != 0:
        return None
    pkg_config = ctx.path(res.stdout.strip())

    res = ctx.execute([pkg_config, "--cflags-only-I", "libusb-1.0"])
    if res.return_code != 0:
        return None

    return res.stdout.strip()[2:-19]

def _libusb_impl(ctx):
    lower_name = ctx.os.name.lower()
    if lower_name.startswith("linux"):
        path = _pkg_config_libusb_path(ctx)
        build_file_content = """
cc_library(
  name = "headers",
  srcs = ["root/lib/libusb-1.0.so"],
  visibility = ["//visibility:public"],
  includes = ["root/include"],
  hdrs = ["root/include/libusb-1.0/libusb.h"],
)
"""
    elif lower_name.startswith("windows"):
        path = str(ctx.path(Label("@//:WORKSPACE"))) + "/../../libusb"
        build_file_content = """
cc_library(
  name = "headers",
  includes = ["root/include"],
  hdrs = ["root/include/libusb-1.0/libusb.h"],
  visibility = ["//visibility:public"],
)
cc_import(
  name = "shared",
  interface_library = "root/VS2019/MS64/dll/libusb-1.0.lib",
  shared_library = "root/VS2019/MS64/dll/libusb-1.0.dll",
  visibility = ["//visibility:public"],
)
"""
    elif lower_name.startswith("mac os x"):
        path = _brew_libusb_path(ctx)
        if not path:
            path = _port_libusb_path(ctx)
            if not path:
                fail("Install libusb using MacPorts or Homebrew.")

        build_file_content = """
cc_library(
  name = "headers",
  includes = ["root/include"],
  hdrs = ["root/include/libusb-1.0/libusb.h"],
  linkopts = ["-framework", "IOKit", "-framework", "Security"],
  srcs = ["root/lib/libusb-1.0.a"],
  visibility = ["//visibility:public"],
)
"""
    else:
        fail("Unsupported operating system.")

    if path:
        ctx.symlink(path, "root")
    ctx.file(
        "BUILD",
        content = build_file_content,
        executable = False,
    )

libusb_repository = repository_rule(
    implementation = _libusb_impl,
)

def _properties_impl(ctx):
    content = ""
    for name, value in ctx.attr.properties.items():
        content += "filegroup(name='" + name + "#" + value + "')\n"
    ctx.file("BUILD", content = content, executable = False)

_properties_repository = repository_rule(
    implementation = _properties_impl,
    attrs = {
        "properties": attr.string_dict(),
    },
)
