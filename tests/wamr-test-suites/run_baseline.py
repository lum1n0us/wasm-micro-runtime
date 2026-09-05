#!/usr/bin/env python3
#
# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""Baseline coverage invocation (v14).

Runs the report object defined in the coverage plan §1.2/§2.2:
  * running mode: classic-interp
  * preset: baseline (libc-builtin / shared-heap / global-heap-pool /
    load-custom-section + spec exemptions SPEC_TEST / BULK_MEMORY / REF_TYPES)
  * test set: spec + unit (compatible targets only, regression excluded)

Usage (inside the devcontainer, from the repository root):
  python3 tests/wamr-test-suites/run_baseline.py [--out DIR] [--llvm-dir DIR]
"""

import argparse
import os
import subprocess
import sys

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
RUN_COVERAGE = os.path.join(REPO_ROOT, "tests", "wamr-test-suites", "run_coverage.py")


def main():
    parser = argparse.ArgumentParser(description="Run the coverage baseline.")
    parser.add_argument("--out", default="build/coverage",
                        help="Output root directory for reports.")
    parser.add_argument("--llvm-dir", default="core/deps/llvm/build/lib/cmake/llvm",
                        help="LLVM cmake config directory (prebuilt LLVM linked at "
                             "core/deps/llvm/build in the devcontainer).")
    parser.add_argument("--no-unit", action="store_true",
                        help="Skip the unit test suite.")
    parser.add_argument("--full-test", action="store_true",
                        help="Build every unit suite including the "
                             "llm-enhanced-test submodule suites "
                             "(-DFULL_TEST=ON).")
    parser.add_argument("--regression", action="store_true",
                        help="Also run the regression tests (quality gate, "
                             "additional data only).")
    args = parser.parse_args()

    cmd = [
        sys.executable, RUN_COVERAGE,
        "--report", "baseline",
        "--mode", "classic-interp",
        "--preset", "baseline",
        "--llvm-dir", args.llvm_dir,
        "--out", args.out,
    ]
    if not args.no_unit:
        cmd.append("--unit")
    if args.full_test:
        cmd.append("--full-test")
    if args.regression:
        cmd.append("--regression")

    print("Running:", " ".join(cmd))
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)


if __name__ == "__main__":
    main()
