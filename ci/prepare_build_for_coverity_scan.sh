#!/usr/bin/env bash
set -xv

# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# Pre-requisites
# - check cov-build is available
# - check file is available

# Start the build process
WAMR_DIR=/workspaces/wasm-micro-runtime
CMAKE_BUILD_DIR=build-4-cov-analysis

# Function to build wamrc
coverity_build_wamrc() {
    local options="$1"
    local output_dir="$2"
    echo "Building wamrc with options: $options @ $output_dir"

    rm -rf $output_dir
    cmake -S ${WAMR_DIR}/wamr-compiler -B $output_dir \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=Debug \
        $options
    cov-build --dir cov-int --append-log cmake --build $output_dir
    if [[ $? != 0 ]]; then
        echo "Failed to build wamrc with options: $options"
        exit 1
    fi

}

# Function to build iwasm
coverity_build_iwasm() {
    local options="$1"
    local output_parent_dir="$2"
    local output_dir=$(mktemp -d -p $output_parent_dir iwasm-XXXX)

    echo "Building iwasm with options: $options @ $output_dir"

    cmake -S ${WAMR_DIR}/product-mini/platforms/linux -B $output_dir \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=Debug \
        $options
    cov-build --dir cov-int --append-log cmake --build $output_dir --target iwasm
    if [[ $? != 0 ]]; then
        echo "Failed to build iwasm with options: $options"
        exit 1
    fi
}

# List of compilation options for wamrc
wamrc_options_list=(
    #
    #default
    ""
)

# List of compilation options for iwasm
iwasm_options_list=(
    #default
    ""
    # +classic interp
    "-DWAMR_BUILD_FAST_INTERP=0 -DWAMR_BUILD_SIMD=0"
    # fast jit
    "-DWAMR_BUILD_FAST_JIT=1 -DWAMR_BUILD_FAST_JIT_DUMP=1 -DWAMR_BUILD_SIMD=0"
    # +llvm jit
    "-DWAMR_BUILD_JIT=1"
    #
    "-DWAMR_BUILD_TARGET=X86_32"
    #
    # libraries
    "-DWAMR_BUILD_LIBC_BUILTIN=0 -DWAMR_BUILD_LIBC_UVWASI=1 -DWAMR_BUILD_LIBC_EMCC=1"
    "-DWAMR_BUILD_THREAD_MGR=1 -DWAMR_BUILD_LIB_PTHREAD=1      -DWAMR_BUILD_LIB_PTHREAD_SEMAPHORE=1"
    "-DWAMR_BUILD_THREAD_MGR=1 -DWAMR_BUILD_LIB_WASI_THREADS=1 -DWAMR_BUILD_LIB_PTHREAD_SEMAPHORE=1"
    "-DWAMR_BUILD_WASI_NN=1 -DWAMR_BUILD_WASI_NN_LLAMACPP=1"
    #
    # Wasm specs
    "-DWAMR_BUILD_GC=1 -DWAMR_BUILD_STRINGREF=1 -DWAMR_STRINGREF_IMPL_SOURCE=STUB"
    "-DWAMR_BUILD_EXCE_HANDLING=1 -DWAMR_BUILD_AOT=0 -DWAMR_BUILD_FAST_INTERP=0 -DWAMR_BUILD_SIMD=0"
    "-DWAMR_BUILD_MEMORY64=1 -DWAMR_BUILD_MULTI_MEMORY=1 -DWAMR_BUILD_SHARED_MEMORY=1 -DWAMR_BUILD_FAST_INTERP=0 -DWAMR_BUILD_SIMD=0 -DWAMR_BUILD_AOT=0"
    #
    # WARM features
    "-DWAMR_BUILD_MULTI_MODULE=1 -DWAMR_BUILD_MINI_LOADER=1 -DWAMR_BUILD_SHARED_HEAP=1"
    "-DWAMR_DISABLE_HW_BOUND_CHECK=1"
    "-DWAMR_CONFIGURABLE_BOUNDS_CHECKS=1"
    "-DWAMR_BUILD_EXTENDED_CONST_EXPR=1"
    # - Debug
    "-DWAMR_BUILD_DEBUG_INTERP=1 -DWAMR_BUILD_DEBUG_AOT=1 -DWAMR_BUILD_DYNAMIC_AOT_DEBUG=1"
    # - developer options
    "-DWAMR_BUILD_CUSTOM_NAME_SECTION=1  -DWAMR_BUILD_LOAD_CUSTOM_SECTION=1 -DWAMR_BUILD_DUMP_CALL_STACK=1 -DWAMR_BUILD_LINUX_PERF=1 -DWAMR_BUILD_AOT_VALIDATOR=1 -DWAMR_BUILD_MEMORY_PROFILING=1 -DWAMR_BUILD_PERF_PROFILING=1"
    # - global heap
    "-DWAMR_BUILD_ALLOC_WITH_USER_DATA=1  -DWAMR_BUILD_GLOBAL_HEAP_POOL=1 -DWAMR_BUILD_GLOBAL_HEAP_SIZE=131072"
    "-DWAMR_BUILD_QUICK_AOT_ENTRY=0 -DWAMR_DISABLE_WAKEUP_BLOCKING_OP=1 -DWAMR_BUILD_MODULE_INST_CONTEXT=0"
    # - pgo
    "-DWAMR_BUILD_STATIC_PGO=1"
    # TODO: SGX specifics.
)

rm -rf cov-int
rm -rf ${CMAKE_BUILD_DIR}
mkdir -p ${CMAKE_BUILD_DIR}

# # Loop through all iwasm options and build
for options in "${iwasm_options_list[@]}"; do
    coverity_build_iwasm "$options" $CMAKE_BUILD_DIR
done

# # Loop through all wamrc options and build
for options in "${wamrc_options_list[@]}"; do
    coverity_build_wamrc "$options" ${CMAKE_BUILD_DIR}/wamrc
done

grep "compilation units" cov-int/build-log.txt

branch=$(git branch --show-current)
commit=$(git rev-parse --short HEAD)
tar caf wamr-${branch}-${commit}.xz cov-int

set +xv
