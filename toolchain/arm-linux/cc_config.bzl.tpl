load("@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
    "action_config",
    "artifact_name_pattern",
    "env_entry",
    "env_set",
    "feature",
    "feature_set",
    "flag_group",
    "flag_set",
    "make_variable",
    "tool",
    "tool_path",
    "variable_with_value",
    "with_feature_set",
)
load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")

def _impl(ctx):
    if (ctx.attr.cpu == "armv6"):
        toolchain_identifier = "armv6-linux-gnueabihf"
        host_system_name = "armv6"
        target_system_name = "armv6"
        target_cpu = "armv6"
        target_libc = "armv6"
        abi_version = "armv6"
        abi_libc_version = "armv6"
    else:
        fail("Unreachable")

    compiler = "compiler"

    cc_target_os = None

    builtin_sysroot = None

    all_compile_actions = [
        ACTION_NAMES.c_compile,
        ACTION_NAMES.cpp_compile,
        ACTION_NAMES.linkstamp_compile,
        ACTION_NAMES.assemble,
        ACTION_NAMES.preprocess_assemble,
        ACTION_NAMES.cpp_header_parsing,
        ACTION_NAMES.cpp_module_compile,
        ACTION_NAMES.cpp_module_codegen,
        ACTION_NAMES.clif_match,
        ACTION_NAMES.lto_backend,
    ]

    all_cpp_compile_actions = [
        ACTION_NAMES.cpp_compile,
        ACTION_NAMES.linkstamp_compile,
        ACTION_NAMES.cpp_header_parsing,
        ACTION_NAMES.cpp_module_compile,
        ACTION_NAMES.cpp_module_codegen,
        ACTION_NAMES.clif_match,
    ]

    preprocessor_compile_actions = [
        ACTION_NAMES.c_compile,
        ACTION_NAMES.cpp_compile,
        ACTION_NAMES.linkstamp_compile,
        ACTION_NAMES.preprocess_assemble,
        ACTION_NAMES.cpp_header_parsing,
        ACTION_NAMES.cpp_module_compile,
        ACTION_NAMES.clif_match,
    ]

    codegen_compile_actions = [
        ACTION_NAMES.c_compile,
        ACTION_NAMES.cpp_compile,
        ACTION_NAMES.linkstamp_compile,
        ACTION_NAMES.assemble,
        ACTION_NAMES.preprocess_assemble,
        ACTION_NAMES.cpp_module_codegen,
        ACTION_NAMES.lto_backend,
    ]

    all_link_actions = [
        ACTION_NAMES.cpp_link_executable,
        ACTION_NAMES.cpp_link_dynamic_library,
        ACTION_NAMES.cpp_link_nodeps_dynamic_library,
    ]

    if (ctx.attr.cpu == "armv6"):
        action_configs = []
    else:
        fail("Unreachable")

    opt_feature = feature(name = "opt")

    dbg_feature = feature(name = "dbg")

    sysroot_feature = feature(
        name = "sysroot",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = [
                    ACTION_NAMES.preprocess_assemble,
                    ACTION_NAMES.linkstamp_compile,
                    ACTION_NAMES.c_compile,
                    ACTION_NAMES.cpp_compile,
                    ACTION_NAMES.cpp_header_parsing,
                    ACTION_NAMES.cpp_module_compile,
                    ACTION_NAMES.cpp_module_codegen,
                    ACTION_NAMES.lto_backend,
                    ACTION_NAMES.clif_match,
                    ACTION_NAMES.cpp_link_executable,
                    ACTION_NAMES.cpp_link_dynamic_library,
                    ACTION_NAMES.cpp_link_nodeps_dynamic_library,
                ],
                flag_groups = [
                    flag_group(
                        flags = ["--sysroot=%{sysroot}"],
                        expand_if_available = "sysroot",
                    ),
                ],
            ),
        ],
    )

    if (ctx.attr.cpu == "armv6"):
        unfiltered_compile_flags_feature = feature(
            name = "unfiltered_compile_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = [
                        ACTION_NAMES.assemble,
                        ACTION_NAMES.preprocess_assemble,
                        ACTION_NAMES.linkstamp_compile,
                        ACTION_NAMES.c_compile,
                        ACTION_NAMES.cpp_compile,
                        ACTION_NAMES.cpp_header_parsing,
                        ACTION_NAMES.cpp_module_compile,
                        ACTION_NAMES.cpp_module_codegen,
                        ACTION_NAMES.lto_backend,
                        ACTION_NAMES.clif_match,
                    ],
                    flag_groups = [
                        flag_group(
                            flags = [
                                "-Wno-builtin-macro-redefined",
                                "-D__DATE__=\"redacted\"",
                                "-D__TIMESTAMP__=\"redacted\"",
                                "-D__TIME__=\"redacted\"",
                                "-no-canonical-prefixes",
                                "-fno-canonical-system-headers",
                            ],
                        ),
                    ],
                ),
            ],
        )
    else:
        unfiltered_compile_flags_feature = None

    if (ctx.attr.cpu == "armv6"):
        default_compile_flags_feature = feature(
            name = "default_compile_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = [
                        ACTION_NAMES.assemble,
                        ACTION_NAMES.preprocess_assemble,
                        ACTION_NAMES.linkstamp_compile,
                        ACTION_NAMES.c_compile,
                        ACTION_NAMES.cpp_compile,
                        ACTION_NAMES.cpp_header_parsing,
                        ACTION_NAMES.cpp_module_compile,
                        ACTION_NAMES.cpp_module_codegen,
                        ACTION_NAMES.lto_backend,
                        ACTION_NAMES.clif_match,
                    ],
                    flag_groups = [
                        flag_group(
                            flags = [
                                "-fstack-protector",
                            ],
                        ),
                    ],
                ),
                flag_set(
                    actions = [
                        ACTION_NAMES.assemble,
                        ACTION_NAMES.preprocess_assemble,
                        ACTION_NAMES.linkstamp_compile,
                        ACTION_NAMES.c_compile,
                        ACTION_NAMES.cpp_compile,
                        ACTION_NAMES.cpp_header_parsing,
                        ACTION_NAMES.cpp_module_compile,
                        ACTION_NAMES.cpp_module_codegen,
                        ACTION_NAMES.lto_backend,
                        ACTION_NAMES.clif_match,
                    ],
                    flag_groups = [flag_group(flags = ["-g"])],
                    with_features = [with_feature_set(features = ["dbg"])],
                ),
                flag_set(
                    actions = [
                        ACTION_NAMES.assemble,
                        ACTION_NAMES.preprocess_assemble,
                        ACTION_NAMES.linkstamp_compile,
                        ACTION_NAMES.c_compile,
                        ACTION_NAMES.cpp_compile,
                        ACTION_NAMES.cpp_header_parsing,
                        ACTION_NAMES.cpp_module_compile,
                        ACTION_NAMES.cpp_module_codegen,
                        ACTION_NAMES.lto_backend,
                        ACTION_NAMES.clif_match,
                    ],
                    flag_groups = [
                        flag_group(
                            flags = [
                                "-g0",
                                "-O2",
                                "-DNDEBUG",
                                "-ffunction-sections",
                                "-fdata-sections",
                            ],
                        ),
                    ],
                    with_features = [with_feature_set(features = ["opt"])],
                ),
                flag_set(
                    actions = [
                        ACTION_NAMES.linkstamp_compile,
                        ACTION_NAMES.cpp_compile,
                        ACTION_NAMES.cpp_header_parsing,
                        ACTION_NAMES.cpp_module_compile,
                        ACTION_NAMES.cpp_module_codegen,
                        ACTION_NAMES.lto_backend,
                        ACTION_NAMES.clif_match,
                    ],
                    flag_groups = [
                        flag_group(
                            flags = [
                                "-isystem",
                                "%{ARMHF_COMPILER_PATH}%/lib/gcc/%{ARMHF_COMPILER_PREFIX}%/%{ARMHF_COMPILER_VERSION}%/include",
                                "-isystem",
                                "%{ARMHF_COMPILER_PATH}%/lib/gcc/%{ARMHF_COMPILER_PREFIX}%/%{ARMHF_COMPILER_VERSION}%/include-fixed",
                                "-isystem",
                                "%{ARMHF_COMPILER_PATH}%/%{ARMHF_COMPILER_PREFIX}%/include/c++/%{ARMHF_COMPILER_VERSION}%/",
                                "-isystem",
                                "%{ARMHF_COMPILER_PATH}%/%{ARMHF_COMPILER_PREFIX}%/libc/usr/include/",
                                "-isystem",
                                "%{ARMHF_COMPILER_PATH}%/%{ARMHF_COMPILER_PREFIX}%/sysroot/usr/include",
                            ],
                        ),
                    ],
                ),
            ],
        )
    else:
        default_compile_flags_feature = None

    if (ctx.attr.cpu == "armv6"):
        default_link_flags_feature = feature(
            name = "default_link_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = all_link_actions,
                    flag_groups = [
                        flag_group(
                            flags = [
                                "-lstdc++",
                                "-Wl,-z,relro,-z,now",
                                "-no-canonical-prefixes",
                                "-pass-exit-codes",
                                "-Wl,--build-id=md5",
                                "-Wl,--hash-style=gnu",
                            ],
                        ),
                    ],
                ),
                flag_set(
                    actions = all_link_actions,
                    flag_groups = [flag_group(flags = ["-Wl,--gc-sections"])],
                    with_features = [with_feature_set(features = ["opt"])],
                ),
            ],
        )
    else:
        default_link_flags_feature = None

    supports_dynamic_linker_feature = feature(name = "supports_dynamic_linker", enabled = True)

    supports_pic_feature = feature(name = "supports_pic", enabled = True)

    user_compile_flags_feature = feature(
        name = "user_compile_flags",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = [
                    ACTION_NAMES.assemble,
                    ACTION_NAMES.preprocess_assemble,
                    ACTION_NAMES.linkstamp_compile,
                    ACTION_NAMES.c_compile,
                    ACTION_NAMES.cpp_compile,
                    ACTION_NAMES.cpp_header_parsing,
                    ACTION_NAMES.cpp_module_compile,
                    ACTION_NAMES.cpp_module_codegen,
                    ACTION_NAMES.lto_backend,
                    ACTION_NAMES.clif_match,
                ],
                flag_groups = [
                    flag_group(
                        flags = ["%{user_compile_flags}"],
                        iterate_over = "user_compile_flags",
                        expand_if_available = "user_compile_flags",
                    ),
                ],
            ),
        ],
    )

    if (ctx.attr.cpu == "armv6"):
        features = [
                default_compile_flags_feature,
                default_link_flags_feature,
                supports_dynamic_linker_feature,
                supports_pic_feature,
                opt_feature,
                dbg_feature,
                user_compile_flags_feature,
                sysroot_feature,
                unfiltered_compile_flags_feature,
            ]
    else:
        fail("Unreachable")

    if (ctx.attr.cpu == "armv6"):
        cxx_builtin_include_directories = [
            "%{ARMHF_COMPILER_PATH}%/lib/gcc/%{ARMHF_COMPILER_PREFIX}%/%{ARMHF_COMPILER_VERSION}%/include",
            "%{ARMHF_COMPILER_PATH}%/lib/gcc/%{ARMHF_COMPILER_PREFIX}%/%{ARMHF_COMPILER_VERSION}%/include-fixed",
            "%{ARMHF_COMPILER_PATH}%/%{ARMHF_COMPILER_PREFIX}%/include/c++/%{ARMHF_COMPILER_VERSION}%",
            "%{ARMHF_COMPILER_PATH}%/%{ARMHF_COMPILER_PREFIX}%/libc/usr/include",
            "%{ARMHF_COMPILER_PATH}%/%{ARMHF_COMPILER_PREFIX}%/sysroot/usr/include",
            "%{ARMHF_COMPILER_PATH}%/%{ARMHF_COMPILER_PREFIX}%/sysroot",
        ]
    else:
        fail("Unreachable")

    artifact_name_patterns = []

    make_variables = []

    if (ctx.attr.cpu == "armv6"):
        tool_paths = [
            tool_path(
                name = "ar",
                path = "%{ARMHF_COMPILER_PATH}%/bin/%{ARMHF_COMPILER_PREFIX}%-ar",
            ),
            tool_path(name = "compat-ld", path = "/bin/false"),
            tool_path(
                name = "cpp",
                path = "%{ARMHF_COMPILER_PATH}%/bin/%{ARMHF_COMPILER_PREFIX}%-cpp",
            ),
            tool_path(
                name = "dwp",
                path = "%{ARMHF_COMPILER_PATH}%/bin/%{ARMHF_COMPILER_PREFIX}%-dwp",
            ),
            tool_path(
                name = "gcc",
                path = "%{ARMHF_COMPILER_PATH}%/bin/%{ARMHF_COMPILER_PREFIX}%-gcc",
            ),
            tool_path(
                name = "gcov",
                path = "%{ARMHF_COMPILER_PATH}%/bin/%{ARMHF_COMPILER_PREFIX}%-gcov",
            ),
            tool_path(
                name = "ld",
                path = "%{ARMHF_COMPILER_PATH}%/bin/%{ARMHF_COMPILER_PREFIX}%-ld",
            ),
            tool_path(
                name = "nm",
                path = "%{ARMHF_COMPILER_PATH}%/bin/%{ARMHF_COMPILER_PREFIX}%-nm",
            ),
            tool_path(
                name = "objcopy",
                path = "%{ARMHF_COMPILER_PATH}%/bin/%{ARMHF_COMPILER_PREFIX}%-objcopy",
            ),
            tool_path(
                name = "objdump",
                path = "%{ARMHF_COMPILER_PATH}%/bin/%{ARMHF_COMPILER_PREFIX}%-objdump",
            ),
            tool_path(
                name = "strip",
                path = "%{ARMHF_COMPILER_PATH}%/bin/%{ARMHF_COMPILER_PREFIX}%-strip",
            ),
        ]
    else:
        fail("Unreachable")

    out = ctx.actions.declare_file(ctx.label.name)
    ctx.actions.write(out, "Fake executable")
    return [
        cc_common.create_cc_toolchain_config_info(
            ctx = ctx,
            features = features,
            action_configs = action_configs,
            artifact_name_patterns = artifact_name_patterns,
            cxx_builtin_include_directories = cxx_builtin_include_directories,
            toolchain_identifier = toolchain_identifier,
            host_system_name = host_system_name,
            target_system_name = target_system_name,
            target_cpu = target_cpu,
            target_libc = target_libc,
            compiler = compiler,
            abi_version = abi_version,
            abi_libc_version = abi_libc_version,
            tool_paths = tool_paths,
            make_variables = make_variables,
            builtin_sysroot = builtin_sysroot,
            cc_target_os = cc_target_os
        ),
        DefaultInfo(
            executable = out,
        ),
    ]
cc_toolchain_config =  rule(
    implementation = _impl,
    attrs = {
        "cpu": attr.string(mandatory=True, values=["armv6"]),
    },
    provides = [CcToolchainConfigInfo],
    executable = True,
)
