#!/usr/bin/env python3

#
# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#

"""The feature set F of a coverage report object.

A "report object" is `(running mode × spec options × feature set F)`.  The
user spells F out directly as cmake switches (--feature):

    --feature "-DWAMR_BUILD_GC=1 -DWAMR_BUILD_REF_TYPES=1"

`expand_features()` completes it into a fully enumerated F
(WAMR_BUILD_* cmake variables, every one set to 0/1):

    F = the listed =1 features
      + implied features (derived from the cmake "if (X EQUAL 1) set (Y 1)"
        rules, mirrored from build-scripts/runtime_lib.cmake)
      + everything else explicitly =0  (the full feature checklist)
      + the running-mode features, calibrated to the report's --mode

so there is no "unspecified = wildcard" ambiguity left.  An *empty* F is the
one exception: it constrains only the running-mode features and leaves every
other feature a wildcard (each unit suite then keeps the values its own
CMakeLists declares), which is the default for a report that is not about a
specific feature set.

F is never injected into the build: it only decides which unit targets belong
to the report (coverage_compile_commands.py) and drives the denominator
checks.
"""

# Every WAMR_BUILD_* cmake switch known to the build: the full checklist.
# Used to emit an explicit -DWAMR_BUILD_XXX=0 for every feature F does not
# enable, and to reject typos in --feature.
ALL_FEATURES = [
    # runtime modes
    "WAMR_BUILD_INTERP",
    "WAMR_BUILD_FAST_INTERP",
    "WAMR_BUILD_JIT",
    "WAMR_BUILD_FAST_JIT",
    "WAMR_BUILD_LAZY_JIT",
    "WAMR_BUILD_AOT",
    # core features
    "WAMR_BUILD_SIMD",
    "WAMR_BUILD_GC",
    "WAMR_BUILD_GC_HEAP_VERIFY",
    "WAMR_BUILD_GC_PERF_PROFILING",
    "WAMR_BUILD_MEMORY64",
    "WAMR_BUILD_MULTI_MEMORY",
    "WAMR_BUILD_MULTI_MODULE",
    "WAMR_BUILD_EXCE_HANDLING",
    "WAMR_BUILD_TAIL_CALL",
    "WAMR_BUILD_EXTENDED_CONST_EXPR",
    "WAMR_BUILD_LIME",
    "WAMR_BUILD_LIME1",
    "WAMR_BUILD_SHARED_MEMORY",
    "WAMR_BUILD_STRINGREF",
    "WAMR_BUILD_LIB_PTHREAD",
    "WAMR_BUILD_LIB_PTHREAD_SEMAPHORE",
    "WAMR_BUILD_LIB_WASI_THREADS",
    "WAMR_BUILD_LIBC_BUILTIN",
    "WAMR_BUILD_LIBC_WASI",
    "WAMR_BUILD_LIBC_UVWASI",
    "WAMR_BUILD_LIBC_EMCC",
    "WAMR_BUILD_SHARED_HEAP",
    "WAMR_BUILD_GLOBAL_HEAP_POOL",
    "WAMR_BUILD_LOAD_CUSTOM_SECTION",
    "WAMR_BUILD_MINI_LOADER",
    "WAMR_BUILD_SHRUNK_MEMORY",
    "WAMR_BUILD_DUMP_CALL_STACK",
    "WAMR_BUILD_PERF_PROFILING",
    "WAMR_BUILD_MEMORY_PROFILING",
    "WAMR_BUILD_MEMORY_TRACING",
    "WAMR_BUILD_DEBUG_INTERP",
    "WAMR_BUILD_DEBUG_AOT",
    "WAMR_BUILD_CUSTOM_NAME_SECTION",
    "WAMR_BUILD_DYNAMIC_AOT_DEBUG",
    "WAMR_BUILD_AOT_STACK_FRAME",
    "WAMR_BUILD_QUICK_AOT_ENTRY",
    "WAMR_BUILD_STATIC_PGO",
    "WAMR_BUILD_WASM_CACHE",
    "WAMR_BUILD_LINUX_PERF",
    "WAMR_BUILD_INSTRUCTION_METERING",
    "WAMR_BUILD_BRANCH_HINTS",
    "WAMR_BUILD_WASI_TEST",
    "WAMR_BUILD_SGX_IPFS",
    "WAMR_BUILD_WASI_NN",
    "WAMR_BUILD_LIB_SIMDE",
    "WAMR_BUILD_LIB_RATS",
    "WAMR_BUILD_MODULE_INST_CONTEXT",
    "WAMR_BUILD_COPY_CALL_STACK",
    "WAMR_BUILD_AOT_VALIDATOR",
    "WAMR_BUILD_AOT_INTRINSICS",
    "WAMR_BUILD_ALLOC_WITH_USAGE",
    "WAMR_BUILD_ALLOC_WITH_USER_DATA",
    "WAMR_BUILD_BULK_MEMORY",
    "WAMR_BUILD_BULK_MEMORY_OPT",
    "WAMR_BUILD_CALL_INDIRECT_OVERLONG",
    "WAMR_BUILD_SPEC_TEST",
    "WAMR_BUILD_REF_TYPES",
    "WAMR_BUILD_WASI_NN_TFLITE",
    "WAMR_BUILD_WASI_NN_OPENVINO",
    "WAMR_BUILD_WASI_NN_ONNX",
    "WAMR_BUILD_WASI_NN_LLAMACPP",
    "WAMR_BUILD_WASI_NN_EPHEMERAL_NN",
    "WAMR_BUILD_WASI_NN_ENABLE_GPU",
    "WAMR_BUILD_WASI_NN_ENABLE_EXTERNAL_DELEGATE",
]

# Implied-feature rules, mirrored from build-scripts/runtime_lib.cmake /
# config_common.cmake: when the key feature is =1, the implied one becomes =1
# too (the cmake configure does this; the list documents it for the F
# expansion and the unit-target compatibility check).
IMPLIED_FEATURES = {
    "WAMR_BUILD_STRINGREF": ["WAMR_BUILD_GC"],
    "WAMR_BUILD_GC": ["WAMR_BUILD_REF_TYPES"],
    "WAMR_BUILD_FAST_JIT": ["WAMR_BUILD_INTERP"],
    "WAMR_BUILD_JIT": ["WAMR_BUILD_INTERP"],
    "WAMR_BUILD_LIB_PTHREAD_SEMAPHORE": ["WAMR_BUILD_LIB_PTHREAD"],
    "WAMR_BUILD_LIBC_BUILTIN": ["WAMR_BUILD_MODULE_INST_CONTEXT"],
    "WAMR_BUILD_LIBC_WASI": ["WAMR_BUILD_MODULE_INST_CONTEXT"],
}

# Running-mode features: controlled by the --mode flags (MODE_BUILD_FLAGS in
# run_coverage.py), not by --feature.  When expanding F they are calibrated
# to the mode; when serializing cmake flags they are skipped.
MODE_FEATURES = [
    "WAMR_BUILD_INTERP",
    "WAMR_BUILD_FAST_INTERP",
    "WAMR_BUILD_JIT",
    "WAMR_BUILD_FAST_JIT",
    "WAMR_BUILD_LAZY_JIT",
    "WAMR_BUILD_AOT",
]

# Running mode -> values of MODE_FEATURES (mirrors test_wamr.sh COMPILE_FLAGS).
MODE_FEATURE_VALUES = {
    "classic-interp": {
        "WAMR_BUILD_INTERP": 1, "WAMR_BUILD_FAST_INTERP": 0,
        "WAMR_BUILD_JIT": 0, "WAMR_BUILD_AOT": 0,
        "WAMR_BUILD_FAST_JIT": 0, "WAMR_BUILD_LAZY_JIT": 0,
    },
    "fast-interp": {
        "WAMR_BUILD_INTERP": 1, "WAMR_BUILD_FAST_INTERP": 1,
        "WAMR_BUILD_JIT": 0, "WAMR_BUILD_AOT": 0,
        "WAMR_BUILD_FAST_JIT": 0, "WAMR_BUILD_LAZY_JIT": 0,
    },
    "aot": {
        "WAMR_BUILD_INTERP": 1, "WAMR_BUILD_FAST_INTERP": 0,
        "WAMR_BUILD_JIT": 0, "WAMR_BUILD_AOT": 1,
        "WAMR_BUILD_FAST_JIT": 0, "WAMR_BUILD_LAZY_JIT": 0,
    },
    "jit": {
        "WAMR_BUILD_INTERP": 1, "WAMR_BUILD_FAST_INTERP": 0,
        "WAMR_BUILD_JIT": 1, "WAMR_BUILD_AOT": 1,
        "WAMR_BUILD_FAST_JIT": 0, "WAMR_BUILD_LAZY_JIT": 0,
    },
    # The unit-test run mode name for WAMR_BUILD_JIT=1 (unit_common.cmake
    # wamr_unit_test_get_current_run_mode).  test_wamr.sh calls it 'jit';
    # run_coverage.py maps the label for the spec layer.  Note: the unit
    # configure must NOT pass -DWAMR_BUILD_AOT=1 (the mode validation
    # forbids combining it with JIT), but the llvm-jit runtime compiles the
    # AOT-compiler sources, so its compile macros DO carry WASM_ENABLE_AOT=1
    # - the F calibration below mirrors the macro plane, not the cmake
    # variable plane.
    "llvm-jit": {
        "WAMR_BUILD_INTERP": 1, "WAMR_BUILD_FAST_INTERP": 0,
        "WAMR_BUILD_JIT": 1, "WAMR_BUILD_AOT": 1,
        "WAMR_BUILD_FAST_JIT": 0, "WAMR_BUILD_LAZY_JIT": 0,
    },
    "fast-jit": {
        "WAMR_BUILD_INTERP": 1, "WAMR_BUILD_FAST_INTERP": 0,
        "WAMR_BUILD_JIT": 0, "WAMR_BUILD_AOT": 0,
        "WAMR_BUILD_FAST_JIT": 1, "WAMR_BUILD_LAZY_JIT": 0,
    },
    "multi-tier-jit": {
        "WAMR_BUILD_INTERP": 1, "WAMR_BUILD_FAST_INTERP": 0,
        "WAMR_BUILD_JIT": 1, "WAMR_BUILD_AOT": 0,
        "WAMR_BUILD_FAST_JIT": 1, "WAMR_BUILD_LAZY_JIT": 1,
    },
}


def parse_feature_flags(features: str) -> dict:
    """Parse '-DWAMR_BUILD_GC=1 -DWAMR_BUILD_REF_TYPES=1' into
    {'WAMR_BUILD_GC': 1, 'WAMR_BUILD_REF_TYPES': 1}.

    An unknown feature name is rejected: it would silently weaken the
    compatibility and denominator checks instead of failing."""
    enabled = {}
    for token in features.split():
        name, sep, value = token.partition("=")
        if not name.startswith("-D") or not sep or value not in ("0", "1"):
            raise ValueError(
                f"Bad feature switch '{token}'; expected -DWAMR_BUILD_XXX=0|1"
            )
        name = name[2:]
        if name not in ALL_FEATURES:
            raise ValueError(
                f"Unknown feature '{name}'; known features are listed in "
                f"ALL_FEATURES (coverage_features.py)"
            )
        enabled[name] = int(value)
    return enabled


def expand_features(features: str, mode: str = "classic-interp") -> dict:
    """Return the complete, explicit feature set F for a report:
    {feature_name: 0|1} for every feature in ALL_FEATURES.  The running-mode
    features are calibrated to the given mode.

    An empty `features` string returns the mode calibration only: every other
    feature stays a wildcard (absent from the dict), so the compatibility
    check constrains nothing."""
    if mode not in MODE_FEATURE_VALUES:
        raise KeyError(
            f"Unknown running mode '{mode}'; known modes: "
            f"{sorted(MODE_FEATURE_VALUES)}"
        )
    enabled = parse_feature_flags(features)
    if not enabled:
        return dict(MODE_FEATURE_VALUES[mode])
    on = {name for name, value in enabled.items() if value}
    # apply implications until a fixpoint (e.g. JIT -> INTERP, GC -> REF_TYPES)
    changed = True
    while changed:
        changed = False
        for feature in list(on):
            for implied in IMPLIED_FEATURES.get(feature, []):
                if implied not in on:
                    on.add(implied)
                    changed = True
    f = {name: (1 if name in on else 0) for name in ALL_FEATURES}
    # calibrate the running-mode features to the mode
    for name, val in MODE_FEATURE_VALUES[mode].items():
        f[name] = val
    return f


def feature_flags_for(f: dict, skip_mode_features: bool = True) -> str:
    """Serialize F to '-DWAMR_BUILD_XXX=0/1 ...' cmake flags (sorted).

    With skip_mode_features=True the running-mode features are omitted: they
    are passed separately via MODE_BUILD_FLAGS in run_coverage.py.
    """
    names = sorted(f)
    if skip_mode_features:
        names = [n for n in names if n not in MODE_FEATURES]
    return " ".join(f"-D{name}={f[name]}" for name in names)


if __name__ == "__main__":
    import sys
    for features in sys.argv[1:] or [""]:
        f = expand_features(features)
        enabled = sorted(k for k, v in f.items() if v == 1)
        print(f"{features or '(none)'}: enabled={enabled}")
