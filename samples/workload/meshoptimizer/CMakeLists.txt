# Copyright (C) 2019 Intel Corporation. All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required (VERSION 3.0)

project(bench-meshoptimizer)

################  WASI SYSROOT  ################
find_path(WASI_SYSROOT
  wasi-sysroot
  PATHS /opt/wasi-sdk-11.0/share /opt/wasi-sdk/share
  REQUIRED
)

if (NOT WASI_SYSROOT)
  message(FATAL_ERROR
    "can not find wasi sysroot. "
    "please download it from "
    "https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-11/wasi-sdk-11.0-linux.tar.gz "
    "and install it under /opt"
  )
endif()

################  CLANG 11 ################
find_program(CLANG_11
  clang-11
  PATHS /usr/lib/llvm-11/bin
  REQUIRED
)

if (NOT CLANG_11)
  message(FATAL_ERROR
    "can not find clang-11. "
    "please install it with a script from https://apt.llvm.org/llvm.sh "
    "and install llvm-11 likes ./llvm.sh 11"
  )
endif()

################  BINARYEN ################
find_program(WASM_OPT
    NAMES wasm-opt
    PATHS /opt/binaryen-version_97/bin /opt/binaryen/bin
)

if (NOT WASM_OPT)
  message(FATAL_ERROR
    "can not find wasm-opt. "
    "please download it from "
    "https://github.com/WebAssembly/binaryen/releases/download/version_97/binaryen-version_97-x86_64-linux.tar.gz "
    "and install it under /opt"
  )
endif()

################  MESHOPTIMIZER  ################
include(ExternalProject)

ExternalProject_Add(codecbench
  PREFIX codecbench
  GIT_REPOSITORY https://github.com/zeux/meshoptimizer.git
  GIT_TAG        master
  GIT_SHALLOW    ON
  GIT_PROGRESS   ON
  SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/meshoptimizer
  UPDATE_COMMAND git clean -fd && git checkout -- *
                   && cmake -E echo "Applying patch"
                   && git apply ${CMAKE_CURRENT_SOURCE_DIR}/codecbench.patch
  BUILD_COMMAND  make codecbench codecbench.opt
  INSTALL_COMMAND cmake -E copy ./codecbench.opt.wasm ${CMAKE_CURRENT_SOURCE_DIR}/build/codecbench.wasm
)
