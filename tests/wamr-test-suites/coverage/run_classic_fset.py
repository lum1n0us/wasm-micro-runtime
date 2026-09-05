#!/usr/bin/env python3
#
# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""Classic-interp feature-set coverage invocation.

Runs one canned report object:
  * running mode: classic-interp
  * feature set F: the classic interpreter with the libc-builtin runtime,
    shared heap, global heap pool and custom-section loading, plus the
    spec-test exemptions (SPEC_TEST / BULK_MEMORY / REF_TYPES)
  * test set: spec + unit (compatible targets only)

Usage (from anywhere in the repository):
  python3 tests/wamr-test-suites/coverage/run_classic_fset.py \
      [--out DIR] [--llvm-dir DIR]
"""

import argparse
import os
import subprocess
import sys

COVERAGE_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(COVERAGE_DIR)))
RUN_COVERAGE = os.path.join(COVERAGE_DIR, "run_coverage.py")

# The report's feature set, spelled out the way --feature expects it.
FEATURE_SET = " ".join([
    "-DWAMR_BUILD_LIBC_BUILTIN=1",
    "-DWAMR_BUILD_SHARED_HEAP=1",
    "-DWAMR_BUILD_GLOBAL_HEAP_POOL=1",
    "-DWAMR_BUILD_LOAD_CUSTOM_SECTION=1",
    "-DWAMR_BUILD_SPEC_TEST=1",
    "-DWAMR_BUILD_BULK_MEMORY=1",
    "-DWAMR_BUILD_REF_TYPES=1",
])


def main():
    parser = argparse.ArgumentParser(
        description="Run the classic-interp feature-set coverage report.")
    parser.add_argument("--out", default="build/coverage",
                        help="Output root directory for reports.")
    parser.add_argument("--llvm-dir", default="",
                        help="LLVM cmake config dir; leave empty to use "
                             "run_coverage.py's default (the bundled LLVM "
                             "build at core/deps/llvm/build/lib/cmake/llvm).")
    args = parser.parse_args()

    cmd = [
        sys.executable, RUN_COVERAGE,
        "--report", "classic-fset",
        "--mode", "classic-interp",
        "--feature", FEATURE_SET,
        "--unit",
        "--out", args.out,
    ]
    if args.llvm_dir:
        cmd += ["--llvm-dir", args.llvm_dir]

    print("Running:", " ".join(cmd))
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)


if __name__ == "__main__":
    main()
