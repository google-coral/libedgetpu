"""
This module contains workspace definitions for building and using libedgetpu.
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

# TF release 2.5.0 as of 05/17/2021.
#TENSORFLOW_COMMIT = "a4dfb8d1a71385bd6d122e4f27f86dcebb96712d"
#TENSORFLOW_SHA256 = "cb99f136dc5c89143669888a44bfdd134c086e1e2d9e36278c1eb0f03fe62d76"

# TF release 2.5.3 as of 05/17/2021.
#TENSORFLOW_COMMIT = "a4dfb8d1a71385bd6d122e4f27f86dcebb96712d"
#TENSORFLOW_SHA256 = "cb99f136dc5c89143669888a44bfdd134c086e1e2d9e36278c1eb0f03fe62d76"

# TF release 2.8.4 as of 11/14/2022.
#TENSORFLOW_COMMIT = "1b8f5c396f0c016ebe81fe1af029e6f205c926a4"
#TENSORFLOW_SHA256 = "2f44f6b57e065a545d16a64a428d91ec2006def421a6ea44940e159048f15f0c"

# TF release 2.9.3 as of 11/14/2023.
#TENSORFLOW_COMMIT = "a5ed5f39b675a1c6f315e0caf3ad4b38478fa571"
#TENSORFLOW_SHA256 = "cbed52f7f29fb4eaec3cb146f4aba4f622fdf3d3471c88cdf787428005a854c6"

# TF release 2.10.1 as of 11/14/2023.
TENSORFLOW_COMMIT = "fdfc646704c37bdf450525f6ced9d80df86e4993"
TENSORFLOW_SHA256 = "094320c39b502bcaa3412744517e21e3cd80f55db1371e6692dfe23eeb3cef04"

#CORAL_CROSSTOOL_COMMIT = "6bcc2261d9fc60dff386b557428d98917f0af491"
#CORAL_CROSSTOOL_SHA256 = "38cb4da13009d07ebc2fed4a9d055b0f914191b344dd2d1ca5803096343958b4"

# Crosstool release as of 02/28/2023
CORAL_CROSSTOOL_COMMIT = "8e885509123395299bed6a5f9529fdc1b9751599"
CORAL_CROSSTOOL_SHA256 = "f86d488ca353c5ee99187579fe408adb73e9f2bb1d69c6e3a42ffb904ce3ba01"

def libedgetpu_dependencies(
        tensorflow_commit = TENSORFLOW_COMMIT,
        tensorflow_sha256 = TENSORFLOW_SHA256,
        coral_crosstool_commit = CORAL_CROSSTOOL_COMMIT,
        coral_crosstool_sha256 = CORAL_CROSSTOOL_SHA256):
    """Sets up libedgetpu dependencies.

    Args:
      tensorflow_commit: https://github.com/tensorflow/tensorflow commit ID
      tensorflow_sha256: corresponding sha256 of the source archive
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

def _libusb_impl(ctx):
    lower_name = ctx.os.name.lower()
    if lower_name.startswith("linux"):
        path = None
        build_file_content = """
cc_library(
  name = "headers",
  linkopts = ["-l:libusb-1.0.so"],
  visibility = ["//visibility:public"],
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
  srcs = ["root/lib/libusb-1.0.dylib"],
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
