#!/usr/bin/env python3
"""Modern build orchestration for WAMR runtime variants."""

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from typing import Iterable, List


@dataclass(frozen=True)
class BuildTarget:
    name: str
    source_dir: Path
    cmake_flags: List[str]
    build_targets: List[str]
    use_coverage_toolchain: bool


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build multiple WAMR configurations with clean, logged steps.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        required=True,
        help="Directory for build outputs and logs.",
    )
    parser.add_argument(
        "--project-root",
        type=Path,
        required=True,
        help="Repository root containing product-mini/ and wamr-compiler/.",
    )
    parser.add_argument(
        "--compiler",
        choices=["gcc", "clang"],
        required=True,
        help="Compiler toolchain selection.",
    )
    parser.add_argument(
        "--mode",
        choices=["normal", "coverity"],
        default="normal",
        help="Build mode: normal or coverity.",
    )
    args = parser.parse_args()

    args.output_dir = args.output_dir.resolve()
    args.project_root = args.project_root.resolve()

    if not args.project_root.exists():
        parser.error(f"project root does not exist: {args.project_root}")
    if not (args.project_root / "product-mini" / "platforms" / "linux").exists():
        parser.error("project root is missing product-mini/platforms/linux")
    if not (args.project_root / "wamr-compiler").exists():
        parser.error("project root is missing wamr-compiler")

    return args


def timestamp() -> str:
    return datetime.now().strftime("%Y%m%d_%H%M")


def clean_dir(path: Path) -> None:
    if path.exists():
        shutil.rmtree(path, ignore_errors=True)


def write_log_header(log_file, cmd: Iterable[str], cwd: Path | None) -> None:
    log_file.write(f"Command: {' '.join(cmd)}\n")
    if cwd:
        log_file.write(f"Working directory: {cwd}\n")
    log_file.write("-" * 80 + "\n")
    log_file.flush()


def run_step(step_name: str, cmd: List[str], log_path: Path, cwd: Path | None = None) -> None:
    print(f"[step] {step_name}")
    log_path.parent.mkdir(parents=True, exist_ok=True)
    with log_path.open("w", encoding="utf-8") as log_file:
        write_log_header(log_file, cmd, cwd)
        try:
            subprocess.run(
                cmd,
                cwd=str(cwd) if cwd else None,
                stdout=log_file,
                stderr=subprocess.STDOUT,
                check=True,
                text=True,
            )
        except subprocess.CalledProcessError as exc:
            log_file.flush()
            log_contents = log_path.read_text(encoding="utf-8", errors="replace")
            print(f"[error] {step_name} failed with exit code {exc.returncode}")
            print("[error] Full log:")
            print(log_contents)
            sys.exit(exc.returncode)


def toolchain_flag(project_root: Path, compiler: str, use_coverage_toolchain: bool) -> str:
    if use_coverage_toolchain:
        toolchain = project_root / "tests" / "fuzz" / "wasm-mutator-fuzz" / "clang_toolchain.cmake"
    else:
        toolchain_name = "gcc_toolchain.cmake" if compiler == "gcc" else "clang_toolchain.cmake"
        toolchain = project_root / "build-scripts" / toolchain_name
    if not toolchain.exists():
        raise FileNotFoundError(f"toolchain file not found: {toolchain}")
    return f"-DCMAKE_TOOLCHAIN_FILE={toolchain}"


def build_targets(
    project_root: Path,
    compiler: str,
    mode: str,
    output_dir: Path,
) -> None:
    universal_flags = [
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
        "-DCMAKE_BUILD_TYPE=Debug",
        "-DCMAKE_CXX_FLAGS=-O0 -fno-inline-functions -g3",
        "-DCMAKE_C_FLAGS=-O0 -fno-inline-functions -g3",
        "-G",
        "Ninja",
    ]

    wasm_proposals = [
        "-DWAMR_BUILD_BULK_MEMORY=1",
        "-DWAMR_BUILD_BULK_MEMORY_OPT=1",
        "-DWAMR_BUILD_EXTENDED_CONST_EXPR=1",
        "-DWAMR_BUILD_CALL_INDIRECT_OVERLONG=1",
        "-DWAMR_BUILD_MEMORY64=1",
        "-DWAMR_BUILD_REF_TYPES=1",
        "-DWAMR_BUILD_SHARED_MEMORY=1",
        "-DWAMR_BUILD_SIMD=1",
        "-DWAMR_BUILD_CUSTOM_NAME_SECTION=1",
        "-DWAMR_BUILD_LOAD_CUSTOM_SECTION=1",
        "-DWAMR_BUILD_GC=1",
        "-DWAMR_BUILD_GC_HEAP_VERIFY=1",
        "-DWMAR_BUILD_GC_PERF_PROFILING=1",
        "-DWAMR_BUILD_STRINGREF=1",
        "-DWAMR_STRINGREF_IMPL_SOURCE=STUB",
        "-DWAMR_BUILD_TAIL_CALL=1",
        "-DWAMR_BUILD_LIME1=1",
        "-DWAMR_BUILD_EXCE_HANDLING=1",
        "-DWAMR_BUILD_MULTI_MEMORY=1",
    ]

    wasi_support = [
        "-DWAMR_BUILD_LIBC_WASI=1",
        "-DWAMR_BUILD_LIB_WASI_THREADS=1",
        "-DWAMR_BUILD_THREAD_MGR=1",
        "-DWAMR_BUILD_LIB_PTHREAD=1",
        "-DWAMR_BUILD_LIB_PTHREAD_SEMAPHORE=1",
    ]

    native_support = [
        "-DWAMR_BUILD_LIBC_BUILTIN=1",
        "-DWAMR_BUILD_THREAD_MGR=1",
        "-DWAMR_BUILD_LIB_PTHREAD=1",
        "-DWAMR_BUILD_LIB_PTHREAD_SEMAPHORE=1",
        "-DWAMR_BUILD_LIBC_EMCC=1",
        "-DWAMR_BUILD_LIBC_UVWASI=1",
    ]

    wasi_nn_support = [
        "-DWAMR_BUILD_WASI_NN=1",
        "-DWAMR_BUILD_WASI_EPHEMERAL_NN=1",
        "-DWAMR_BUILD_WASI_NN_LLAMACPP=1",
    ]

    pgo_support = [
        "-DWAMR_BUILD_STATIC_PGO=1",
    ]

    runtime_specific = [
        "-DWAMR_BUILD_GLOBAL_HEAP_POOL=1",
        "-DWAMR_BUILD_GLOBAL_HEAP_SIZE=1",
        "-DWAMR_BUILD_MODULE_INST_CONTEXT=1",
        "-DWAMR_BUILD_SHRUNK_MEMORY=1",
        "-DWAMR_BUILD_ALLOC_WITH_USAGE=1",
        "-DWAMR_BUILD_ALLOC_WITH_USER_DATA=1",
        "-DWAMR_BUILD_COPY_CALL_STACK=1",
        "-DWAMR_BUILD_DEBUG_INTERP=1",
        "-DWAMR_BUILD_DUMP_CALL_STACK=1",
        "-DWAMR_BUILD_INVOKE_NATIVE_GENERAL=1",
        "-DWAMR_BUILD_LINUX_PERF=1",
        "-DWAMR_BUILD_MEMORY_PROFILING=1",
        "-DWAMR_BUILD_MEMORY_TRACING=1",
        "-DWAMR_BUILD_MULTI_MODULE=1",
        "-DWAMR_BUILD_PERF_PROFILING=1",
        "-DWAMR_BUILD_SHARED_HEAP=1",
        "-DWAMR_BUILD_STACK_GUARD_SIZE=1",
        "-DWAMR_BUILD_INSTRUCTION_METERING=1",
        "-DWAMR_BUILD_WASM_CACHE=1",
    ]

    iwasm_aot = [
        "-DWAMR_BUILD_AOT=1",
        *wasm_proposals,
        *wasi_support,
        *native_support,
        *runtime_specific,
        *pgo_support,
        "-DWAMR_BUILD_EXCE_HANDLING=0",
        "-DWAMR_BUILD_MULTI_MEMORY=0",
    ]

    iwasm_classic_interp = [
        "-DWAMR_BUILD_AOT=0",
        "-DWAMR_BUILD_FAST_INTERP=0",
        *wasm_proposals,
        *wasi_support,
        *native_support,
        *runtime_specific,
        "-DWAMR_BUILD_SIMD=0",
    ]

    iwasm_fast_interp = [
        "-DWAMR_BUILD_AOT=0",
        *wasm_proposals,
        *wasi_support,
        *native_support,
        *runtime_specific,
    ]

    iwasm_llvm_jit = [
        "-DWAMR_BUILD_AOT=0",
        "-DWAMR_BUILD_JIT=1",
        "-DWAMR_BUILD_LAZY_JIT=1",
        *wasm_proposals,
        *wasi_support,
        *native_support,
        *runtime_specific,
        "-DWAMR_BUILD_EXCE_HANDLING=0",
        "-DWAMR_BUILD_MEMORY64=0",
        "-DWAMR_BUILD_MULTI_MEMORY=0",
        "-DWAMR_BUILD_MULTI_MODULE=0",
    ]

    iwasm_fast_jit = [
        "-DWAMR_BUILD_AOT=0",
        "-DWAMR_BUILD_FAST_JIT=1",
        *wasm_proposals,
        *wasi_support,
        *native_support,
        *runtime_specific,
        "-DWAMR_BUILD_EXCE_HANDLING=0",
        "-DWAMR_BUILD_GC=0",
        "-DWAMR_BUILD_MEMORY64=0",
        "-DWAMR_BUILD_MULTI_MEMORY=0",
        "-DWAMR_BUILD_MULTI_MODULE=0",
        "-DWAMR_BUILD_LINUX_PERF=0",
        "-DWAMR_BUILD_STRINGREF=0",
        "-DWAMR_BUILD_SHARED_HEAP=0",
    ]

    product_linux = project_root / "product-mini" / "platforms" / "linux"
    wamr_compiler = project_root / "wamr-compiler"

    targets = [
        BuildTarget(
            name="iwasm-aot",
            source_dir=product_linux,
            cmake_flags=iwasm_aot,
            build_targets=["vmlib"],
            use_coverage_toolchain=False,
        ),
        BuildTarget(
            name="iwasm-ci",
            source_dir=product_linux,
            cmake_flags=iwasm_classic_interp,
            build_targets=["vmlib"],
            use_coverage_toolchain=True,
        ),
        BuildTarget(
            name="iwasm-fi",
            source_dir=product_linux,
            cmake_flags=iwasm_fast_interp,
            build_targets=["vmlib"],
            use_coverage_toolchain=True,
        ),
        BuildTarget(
            name="iwasm-lj",
            source_dir=product_linux,
            cmake_flags=iwasm_llvm_jit,
            build_targets=["vmlib"],
            use_coverage_toolchain=True,
        ),
        BuildTarget(
            name="iwasm-fj",
            source_dir=product_linux,
            cmake_flags=iwasm_fast_jit,
            build_targets=["vmlib"],
            use_coverage_toolchain=False,
        ),
        BuildTarget(
            name="iwasm-nn",
            source_dir=product_linux,
            cmake_flags=wasi_nn_support,
            build_targets=["vmlib"],
            use_coverage_toolchain=True,
        ),
        BuildTarget(
            name="wamrc-a",
            source_dir=wamr_compiler,
            cmake_flags=[],
            build_targets=["vmlib", "aotclib"],
            use_coverage_toolchain=True,
        ),
    ]

    build_root = output_dir / "build"
    cov_int = output_dir / "cov-int"

    clean_dir(build_root)
    clean_dir(cov_int)
    build_root.mkdir(parents=True, exist_ok=True)
    if mode == "coverity" and shutil.which("cov-build") is None:
        raise RuntimeError("coverity mode requested but cov-build not found in PATH")

    for target in targets:
        build_dir = build_root / target.name
        clean_dir(build_dir)

        toolchain = toolchain_flag(project_root, compiler, target.use_coverage_toolchain)
        cmake_config_cmd = [
            "cmake",
            "-S",
            str(target.source_dir),
            "-B",
            str(build_dir),
            *universal_flags,
            toolchain,
            *target.cmake_flags,
        ]

        config_log = output_dir / f"{target.name}_configure_{timestamp()}.log"
        run_step(f"{target.name}: configure", cmake_config_cmd, config_log)

        build_cmd = [
            "cmake",
            "--build",
            str(build_dir),
            "--target",
            *target.build_targets,
        ]

        if mode == "coverity":
            build_cmd = ["cov-build", "--dir", str(cov_int), "--append-log", *build_cmd]

        build_log = output_dir / f"{target.name}_build_{timestamp()}.log"
        run_step(f"{target.name}: build", build_cmd, build_log)


def main() -> None:
    args = parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    try:
        build_targets(args.project_root, args.compiler, args.mode, args.output_dir)
    except (RuntimeError, FileNotFoundError) as exc:
        print(f"[error] {exc}")
        sys.exit(1)


if __name__ == "__main__":
    main()
