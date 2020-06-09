"""Utilities for darwinn."""

def darwinn_port_defines():
    """Generates a list of port defines suitable for the build.

    Returns:
      List of defines.
    """
    return select({
        "//:darwinn_portable": ["DARWINN_PORT_DEFAULT"],
        "//:darwinn_firmware": ["DARWINN_PORT_FIRMWARE"],
        "//conditions:default": ["DARWINN_PORT_GOOGLE3"],
    })
