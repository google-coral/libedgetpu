"""
This module defines rules to set copts and linkopts for
libraries and modules in the libedgetpu project.
"""

# This allows the dependencies to work correctly
# if we are included as a submodule.
def clean_dep(dep):
    return str(Label(dep))

WINDOWS_COPTS = [
    "/DSTRIP_LOG=1",
    "/DABSL_FLAGS_STRIP_NAMES",
    "/D_HAS_DEPRECATED_RESULT_OF",
    "/D_HAS_DEPRECATED_ADAPTOR_TYPEDEFS",
    "/GR-",
    "/DWIN32_LEAN_AND_MEAN",
    "/D_WINSOCKAPI_",
]
WINDOWS_LINKOPTS = []
WINDOWS_OPT_LINKOPTS = []

DARWIN_COPTS = [
    "-fvisibility=hidden",
]
DARWIN_LINKOPTS = [
    "-L/opt/local/lib",
    "-lusb-1.0",
]
DARWIN_OPT_LINKOPTS = []

LINUX_COPTS = []
LINUX_LINKOPTS = [
    "-l:libusb-1.0.so",
]
LINUX_OPT_LINKOPTS = [
    "-Wl,--strip-all",
]

COMMON_COPTS = [
    "-DSTRIP_LOG=1",
    "-fno-rtti",
    "-fno-exceptions",
    '-D__FILE__=\\"\\"',
]
COMMON_LINKOPTS = []
COMMON_OPT_LINKOPTS = []

def libedgetpu_copts():
    return select({
        clean_dep("//:windows"): WINDOWS_COPTS,
        clean_dep("//:darwin"): DARWIN_COPTS + COMMON_COPTS,
        "//conditions:default": LINUX_COPTS + COMMON_COPTS,
    })

def libedgetpu_linkopts():
    return select({
        clean_dep("//:windows"): WINDOWS_LINKOPTS,
        clean_dep("//:darwin"): DARWIN_LINKOPTS + COMMON_LINKOPTS,
        "//conditions:default": LINUX_LINKOPTS + COMMON_LINKOPTS,
    })

def libedgetpu_opt_linkopts():
    return select({
        clean_dep("//:windows"): WINDOWS_OPT_LINKOPTS,
        clean_dep("//:darwin"): DARWIN_OPT_LINKOPTS + COMMON_OPT_LINKOPTS,
        "//conditions:default": LINUX_OPT_LINKOPTS + COMMON_OPT_LINKOPTS,
    })

def libedgetpu_cc_library(name, copts = [], **attrs):
    native.cc_library(
        name = name + "_opt",
        copts = copts + libedgetpu_copts(),
        **attrs
    )
    native.cc_library(
        name = name + "_default",
        copts = copts + libedgetpu_copts(),
        **attrs
    )
    native.cc_library(
        name = name,
        deps = select({
            clean_dep("//:opt"): [name + "_opt"],
            "//conditions:default": [name + "_default"],
        }),
    )

def libedgetpu_cc_binary(name, copts = [], linkopts = [], deps = [], additional_linker_inputs = [], linkshared = 0, **attrs):
    native.cc_library(
        name = name + "_opt",
        copts = copts + libedgetpu_copts(),
        linkopts = libedgetpu_linkopts() + libedgetpu_opt_linkopts(),
        deps = deps,
        **attrs
    )
    native.cc_library(
        name = name + "_default",
        copts = copts + libedgetpu_copts(),
        linkopts = libedgetpu_linkopts(),
        deps = deps,
        **attrs
    )
    native.cc_binary(
        name = name,
        deps = select({
            clean_dep("//:opt"): [name + "_opt"],
            "//conditions:default": [name + "_default"],
        }) + deps,
        linkopts = linkopts,
        additional_linker_inputs = additional_linker_inputs,
        linkshared = linkshared,
        **attrs
    )
