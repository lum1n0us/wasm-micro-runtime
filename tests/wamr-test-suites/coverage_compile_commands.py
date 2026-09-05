#!/usr/bin/env python3

#
# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#

"""Unit-test compatibility filtering driven by compile_commands.json.

Why compile_commands.json (plan §1.1/§2.3): WAMR turns every feature cmake
variable into a `-DWASM_ENABLE_XXX=1` compile macro (config_common.cmake /
runtime_lib.cmake / iwasm_gc.cmake via add_definitions), so the compile
command of each translation unit is the single authoritative source of the
feature set that unit was built with -- set(), if(), included .cmake files
and platform conditions are all already expanded.

Matching rules (compile-unit level, plan §2.3):
  * rule 1: F[f]=1  -> the unit's macro set M(u) must contain f with value 1
  * rule 2: F[f]=0  -> M(u) must not contain f with value 1
  * features not enumerated in F are wildcards (not constrained)

Granularity: compile unit -> target (add_executable) -> suite.  The check is
done at the *target* level: a target's macro set is the union of its compile
units' macro sets (a suite can build several executables, e.g. mem-alloc).
Individual gtest cases inherit their target; finer case-level control relies
on source-level #if guards at compile time (no runtime filtering).
"""

import json
import os
import re
from collections import defaultdict
from typing import Dict, List, Optional, Set, Tuple

# CMake variable name <-> compile macro name mapping (plan §2.3).
# Most features map WAMR_BUILD_X -> WASM_ENABLE_X; a few differ.
CMAKE_TO_MACRO = {
    "WAMR_BUILD_LIBC_BUILTIN": "WASM_ENABLE_LIBC_BUILTIN",
    "WAMR_BUILD_MULTI_MODULE": "WASM_ENABLE_MULTI_MODULE",
    "WAMR_BUILD_SHARED_HEAP": "WASM_ENABLE_SHARED_HEAP",
    "WAMR_BUILD_LOAD_CUSTOM_SECTION": "WASM_ENABLE_LOAD_CUSTOM_SECTION",
    "WAMR_BUILD_EXCE_HANDLING": "WASM_ENABLE_EXCE_HANDLING",
    "WAMR_BUILD_MEMORY64": "WASM_ENABLE_MEMORY64",
    "WAMR_BUILD_GC": "WASM_ENABLE_GC",
    "WAMR_BUILD_REF_TYPES": "WASM_ENABLE_REF_TYPES",
    "WAMR_BUILD_BULK_MEMORY": "WASM_ENABLE_BULK_MEMORY",
    "WAMR_BUILD_SIMD": "WASM_ENABLE_SIMD",
    "WAMR_BUILD_SHARED_MEMORY": "WASM_ENABLE_SHARED_MEMORY",
    "WAMR_BUILD_LIB_PTHREAD": "WASM_ENABLE_LIB_PTHREAD",
    "WAMR_BUILD_TAIL_CALL": "WASM_ENABLE_TAIL_CALL",
    "WAMR_BUILD_FAST_INTERP": "WASM_ENABLE_FAST_INTERP",
    "WAMR_BUILD_LAZY_JIT": "WASM_ENABLE_LAZY_JIT",
    "WAMR_BUILD_JIT": "WASM_ENABLE_JIT",
    "WAMR_BUILD_FAST_JIT": "WASM_ENABLE_FAST_JIT",
    "WAMR_BUILD_AOT": "WASM_ENABLE_AOT",
    "WAMR_BUILD_INTERP": "WASM_ENABLE_INTERP",
    "WAMR_BUILD_LIBC_WASI": "WASM_ENABLE_LIBC_WASI",
    "WAMR_BUILD_LIBC_UVWASI": "WASM_ENABLE_LIBC_UVWASI",
    "WAMR_BUILD_MINI_LOADER": "WASM_ENABLE_MINI_LOADER",
    "WAMR_BUILD_MULTI_MEMORY": "WASM_ENABLE_MULTI_MEMORY",
    "WAMR_BUILD_EXTENDED_CONST_EXPR": "WASM_ENABLE_EXTENDED_CONST_EXPR",
}

# compile macro -> cmake variable (inverse of the above, best-effort).
MACRO_TO_CMAKE = {v: k for k, v in CMAKE_TO_MACRO.items()}

# Denominator validation #2 (plan §2.5): feature -> core source directories.
# For every F=0 feature, no compatible target may compile files under its
# directories; for every F=1 feature, at least one compatible target must.
# Only feature-specific directories are listed: shared directories that are
# compiled in every configuration (e.g. core/iwasm/common, common/arch) are
# deliberately absent so they do not produce false violations.
FEATURE_DIRS = {
    "WAMR_BUILD_GC": ["core/iwasm/common/gc"],
    "WAMR_BUILD_AOT": ["core/iwasm/aot", "core/iwasm/compilation"],
    "WAMR_BUILD_JIT": ["core/iwasm/compilation"],
    "WAMR_BUILD_FAST_JIT": ["core/iwasm/fast-jit"],
    "WAMR_BUILD_MEMORY64": [],  # memory64 affects many files via macro
    "WAMR_BUILD_LIBC_BUILTIN": ["core/iwasm/libraries/libc-builtin"],
    "WAMR_BUILD_LIBC_WASI": ["core/iwasm/libraries/libc-wasi"],
    "WAMR_BUILD_LIBC_UVWASI": ["core/iwasm/libraries/libc-uvwasi"],
    "WAMR_BUILD_SHARED_HEAP": ["core/iwasm/libraries/shared-heap"],
    "WAMR_BUILD_EXCE_HANDLING": [],
    "WAMR_BUILD_MULTI_MODULE": [],
    "WAMR_BUILD_LOAD_CUSTOM_SECTION": [],
}

# Regex for -DWASM_ENABLE_XXX[=0|1] tokens in a compile command.
_MACRO_RE = re.compile(r'-D(WASM_ENABLE_[A-Z0-9_]+)(?:=([01]))?')


class CompileUnit:
    """One entry of compile_commands.json (one source file's compile command)."""

    __slots__ = ("file", "directory", "macros", "target")

    def __init__(self, file: str, directory: str, macros: Dict[str, int]):
        self.file = file          # absolute source path (may be under core/ or tests/unit/)
        self.directory = directory  # build dir this unit was compiled into
        self.macros = macros      # WASM_ENABLE_* -> 0|1 (present macros only)
        self.target = ""          # target/suite name, filled by classify()

    def __repr__(self):
        return f"<CompileUnit {self.file} target={self.target} macros={self.macros}>"


def parse_compile_commands(path: str) -> List[CompileUnit]:
    """Load compile_commands.json and extract macro sets per compile unit."""
    with open(path, "r") as f:
        data = json.load(f)
    units = []
    for entry in data:
        file = entry.get("file", "")
        directory = entry.get("directory", "")
        # command may be a string or an argv list (depending on the generator)
        command = entry.get("command")
        if command is None and "arguments" in entry:
            command = " ".join(entry["arguments"])
        if not command:
            continue
        macros = {}
        for m in _MACRO_RE.finditer(command):
            name, val = m.group(1), m.group(2)
            macros[name] = int(val) if val is not None else 1
        units.append(CompileUnit(file, directory, macros))
    return units


def _known_suites(unit_dir: str):
    """Immediate suite directories under tests/unit (both the parent suites
    and, when FULL_TEST is on, the llm-enhanced-test submodule suites).

    Only directories that contain their own CMakeLists.txt count as suites:
    stray/gitignored directories (e.g. a leftover 'build' dir or the 'common'
    test-helper include dir) must not be treated as targets.

    Returns (parent_names, llm_names).  llm_names are the suites that live
    under tests/unit/llm-enhanced-test/."""
    def _cmake_suites(root: str):
        names = set()
        if not os.path.isdir(root):
            return names
        for name in os.listdir(root):
            sub = os.path.join(root, name)
            if os.path.isdir(sub) and not name.startswith(".") \
                    and os.path.isfile(os.path.join(sub, "CMakeLists.txt")):
                names.add(name)
        return names

    return _cmake_suites(unit_dir), \
        _cmake_suites(os.path.join(unit_dir, "llm-enhanced-test"))


def classify_targets(units: List[CompileUnit], unit_dir: str) -> Dict[str, List[CompileUnit]]:
    """Group compile units by suite (add_executable).

    A unit-test target belongs to a suite; the suite key is its directory
    relative to tests/unit: a plain "<suite>" for the parent suites and a
    nested "llm-enhanced-test/<suite>" for the llm-enhanced-test submodule
    suites (FULL_TEST=ON).  Core/ sources are compiled into every suite, so
    they are assigned to the suite that includes them via the build directory
    path (unittest-build-<mode>/<suite>/...).  Returns {suite_key: [units]}.
    """
    targets: Dict[str, List[CompileUnit]] = defaultdict(list)
    unit_root = os.path.abspath(unit_dir)
    parent_names, llm_names = _known_suites(unit_root)

    def llm_key(suite_name: str) -> str:
        return f"llm-enhanced-test/{suite_name}"

    for u in units:
        key = None
        # suite source files: tests/unit/<suite>/... or
        # tests/unit/llm-enhanced-test/<suite>/...
        rel = os.path.relpath(u.file, unit_root)
        if not rel.startswith(".."):
            parts = [p for p in rel.split(os.sep) if p and p != "."]
            if parts:
                if parts[0] == "llm-enhanced-test":
                    if len(parts) > 1:
                        key = llm_key(parts[1])
                elif parts[0] in parent_names:
                    key = parts[0]
        if key is None:
            # core/ sources: infer the suite from the build directory (the
            # build tree is <build>/<suite>/... for the parent suites and
            # <build>/llm-enhanced-test/<suite>/... for the submodule ones).
            # Pick the *deepest* suite-looking segment so an llm suite that
            # shares a name with a parent suite is not confused with it.
            segs = u.directory.split(os.sep)
            best, best_idx = None, -1
            for i, part in enumerate(segs):
                if part == "llm-enhanced-test":
                    if i + 1 < len(segs) and segs[i + 1] in llm_names \
                            and i + 1 > best_idx:
                        best, best_idx = llm_key(segs[i + 1]), i + 1
                elif part in parent_names and not (
                        i > 0 and segs[i - 1] == "llm-enhanced-test"):
                    if i > best_idx:
                        best, best_idx = part, i
            key = best
        u.target = key or "unknown"
        targets[u.target].append(u)
    return targets


def target_macros(units: List[CompileUnit]) -> Dict[str, int]:
    """Union of macro sets across a target's compile units."""
    merged: Dict[str, int] = {}
    for u in units:
        for name, val in u.macros.items():
            # union: if any unit enables it, the target enables it
            if name not in merged or val == 1:
                merged[name] = val
    return merged


def target_excluded(f: Dict[str, int], macros: Dict[str, int]) -> List[str]:
    """Rule 2 check: does this target enable any feature F explicitly sets to 0?

    This is the hard exclusion rule (the denominator must not contain code
    compiled with a feature the report's F disables).  Returns the conflict
    list (empty = the target passes rule 2 and stays in the denominator).
    """
    conflicts = []
    for cmake_name, want in f.items():
        if want != 0:
            continue
        macro_name = CMAKE_TO_MACRO.get(cmake_name)
        if macro_name is None:
            continue  # no compile macro: not checkable
        if macros.get(macro_name) == 1:
            conflicts.append(f"{cmake_name}=0 but {macro_name}=1")
    return conflicts


def missing_features(f: Dict[str, int], targets: Dict[str, List[CompileUnit]]) -> List[str]:
    """Rule 1 check (whole-denominator): every F=1 feature must be enabled by
    at least one compatible target, otherwise the feature has no unit
    coverage source (a gap, reported as a warning, not an exclusion)."""
    enabled_macros: Set[str] = set()
    for units in targets.values():
        enabled_macros.update(
            name for name, val in target_macros(units).items() if val == 1
        )
    missing = []
    for cmake_name, want in f.items():
        if want != 1:
            continue
        macro_name = CMAKE_TO_MACRO.get(cmake_name)
        if macro_name is None:
            continue
        if macro_name not in enabled_macros:
            missing.append(f"{cmake_name}=1 has no unit target enabling it")
    return missing


def denominator_dir_check(
    f: Dict[str, int],
    targets: Dict[str, List[CompileUnit]],
    compatible: List[str],
    repo_root: str = "",
) -> Tuple[List[str], List[str]]:
    """Denominator validation #2 (plan §2.5): directory whitelist check.

    * F=0 features: their source directories must not appear among the
      compatible targets' files (code that should not exist under F);
    * F=1 features: at least one compatible target must compile a file from
      their directories (otherwise the feature has no unit coverage source).

    Returns (violations, coverage_missing) -- both advisory, reported in the
    report header, not exclusions.
    """
    if not repo_root:
        repo_root = os.getcwd()
    root = os.path.abspath(repo_root)
    compat_units = [
        u for t in compatible for u in targets.get(t, [])
    ]
    compat_files = {os.path.abspath(u.file) for u in compat_units}
    violations = []
    coverage_missing = []
    for cmake_name, want in f.items():
        dirs = FEATURE_DIRS.get(cmake_name, [])
        if not dirs:
            continue
        abs_dirs = [os.path.abspath(os.path.join(root, d)) for d in dirs]
        if want == 0:
            for d in abs_dirs:
                if any(f.startswith(d + os.sep) for f in compat_files):
                    violations.append(
                        f"F sets {cmake_name}=0 but compatible targets "
                        f"compile files under {d}"
                    )
        else:  # want == 1
            if not any(f.startswith(d + os.sep)
                       for d in abs_dirs for f in compat_files):
                coverage_missing.append(
                    f"F sets {cmake_name}=1 but no compatible target "
                    f"compiles files under {dirs}"
                )
    return violations, coverage_missing


def filter_compatible_targets(
    targets: Dict[str, List[CompileUnit]],
    f: Dict[str, int],
    mode: str,
    repo_root: str = "",
) -> Tuple[List[str], List[str], List[Tuple[str, str]], List[str],
           List[str], List[str]]:
    """Classify targets: (compatible, incompatible, unmatched, missing,
    dir_violations, dir_coverage_missing).

    * compatible targets pass rule 2 (they do not enable any F=0 feature);
    * incompatible targets enable at least one F=0 feature (excluded from the
      denominator, reported in the unmatched list);
    * missing is the rule-1 whole-denominator check: F=1 features with no
      compatible target enabling them (gap warnings, not exclusions);
    * dir_violations / dir_coverage_missing: denominator validation #2
      (plan §2.5), advisory.
    """
    compatible, incompatible = [], []
    unmatched: List[Tuple[str, str]] = []
    for target, units in sorted(targets.items()):
        if target == "unknown":
            continue
        conflicts = target_excluded(f, target_macros(units))
        if conflicts:
            incompatible.append(target)
            unmatched.append((target, "; ".join(conflicts)))
        else:
            compatible.append(target)
    missing = missing_features(f, targets)
    dir_violations, dir_coverage_missing = denominator_dir_check(
        f, targets, compatible, repo_root)
    return (compatible, incompatible, unmatched, missing,
            dir_violations, dir_coverage_missing)


def load_and_filter(
    compile_commands_path: str,
    f: Dict[str, int],
    mode: str,
    unit_dir: str,
) -> Tuple[List[str], List[str], List[Tuple[str, str]], List[str],
           List[str], List[str]]:
    """One-stop helper: parse compile_commands.json, classify, filter.

    unit_dir is tests/unit; the repo root is two levels above it."""
    units = parse_compile_commands(compile_commands_path)
    targets = classify_targets(units, unit_dir)
    repo_root = os.path.dirname(os.path.dirname(os.path.abspath(unit_dir)))
    return filter_compatible_targets(targets, f, mode, repo_root)


if __name__ == "__main__":
    import sys
    from coverage_preset import expand_preset

    cc = sys.argv[1] if len(sys.argv) > 1 else "compile_commands.json"
    preset = sys.argv[2] if len(sys.argv) > 2 else "baseline"
    f = expand_preset(preset)
    (compatible, incompatible, unmatched, missing,
     dir_violations, dir_coverage_missing) = load_and_filter(
        cc, f, "classic-interp",
        os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                     "unit"),
    )
    print(f"preset={preset}")
    print(f"compatible targets ({len(compatible)}): {sorted(compatible)}")
    print(f"incompatible targets ({len(incompatible)}):")
    for t, c in unmatched:
        print(f"  {t}: {c}")
    print("missing (rule-1 gap warnings):")
    for m in missing:
        print(f"  {m}")
    print("denominator dir violations (F=0 dirs present):")
    for v in dir_violations:
        print(f"  {v}")
    print("denominator dir coverage missing (F=1 dirs absent):")
    for m in dir_coverage_missing:
        print(f"  {m}")
