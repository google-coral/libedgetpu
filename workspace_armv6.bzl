load("//toolchain/arm-linux:arm_linux_toolchain_configure.bzl", "my_arm_linux_toolchain_configure")

def _get_link_dict(ctx, link_files, build_file):
    link_dict = {ctx.path(v): ctx.path(Label(k)) for k, v in link_files.items()}
    if build_file:
        # Use BUILD.bazel because it takes precedence over BUILD.
        link_dict[ctx.path("BUILD.bazel")] = ctx.path(Label(build_file))
    return link_dict

def _cc_http_archive_impl(ctx):
    # Construct all paths early on to prevent rule restart. We want the
    # attributes to be strings instead of labels because they refer to files
    # in the TensorFlow repository, not files in repos depending on TensorFlow.
    # See also https://github.com/bazelbuild/bazel/issues/10515.
    link_dict = _get_link_dict(ctx, ctx.attr.link_files, ctx.attr.build_file)

    ctx.download_and_extract(
        url = ctx.attr.urls,
        sha256 = ctx.attr.sha256,
        type = ctx.attr.type,
        stripPrefix = ctx.attr.strip_prefix,
    )

    for dst, src in link_dict.items():
        ctx.delete(dst)
        ctx.symlink(src, dst)

_cc_http_archive = repository_rule(
    implementation = _cc_http_archive_impl,
    attrs = {
        "sha256": attr.string(mandatory = True),
        "urls": attr.string_list(mandatory = True),
        "strip_prefix": attr.string(),
        "build_file": attr.string(),
        "link_files": attr.string_dict(),
        "type": attr.string(),
    },
)

def cc_http_archive(name, sha256, urls, **kwargs):
    _cc_http_archive(
        name = name,
        sha256 = sha256,
        urls = urls,
        **kwargs
    )

def _cc_repositories():
    cc_http_archive(
        name = "my_armv6_linux_toolchain",
        build_file = "//toolchain/arm-linux:armhf-linux-toolchain.BUILD",
        sha256 = "8590e11447307ec54c813ba471d109618e581985bdba0a8c21ecf9d52367ecc6",
        strip_prefix = "nerves_toolchain_armv6_nerves_linux_gnueabihf-linux_x86_64-1.8.0",
        urls = ["https://github.com/nerves-project/toolchains/releases/download/v1.8.0/nerves_toolchain_armv6_nerves_linux_gnueabihf-linux_x86_64-1.8.0-BC50D6D.tar.xz"],
    )

def _arm_toolchain():
    native.register_execution_platforms("@local_execution_config_platform//:platform")
    my_arm_linux_toolchain_configure(
        name = "my_local_config_embedded_arm",
        build_file = "//toolchain/arm-linux:template.BUILD",
        armhf_repo = "../my_armv6_linux_toolchain",
        armhf_version = "12.2.0",
        armhf_prefix = "armv6-nerves-linux-gnueabihf",
    )

def workspace():
    _arm_toolchain()
    _cc_repositories()

workspace_armv6 = workspace
