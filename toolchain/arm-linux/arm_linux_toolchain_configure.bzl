"""Repository rule for ARM cross compiler autoconfiguration."""

def _tpl(repository_ctx, tpl, substitutions = {}, out = None):
    if not out:
        out = tpl
    repository_ctx.template(
        out,
        Label("//toolchain/arm-linux:%s.tpl" % tpl),
        substitutions,
    )

def _arm_linux_toolchain_configure_impl(repository_ctx):
    # We need to find a cross-compilation include directory for Python, so look
    # for an environment variable. Be warned, this crosstool template is only
    # regenerated on the first run of Bazel, so if you change the variable after
    # it may not be reflected in later builds. Doing a shutdown and clean of Bazel
    # doesn't fix this, you'll need to delete the generated file at something like:
    # external/local_config_arm_compiler/CROSSTOOL in your Bazel install.
    _tpl(repository_ctx, "cc_config.bzl", {
        "%{ARMHF_COMPILER_PATH}%": str(repository_ctx.path(
            repository_ctx.attr.armhf_repo,
        )),
        "%{ARMHF_COMPILER_PREFIX}%": str(repository_ctx.attr.armhf_prefix),
        "%{ARMHF_COMPILER_VERSION}%": str(repository_ctx.attr.armhf_version),
    })
    repository_ctx.symlink(Label(repository_ctx.attr.build_file), "BUILD")

my_arm_linux_toolchain_configure = repository_rule(
    implementation = _arm_linux_toolchain_configure_impl,
    attrs = {
        "armhf_repo": attr.string(mandatory = True, default = ""),
        "armhf_version": attr.string(mandatory = True, default = ""),
        "armhf_prefix": attr.string(mandatory = True, default = ""),
        "build_file": attr.string(),
    },
)
