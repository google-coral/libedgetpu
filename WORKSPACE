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
workspace(name = "libedgetpu")

load(":workspace.bzl", "libedgetpu_dependencies")
libedgetpu_dependencies()

# ==================================================================

# We must initialize hermetic python first.
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bazel_skylib",
    sha256 = "74d544d96f4a5bb630d465ca8bbcfe231e3594e5aae57e1edbf17a6eb3ca2506",
    urls = [
        "https://storage.googleapis.com/mirror.tensorflow.org/github.com/bazelbuild/bazel-skylib/releases/download/1.3.0/bazel-skylib-1.3.0.tar.gz",
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.3.0/bazel-skylib-1.3.0.tar.gz",
    ],
)

http_archive(
    name = "rules_python",
    sha256 = "9d04041ac92a0985e344235f5d946f71ac543f1b1565f2cdbc9a2aaee8adf55b",
    strip_prefix = "rules_python-0.26.0",
    url = "https://github.com/bazelbuild/rules_python/releases/download/0.26.0/rules_python-0.26.0.tar.gz",
)

load("@rules_python//python:repositories.bzl", "py_repositories")

py_repositories()

load("@rules_python//python:repositories.bzl", "python_register_toolchains")
load(
    "@org_tensorflow//tensorflow/tools/toolchains/python:python_repo.bzl",
    "python_repository",
)

python_repository(name = "python_version_repo")

load("@python_version_repo//:py_version.bzl", "HERMETIC_PYTHON_VERSION")

python_register_toolchains(
    name = "python",
    ignore_root_user_error = True,
    python_version = HERMETIC_PYTHON_VERSION,
)

load("@python//:defs.bzl", "interpreter")
load("@rules_python//python:pip.bzl", "package_annotation", "pip_parse")

NUMPY_ANNOTATIONS = {
    "numpy": package_annotation(
        additive_build_content = """\
filegroup(
    name = "includes",
    srcs = glob(["site-packages/numpy/core/include/**/*.h"]),
)
cc_library(
    name = "numpy_headers",
    hdrs = [":includes"],
    strip_include_prefix="site-packages/numpy/core/include/",
)
""",
    ),
}

pip_parse(
    name = "pypi",
    annotations = NUMPY_ANNOTATIONS,
    python_interpreter_target = interpreter,
    requirements = "@org_tensorflow//:requirements_lock_" + HERMETIC_PYTHON_VERSION.replace(".", "_") + ".txt",
)

load("@pypi//:requirements.bzl", "install_deps")

install_deps()

# ==================================================================

load(":workspace.bzl", "libedgetpu_dependencies")
libedgetpu_dependencies()

load("@org_tensorflow//tensorflow:workspace3.bzl", "tf_workspace3")
tf_workspace3()

load("@org_tensorflow//tensorflow:workspace2.bzl", "tf_workspace2")
tf_workspace2()

load("@org_tensorflow//tensorflow:workspace1.bzl", "tf_workspace1")
tf_workspace1()

load("@org_tensorflow//tensorflow:workspace0.bzl", "tf_workspace0")
tf_workspace0()

load("@coral_crosstool//:configure.bzl", "cc_crosstool")
cc_crosstool(name = "crosstool", cpp_version = "c++14")

BAZEL_ZIG_CC_VERSION = "1.0.3"
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive") 

http_archive(
  name = "io_bazel_rules_closure",
  sha256 = "7d206c2383811f378a5ef03f4aacbcf5f47fd8650f6abbc3fa89f3a27dd8b176",
  strip_prefix = "rules_closure-0.10.0",
  urls = [
    "https://mirror.bazel.build/github.com/bazelbuild/rules_closure/archive/0.10.0.tar.gz",
    "https://github.com/bazelbuild/rules_closure/archive/0.10.0.tar.gz",
  ],
)

http_archive(
    name = "bazel-zig-cc",
    sha256 = "b1a20a76f75ad0585d12ca1260b3a9df1da0f19d1d602f59ecba040aeb3c6a27",
    strip_prefix = "bazel-zig-cc-{}".format(BAZEL_ZIG_CC_VERSION),
    urls = ["https://github.com/cocoa-xu/bazel-zig-cc/archive/refs/tags/v{}.tar.gz".format(BAZEL_ZIG_CC_VERSION)],
)

load("@bazel-zig-cc//toolchain:defs.bzl", zig_toolchains = "toolchains")

zig_toolchains(
    version = "0.10.1",
    url_formats = [
        "https://ziglang.org/download/{version}/zig-{host_platform}-{version}.tar.xz",
    ],
    host_platform_sha256 = {
        "linux-x86_64": "6699f0e7293081b42428f32c9d9c983854094bd15fee5489f12c4cf4518cc380",
        "linux-aarch64": "db0761664f5f22aa5bbd7442a1617dd696c076d5717ddefcc9d8b95278f71f5d",
        "macos-x86_64": "02483550b89d2a3070c2ed003357fd6e6a3059707b8ee3fbc0c67f83ca898437",
        "macos-aarch64": "b9b00477ec5fa1f1b89f35a7d2a58688e019910ab80a65eac2a7417162737656"
    },
)
