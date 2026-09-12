#!/usr/bin/env python3
#
# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
"""Assert that a build which asks for nothing gets every feature turned off.

Reads the compile_commands.json of the `minimum` sample -- which includes
build-scripts/runtime_lib.cmake directly and sets WAMR_BUILD_INTERP only -- and
fails if any feature macro came out non-zero.

    cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    python3 check_defaults.py build/compile_commands.json
"""

import json
import re
import sys

# Macros which are legitimately non-zero in a minimum build:
#   - the running mode the sample explicitly asks for
#   - values that are sizes, not switches
#   - values derived from the host platform
ALLOWED_NON_ZERO = {
    "WASM_ENABLE_INTERP",
    "WASM_ENABLE_LOG",
    "WASM_GLOBAL_HEAP_SIZE",
    "WASM_HAVE_MREMAP",
}

MACRO = re.compile(r"-D(WASM_[A-Z0-9_]+|BH_ENABLE_[A-Z0-9_]+)=(\d+)")


def main(path: str) -> int:
    with open(path, encoding="utf-8") as f:
        entries = json.load(f)

    values = {}
    for entry in entries:
        for name, value in MACRO.findall(entry.get("command", "")):
            values[name] = value

    unexpected = sorted(
        f"{name}={value}"
        for name, value in values.items()
        if value != "0" and name not in ALLOWED_NON_ZERO
    )

    if unexpected:
        print("A minimum build should have every feature off, but found:")
        for item in unexpected:
            print(f"  {item}")
        return 1

    print(f"minimum build: {len(values)} macros, all off except the expected ones")
    return 0


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(2)
    sys.exit(main(sys.argv[1]))
