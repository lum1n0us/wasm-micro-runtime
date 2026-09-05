#!/usr/bin/env python3

#
# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#

"""Preset feature groups for the coverage report objects (plan §2.3/§2.4).

A "report object" is `(running mode × preset)`.  The user only selects the
running mode; the preset expands to a complete, explicit feature set F
(WAMR_BUILD_* cmake variables, every one set to 0/1):

    F = preset's explicit =1 features
      + implied features (derived from the cmake "if (X EQUAL 1) set (Y 1)"
        rules, mirrored from build-scripts/runtime_lib.cmake)   -- nothing to
        fill here: the cmake configure itself applies the implication, so the
        script only needs the explicit =1 list)
      + everything else explicitly =0  (the full feature checklist)

There is no "unspecified = wildcard" ambiguity: F is always fully enumerated.
"""

# Every WAMR_BUILD_* cmake switch known to the build (baseline = all off).
# Mirrors run_baseline.py DISABLED_FEATURES + ENABLED/EXEMPTED; used to emit
# explicit -DWAMR_BUILD_XXX=0 for every feature a preset does not enable.
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

# Preset groups: name -> set of features explicitly enabled (=1).  Everything
# else in ALL_FEATURES is set to 0; implications are applied on top.
PRESETS = {
    # Plan §2.2 baseline: classic-interp + libc-builtin/shared-heap/
    # global-heap-pool/load-custom-section (+ spec exemptions).
    "baseline": {
        "WAMR_BUILD_LIBC_BUILTIN",
        "WAMR_BUILD_SHARED_HEAP",
        "WAMR_BUILD_GLOBAL_HEAP_POOL",
        "WAMR_BUILD_LOAD_CUSTOM_SECTION",
        "WAMR_BUILD_SPEC_TEST",   # spec exemption
        "WAMR_BUILD_BULK_MEMORY", # spec exemption
        "WAMR_BUILD_REF_TYPES",   # spec exemption
    },
    "gc": {"WAMR_BUILD_GC", "WAMR_BUILD_SPEC_TEST",
           "WAMR_BUILD_BULK_MEMORY", "WAMR_BUILD_REF_TYPES"},
    "memory64": {"WAMR_BUILD_MEMORY64", "WAMR_BUILD_SHARED_MEMORY",
                 "WAMR_BUILD_SPEC_TEST", "WAMR_BUILD_BULK_MEMORY",
                 "WAMR_BUILD_REF_TYPES"},
    "wasi": {"WAMR_BUILD_LIBC_WASI", "WAMR_BUILD_SPEC_TEST",
             "WAMR_BUILD_BULK_MEMORY", "WAMR_BUILD_REF_TYPES"},
    "aot": {"WAMR_BUILD_AOT", "WAMR_BUILD_SPEC_TEST",
            "WAMR_BUILD_BULK_MEMORY", "WAMR_BUILD_REF_TYPES"},
    "jit": {"WAMR_BUILD_JIT", "WAMR_BUILD_SPEC_TEST",
            "WAMR_BUILD_BULK_MEMORY", "WAMR_BUILD_REF_TYPES"},
    "fast-jit": {"WAMR_BUILD_FAST_JIT", "WAMR_BUILD_SPEC_TEST",
                 "WAMR_BUILD_BULK_MEMORY", "WAMR_BUILD_REF_TYPES"},
    # Full-default report object (plan §2.4): no feature is constrained, so
    # every unit suite keeps the feature values its own CMakeLists declares
    # and no -DWAMR_BUILD_* override flags are passed to the configure.
    # expand_preset() special-cases it: F contains only the running-mode
    # calibration, all other features are wildcards (see expand_preset()).
    "default": set(),
}

# Default running mode per preset (plan §2.4 mode→preset mapping).
PRESET_MODES = {
    "baseline": "classic-interp",
    "gc": "classic-interp",
    "memory64": "classic-interp",
    "wasi": "classic-interp",
    "aot": "aot",
    "jit": "llvm-jit",
    "fast-jit": "fast-jit",
}

# Inverse mapping: running mode -> default preset name.  When several presets
# share one mode, the *first* one in this list wins (baseline is the default
# for classic-interp).
MODE_PRESETS = {}
for _name in ["baseline", "gc", "memory64", "wasi", "aot", "jit", "fast-jit"]:
    _mode = PRESET_MODES[_name]
    MODE_PRESETS.setdefault(_mode, _name)


# Running-mode features: controlled by the --mode flags (MODE_BUILD_FLAGS in
# run_coverage.py), not by the preset.  When expanding F they are calibrated
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


def expand_preset(preset: str, mode: str = "classic-interp") -> dict:
    """Return the complete, explicit feature set F for a preset:
    {feature_name: 0|1} for every feature in ALL_FEATURES.  The running-mode
    features are calibrated to the given mode."""
    if preset not in PRESETS:
        raise KeyError(
            f"Unknown preset '{preset}'; known presets: {sorted(PRESETS)}"
        )
    if preset == "default":
        # Plan §2.4 "default" report object: do not adjust any compilation
        # flag.  F constrains only the running-mode features (the mode the
        # report was built for); every other feature is a wildcard, so the
        # compatibility check excludes nothing and run_coverage.py passes no
        # -DWAMR_BUILD_* override flags to the unit configure.
        if mode not in MODE_FEATURE_VALUES:
            raise KeyError(
                f"Unknown running mode '{mode}' for preset 'default'; "
                f"known modes: {sorted(MODE_FEATURE_VALUES)}"
            )
        return dict(MODE_FEATURE_VALUES[mode])
    enabled = set(PRESETS[preset])
    # apply implications until a fixpoint (e.g. JIT -> INTERP, GC -> REF_TYPES)
    changed = True
    while changed:
        changed = False
        for feature in list(enabled):
            for implied in IMPLIED_FEATURES.get(feature, []):
                if implied not in enabled:
                    enabled.add(implied)
                    changed = True
    f = {name: (1 if name in enabled else 0) for name in ALL_FEATURES}
    # calibrate the running-mode features to the mode
    for name, val in MODE_FEATURE_VALUES[mode].items():
        f[name] = val
    return f


def cmake_flags_for(f: dict, skip_mode_features: bool = True) -> str:
    """Serialize F to '-DWAMR_BUILD_XXX=0/1 ...' cmake flags (sorted).

    With skip_mode_features=True the running-mode features are omitted: they
    are passed separately via MODE_BUILD_FLAGS in run_coverage.py.
    """
    names = sorted(f)
    if skip_mode_features:
        names = [n for n in names if n not in MODE_FEATURES]
    return " ".join(f"-D{name}={f[name]}" for name in names)


def validate_mode_preset(mode: str, preset: str) -> str:
    """Check the mode↔preset pairing; if preset is empty, derive it from the
    mode.  Returns the preset name to use."""
    if not preset:
        preset = MODE_PRESETS.get(mode)
        if preset is None:
            raise ValueError(
                f"No default preset for running mode '{mode}'; "
                f"pick one of {sorted(PRESETS)}"
            )
        print(f"Derived preset '{preset}' for mode '{mode}'")
    else:
        default_mode = PRESET_MODES.get(preset)
        if default_mode is not None and default_mode != mode:
            print(f"NOTE: preset '{preset}' normally pairs with mode "
                  f"'{default_mode}', got '{mode}' (report fingerprint will "
                  f"record both)")
    return preset


if __name__ == "__main__":
    import sys
    for name in sorted(PRESETS):
        f = expand_preset(name)
        enabled = sorted(k for k, v in f.items() if v == 1)
        print(f"{name}: enabled={enabled}")
