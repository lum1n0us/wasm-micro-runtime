#!/usr/bin/env python3

#
# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#

"""Parameterized code coverage runner for WAMR (v14).

Builds iwasm (spec layer) and unit tests under a given (running mode × preset)
combination and collects coverage with gcovr (see collect_gcovr.py).

A "report object" is one or more (running mode × preset) combinations; the
fingerprint of a report is the normalized serialization of its combinations:

    fingerprint = running-modes + preset(s)

Input is intentionally limited (plan §1.2/§2.3):
  * the user only selects the running mode (--mode);
  * the feature set F is expanded from a predefined preset (--preset, default
    derived from the mode);
  * no free-form --spec/--cmake feature input is accepted.

Unit compatibility (plan §2.3): the unit build for each mode is configured
with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON; the generated compile_commands.json
is parsed (coverage_compile_commands.py) and every target is checked against F
(rule 1: F=1 features must be enabled; rule 2: F=0 features must not be
enabled).  Only compatible targets' build directories are collected, and the
unmatched targets are written to the report as the "unmatched list".

Regression tests are a quality gate only (plan §1.2): they are not part of the
coverage report.  --regression keeps the old behavior for comparison but
labels the data as additional, not part of the scope.

Examples:
  # baseline report: classic-interp + preset=baseline + spec + unit
  python3 run_coverage.py --report baseline --mode classic-interp \
      --unit --out build/coverage

  # aot preset (mode derived from preset)
  python3 run_coverage.py --report aot --preset aot --unit --out build/coverage

  # full-default: classic-interp with no feature overrides and every unit
  # suite including llm-enhanced-test (FULL_TEST=ON)
  python3 run_coverage.py --report default-ci --mode classic-interp \
      --preset default --unit --full-test --out build/coverage

  # merge two previously generated reports
  python3 run_coverage.py --merge build/coverage/baseline_* build/coverage/aot_* \
      --out build/coverage
"""

import argparse
import glob
import hashlib
import os
import shutil
import subprocess
import sys

from coverage_preset import (
    MODE_PRESETS,
    PRESET_MODES,
    PRESETS,
    cmake_flags_for,
    expand_preset,
    validate_mode_preset,
)
from coverage_compile_commands import load_and_filter

TESTS_DIR = os.path.dirname(os.path.abspath(__file__))
WAMR_DIR = os.path.dirname(os.path.dirname(TESTS_DIR))
SPEC_TEST_DIR = os.path.join(TESTS_DIR, "spec-test-script")
UNIT_DIR = os.path.join(WAMR_DIR, "tests", "unit")
IWEAM_PLATFORM_DIR = os.path.join(WAMR_DIR, "product-mini", "platforms")

RUNNING_MODES = sorted(MODE_PRESETS) + ["fast-interp", "multi-tier-jit"]

# test_wamr.sh COMPILE_FLAGS equivalents per running mode.
MODE_BUILD_FLAGS = {
    "classic-interp": (
        "-DWAMR_BUILD_INTERP=1 -DWAMR_BUILD_FAST_INTERP=0 "
        "-DWAMR_BUILD_JIT=0 -DWAMR_BUILD_AOT=0"
    ),
    "fast-interp": (
        "-DWAMR_BUILD_INTERP=1 -DWAMR_BUILD_FAST_INTERP=1 "
        "-DWAMR_BUILD_JIT=0 -DWAMR_BUILD_AOT=0"
    ),
    "aot": (
        "-DWAMR_BUILD_INTERP=1 -DWAMR_BUILD_FAST_INTERP=0 "
        "-DWAMR_BUILD_JIT=0 -DWAMR_BUILD_AOT=1"
    ),
    "jit": (
        "-DWAMR_BUILD_INTERP=1 -DWAMR_BUILD_FAST_INTERP=0 "
        "-DWAMR_BUILD_JIT=1 -DWAMR_BUILD_AOT=0 -DWAMR_BUILD_LAZY_JIT=0"
    ),
    # Same configuration as 'jit'; 'llvm-jit' is the unit-test run mode name
    # (unit_common.cmake), the spec layer still calls it 'jit'.  Note the
    # unit-test configure validates the mode combination: AOT must not be
    # combined with JIT (unit_common.cmake) and the classic interpreter stays
    # on (top-level default), mirroring the CI unit runs.
    "llvm-jit": (
        "-DWAMR_BUILD_INTERP=1 -DWAMR_BUILD_FAST_INTERP=0 "
        "-DWAMR_BUILD_JIT=1 -DWAMR_BUILD_AOT=0 -DWAMR_BUILD_LAZY_JIT=0"
    ),
    "fast-jit": (
        "-DWAMR_BUILD_INTERP=1 -DWAMR_BUILD_FAST_INTERP=0 "
        "-DWAMR_BUILD_JIT=0 -DWAMR_BUILD_AOT=0 -DWAMR_BUILD_FAST_JIT=1"
    ),
    "multi-tier-jit": (
        "-DWAMR_BUILD_INTERP=1 -DWAMR_BUILD_FAST_INTERP=0 "
        "-DWAMR_BUILD_FAST_JIT=1 -DWAMR_BUILD_JIT=1"
    ),
}


def platform() -> str:
    return subprocess.run(
        ["uname", "-s"], capture_output=True, text=True, check=True
    ).stdout.strip().lower()


def resolve_llvm_dir(llvm_dir: str) -> str:
    """Return llvm_dir as an absolute path (relative ones are resolved
    against the repository root, since cmake subprocesses run with various
    working directories)."""
    if os.path.isabs(llvm_dir):
        return llvm_dir
    return os.path.abspath(os.path.join(WAMR_DIR, llvm_dir))


def fingerprint(combos) -> str:
    """Normalized fingerprint of the report's combinations."""
    parts = []
    for combo in combos:
        parts.append(combo["mode"])
        parts.append(combo["preset"])
    raw = "||".join(parts)
    return hashlib.sha1(raw.encode()).hexdigest()[:16]


def run_spec(workdir, mode):
    """Run the spec test suite via test_wamr.sh (phase-1 approach).  The spec
    iwasm build uses test_wamr.sh's fixed feature configuration.

    test_wamr.sh names the LLVM-JIT mode 'jit'; 'llvm-jit' is the unit-test
    name, so translate it here.

    test_wamr.sh re-clones the spec repo on github before every run, which
    intermittently fails (empty replies / HTTP2 framing errors), so retry a
    few times before giving up."""
    script = os.path.join(TESTS_DIR, "test_wamr.sh")
    spec_mode = "jit" if mode == "llvm-jit" else mode
    cmd = ["bash", script, "-s", "spec", "-b", "-t", spec_mode, "-C"]
    env = dict(os.environ, COLLECT_CODE_COVERAGE="1")
    last_exc = None
    for attempt in range(1, 4):
        try:
            subprocess.run(cmd, cwd=TESTS_DIR, env=env, check=True)
            return
        except subprocess.CalledProcessError as exc:
            last_exc = exc
            print(f"spec run failed (attempt {attempt}/3); retrying...")
            import time
            time.sleep(5)
    raise last_exc


def run_unit(workdir, mode, llvm_dir, coverage, full_test=False):
    """Configure + build + test the unit suites for one running mode, then
    filter the build directories by compile_commands.json compatibility.

    The preset feature flags are deliberately NOT injected into the unit
    configure: every suite (including the llm-enhanced-test ones under
    FULL_TEST=ON) declares the feature values its own test plan needs via
    set(WAMR_BUILD_*), and injecting top-level -DWAMR_BUILD_*=1 flags would
    turn core code on for suites whose curated source lists do not link the
    matching wrapper (e.g. SHARED_HEAP=1 breaks llm interpreter-core).
    compile_commands.json therefore records exactly what each suite built,
    and the compatibility check against F (coverage_compile_commands.py)
    decides which of those targets belong to the report's denominator."""
    build_dir = os.path.join(workdir, f"unittest-build-{mode}")
    cmake_args = [
        "cmake", "-S", UNIT_DIR, "-B", build_dir,
        "-DCMAKE_BUILD_TYPE=Debug",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
    ]
    if shutil.which("ninja"):
        cmake_args.append("-G Ninja")
        if shutil.which("ccache"):
            # Every suite compiles the same core sources; ccache turns the
            # repeated compilations into cache hits.
            cmake_args.append("-DCMAKE_C_COMPILER_LAUNCHER=ccache")
            cmake_args.append("-DCMAKE_CXX_COMPILER_LAUNCHER=ccache")
    cmake_args.extend(MODE_BUILD_FLAGS[mode].split())
    if full_test:
        # Build every unit suite, including the llm-enhanced-test submodule
        # suites (they declare their own run-mode support).
        cmake_args.append("-DFULL_TEST=ON")
    if coverage:
        cmake_args.append("-DCOLLECT_CODE_COVERAGE=1")
    if llvm_dir:
        cmake_args.append(f"-DLLVM_DIR={resolve_llvm_dir(llvm_dir)}")

    # Reuse already-downloaded googletest/cmocka sources when available (set
    # FETCHCONTENT_LOCAL_SRC to a build dir that contains _deps/): the unit
    # configure downloads them from github/gitlab with libcurl, which is
    # flaky on some networks (HTTP/2 stream errors).
    local_deps = os.environ.get("FETCHCONTENT_LOCAL_SRC", "")
    if local_deps and os.path.isdir(local_deps):
        for name in ("googletest", "cmocka"):
            src = os.path.join(local_deps, name + "-src")
            if os.path.isdir(src):
                cmake_args.append(
                    f"-DFETCHCONTENT_SOURCE_DIR_{name.upper()}={src}")

    # The googletest download is network-dependent and intermittently fails
    # (HTTP/2 framing); retry the configure a few times before giving up.
    last_exc = None
    for _attempt in range(3):
        try:
            subprocess.run(cmake_args, check=True)
            break
        except subprocess.CalledProcessError as exc:
            last_exc = exc
            print(f"cmake configure failed (attempt "
                  f"{_attempt + 1}/3); retrying...")
    else:
        raise last_exc
    jobs = os.cpu_count() or 4
    subprocess.run(
        ["cmake", "--build", build_dir, "-j", str(min(jobs, 8))], check=True)
    subprocess.run(
        ["ctest", "--test-dir", build_dir, "--output-on-failure"],
        check=True,
    )
    return build_dir


def run_regression(mode, coverage):
    """Quality gate only (plan §1.2): regression tests are not part of the
    coverage report.  Kept for the --regression compatibility flag."""
    script = os.path.join(WAMR_DIR, "tests", "regression", "ba-issues",
                          "build_run.py")
    cmd = [sys.executable, script, "--mode", mode]
    if coverage:
        cmd.append("--coverage")
    subprocess.run(cmd, cwd=os.path.dirname(script), check=True)


def collect(build_dirs, out_dir):
    script = os.path.join(SPEC_TEST_DIR, "collect_gcovr.py")
    cmd = [sys.executable, script, "--out", out_dir] + build_dirs
    subprocess.run(cmd, check=True)


def run_report(name, combos, out_root, unit, regression, llvm_dir, coverage,
               full_test=False):
    workdir = os.path.join(out_root, "_work", name)
    os.makedirs(workdir, exist_ok=True)

    fp = fingerprint(combos)
    out_dir = os.path.join(out_root, f"{name}_{fp}")
    os.makedirs(out_dir, exist_ok=True)

    all_build_dirs = []

    for combo in combos:
        mode = combo["mode"]
        preset = combo["preset"]
        if preset == "default":
            # Full-default report object: no compilation flags beyond the
            # running-mode ones (each suite keeps its own CMakeLists
            # feature values); F constrains only the mode features.
            f = expand_preset("default", mode)
            print(f"\n=== report '{name}' / mode '{mode}' / preset 'default' ===")
            print("(no feature overrides: suites keep their own defaults)\n")
        else:
            f = expand_preset(preset, mode)
            print(f"\n=== report '{name}' / mode '{mode}' / preset '{preset}' ===")
            print(f"F = {cmake_flags_for(f)}\n")
            print("(feature flags are not injected into the unit configure: "
                  "each suite declares its own; compile_commands.json and the "
                  "compatibility check below select the report's unit targets)\n")

        # spec layer: test_wamr.sh builds its own iwasm (fixed configuration,
        # mode flags + mandatory SPEC_TEST/BULK_MEMORY/REF_TYPES) and runs the
        # spec suite on it; its .gcda lands in
        # product-mini/platforms/<plat>/build, which we collect below.
        run_spec(workdir, mode)
        spec_build_dir = os.path.join(
            IWEAM_PLATFORM_DIR, platform(), "build")
        if os.path.isdir(spec_build_dir):
            all_build_dirs.append(spec_build_dir)
        else:
            print(f"WARNING: spec iwasm build dir not found: {spec_build_dir}")

        if unit:
            unit_dir = run_unit(workdir, mode, llvm_dir, coverage,
                                full_test)
            cc_path = os.path.join(unit_dir, "compile_commands.json")
            (compatible, incompatible, unmatched, missing,
             dir_violations, dir_coverage_missing) = load_and_filter(
                cc_path, f, mode, UNIT_DIR)
            print(f"unit targets compatible with preset '{preset}': "
                  f"{compatible}")
            print(f"unit targets incompatible (excluded): {incompatible}")
            for t, c in unmatched:
                print(f"  - {t}: {c}")
            print("rule-1 gap warnings (F=1 features with no unit source):")
            for m in missing:
                print(f"  - {m}")
            print("denominator #2 dir violations (F=0 dirs compiled):")
            for v in dir_violations:
                print(f"  - {v}")
            print("denominator #2 coverage missing (F=1 dirs absent):")
            for m in dir_coverage_missing:
                print(f"  - {m}")
            with open(os.path.join(out_dir, "unmatched.txt"), "w") as fh:
                fh.write(f"mode={mode} preset={preset}\n")
                fh.write("compatible targets:\n")
                for t in compatible:
                    fh.write(f"  {t}\n")
                fh.write("incompatible targets (excluded from coverage):\n")
                for t, c in unmatched:
                    fh.write(f"  {t}: {c}\n")
                fh.write("rule-1 gap warnings (F=1 features with no unit "
                         "source):\n")
                for m in missing:
                    fh.write(f"  {m}\n")
                fh.write("denominator #2 dir violations (F=0 dirs compiled):\n")
                for v in dir_violations:
                    fh.write(f"  {v}\n")
                fh.write("denominator #2 coverage missing (F=1 dirs absent):\n")
                for m in dir_coverage_missing:
                    fh.write(f"  {m}\n")

            # collect only the compatible targets' build dirs
            for target in compatible:
                target_dir = os.path.join(unit_dir, target)
                if os.path.isdir(target_dir):
                    all_build_dirs.append(target_dir)

        if regression:
            reg_dir = os.path.join(WAMR_DIR, "tests", "regression", "ba-issues",
                                   "build")
            run_regression(mode, coverage)
            all_build_dirs.append(reg_dir)

    collect(all_build_dirs, out_dir)

    # Write the fingerprint file
    with open(os.path.join(out_dir, "fingerprint.txt"), "w") as f:
        f.write(f"name={name}\n")
        f.write(f"fingerprint={fp}\n")
        f.write("combinations:\n")
        for combo in combos:
            f.write(f"  mode={combo['mode']}\n")
            f.write(f"  preset={combo['preset']}\n")
    print(f"Report {name} written to {out_dir} (fingerprint {fp})")


def merge_reports(reports, out_dir):
    """Merge several previously generated reports (their build dirs are not
    available anymore, so merge at the report level via gcovr object dirs
    collected from each report's _work directory if present)."""
    merged_out = os.path.join(out_dir, "_merged")
    os.makedirs(merged_out, exist_ok=True)
    build_dirs = []
    for report in reports:
        work = os.path.join(out_dir, "_work", report)
        if os.path.isdir(work):
            for d in os.listdir(work):
                p = os.path.join(work, d)
                if os.path.isdir(p):
                    build_dirs.append(p)
    if not build_dirs:
        raise SystemExit(
            "No _work build dirs found for the given reports; run the reports "
            "first so their .gcda data is available for merging."
        )
    collect(build_dirs, merged_out)
    with open(os.path.join(merged_out, "fingerprint.txt"), "w") as f:
        f.write(f"merged={','.join(reports)}\n")
        f.write(f"build_dirs={build_dirs}\n")
    print(f"Merged report written to {merged_out}")


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Parameterized WAMR coverage runner (v14).  Report object = "
            "(running mode × preset).  The user selects the running mode; the "
            "feature set F is expanded from a predefined preset.  No "
            "free-form feature input."
        )
    )
    parser.add_argument(
        "--report", action="append", default=[],
        help="Report name; repeatable. Follow with --mode/--preset.",
    )
    parser.add_argument(
        "--mode", action="append", default=[],
        choices=RUNNING_MODES + ["jit"],
        help="Running mode for the current report; repeatable.  'jit' is an "
             "alias for 'llvm-jit' (the unit-test run mode name).  Default: "
             "derived from --preset (or classic-interp).",
    )
    parser.add_argument(
        "--preset", action="append", default=[],
        choices=sorted(PRESETS),
        help="Predefined feature group for the current report; repeatable.  "
             "Default: derived from --mode.  'default' runs the mode without "
             "any feature overrides (each suite keeps its own defaults).",
    )
    parser.add_argument("--unit", action="store_true",
                        help="Run the unit tests and merge their coverage "
                             "(compatible targets only).")
    parser.add_argument("--full-test", action="store_true",
                        help="With --unit, build every unit suite including "
                             "the llm-enhanced-test submodule suites "
                             "(-DFULL_TEST=ON).")
    parser.add_argument("--regression", action="store_true",
                        help="Also run the regression tests (quality gate). "
                             "Their .gcda is collected as additional data, "
                             "not part of the report scope.")
    parser.add_argument("--llvm-dir", default="",
                        help="LLVM cmake config dir "
                             "(e.g. core/deps/llvm/build/lib/cmake/llvm).")
    parser.add_argument("--out", default="coverage-reports",
                        help="Output root directory for reports.")
    parser.add_argument(
        "--merge", action="append", default=[],
        help="Merge previously generated reports by name; repeatable.",
    )
    args = parser.parse_args()

    if args.merge:
        merge_reports(args.merge, args.out)
        return

    if not args.report:
        parser.error("--report is required (or use --merge)")

    # 'jit' (spec/test_wamr.sh naming) is an alias for 'llvm-jit' (unit-test
    # naming); normalize early so fingerprints and build dirs are stable.
    args.mode = ["llvm-jit" if m == "jit" else m for m in args.mode]

    # Each --report starts a new group; --mode and --preset are paired by
    # position (mode[i], preset[i]); defaults fill the rest.
    reports = []
    n_modes = len(args.mode)
    n_presets = len(args.preset)
    for i, report in enumerate(args.report):
        mode = args.mode[i] if i < n_modes else None
        preset = args.preset[i] if i < n_presets else None
        if mode is None and preset is None:
            mode = "classic-interp"
        if preset is None:
            preset = MODE_PRESETS.get(mode)
            if preset is None:
                parser.error(
                    f"no default preset for mode '{mode}'; pass --preset"
                )
        elif mode is None:
            mode = PRESET_MODES.get(preset, "classic-interp")
        preset = validate_mode_preset(mode, preset)
        reports.append({
            "name": report,
            "combos": [{"mode": mode, "preset": preset}],
        })

    coverage = True  # this tool always collects coverage
    for r in reports:
        run_report(r["name"], r["combos"], args.out, args.unit,
                   args.regression, args.llvm_dir, coverage,
                   args.full_test)


if __name__ == "__main__":
    main()
