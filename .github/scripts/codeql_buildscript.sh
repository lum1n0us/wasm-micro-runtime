#!/usr/bin/env bash

#
# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#

# This script is used to build the WAMR project for CodeQL analysis.

# Pre-requisites
sudo apt update
sudo apt install -y build-essential cmake g++-multilib libgcc-12-dev lib32gcc-12-dev ccache ninja-build

LLVM_VER=15.0.6
pushd /opt
sudo wget --progress=dot:giga -O clang+llvm-x86_64-linux-gnu.tar.xz https://github.com/llvm/llvm-project/releases/download/llvmorg-${LLVM_VER}/clang+llvm-${LLVM_VER}-x86_64-linux-gnu-ubuntu-18.04.tar.xz \
  && tar -xf clang+llvm-x86_64-linux-gnu.tar.xz \
  && mv clang+llvm-${LLVM_VER}-x86_64-linux-gnu-ubuntu-18.04 llvm-${LLVM_VER} \
  && rm clang+llvm-x86_64-linux-gnu.tar.xz
popd

# Start the build process
WAMR_DIR=${PWD}
LLVM_DIR=/opt/llvm-${LLVM_VER}/lib/cmake/llvm

# Function to build wamrc
build_wamrc() {
    local options="$1"
    echo "Building wamrc with options: $options"

    pushd ${WAMR_DIR}/wamr-compiler
    rm -rf build
    cmake -S . -B build -DWAMR_BUILD_WITH_CUSTOM_LLVM=1 -DLLVM_DIR=${LLVM_DIR} $options
    cmake --build build --parallel
    if [[ $? != 0 ]]; then
        echo "Failed to build wamrc with options: $options"
        exit 1
    fi
    popd
}

# Function to build iwasm
build_iwasm() {
    local options="$1"
    echo "Building iwasm with options: $options"

    pushd ${WAMR_DIR}/product-mini/platforms/linux
    rm -rf build
    cmake -S . -B build $options
    cmake --build build --parallel
    if [[ $? != 0 ]]; then
        echo "Failed to build iwasm with options: $options"
        exit 1
    fi
    popd
}

# List of compilation options for wamrc
wamrc_options_list=(
    "-DCMAKE_BUILD_TYPE=Debug"
)

# List of compilation options for iwasm
iwasm_options_list=(
    "-DCMAKE_BUILD_TYPE=Debug"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_FAST_INTERP=0"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_JIT=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_FAST_JIT=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_MULTI_MODULE=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_LIB_PTHREAD=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_LIB_WASI_THREADS=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_LIB_MINI_LOADER=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_DEBUG_INTERP=1"


    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_TARGET=X86_32"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_LIB_PTHREAD=1 -DWAMR_BUILD_LIB_PTHREAD_SEMAPHORE=1 -DWAMR_BUILD_MULTI_MODULE=1 -DWAMR_BUILD_SIMD=1 -DWAMR_BUILD_TAIL_CALL=1 -DWAMR_BUILD_REF_TYPES=1 -DWAMR_BUILD_CUSTOM_NAME_SECTION=1 -DWAMR_BUILD_MEMORY_PROFILING=1 -DWAMR_BUILD_PERF_PROFILING=1 -DWAMR_BUILD_DUMP_CALL_STACK=1 -DWAMR_BUILD_LOAD_CUSTOM_SECTION=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_ALLOC_WITH_USER_DATA=1 -DWAMR_DISABLE_STACK_HW_BOUND_CHECK=1 -DWAMR_BUILD_GLOBAL_HEAP_POOL=1 -DWAMR_BUILD_GLOBAL_HEAP_SIZE=131072"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_LIB_WASI_THREADS=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_GC=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_EXCE_HANDLING=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_MEMORY64=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_MULTI_MEMORY=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_DISABLE_HW_BOUND_CHECK=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_QUICK_AOT_ENTRY=0"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_DISABLE_WAKEUP_BLOCKING_OP=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_MODULE_INST_CONTEXT=0 -DWAMR_BUILD_LIBC_BUILTIN=0 -DWAMR_BUILD_LIBC_WASI=0"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_LIBC_UVWASI=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_FAST_JIT=1 -DWAMR_BUILD_FAST_JIT_DUMP=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_MINI_LOADER=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_DEBUG_INTERP=1 -DWAMR_BUILD_DEBUG_AOT=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_STATIC_PGO=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_CONFIGURABLE_BOUNDS_CHECKS=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_LINUX_PERF=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_SHARED_HEAP=1"
    "-DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_DYNAMIC_AOT_DEBUG=1"
)

# Loop through all iwasm options and build
for options in "${iwasm_options_list[@]}"; do
    build_iwasm "$options"
done

# Loop through all iwasm options and build
for options in "${iwasm_options_list[@]}"; do
    build_iwasm "$options"
done
