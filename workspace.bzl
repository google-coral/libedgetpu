"""
This module contains workspace definitions for building and using libedgetpu.
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

# TF release 2.7.0 
TENSORFLOW_COMMIT = "0db597d0d758aba578783b5bf46c889700a45085"
TENSORFLOW_SHA256 = "e97c242d13e386192e3a9f60fd674461b6595b02b3a2a48edd6fb11aeee7e038"

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
