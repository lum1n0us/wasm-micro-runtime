# Copyright (C) 2026 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Standalone CMake project for the basic sample's wasm application.
# Built by the parent project via samples_build_wasm_app() (ExternalProject)
# so wasm compilation flags stay isolated from the host build.
#
# Tools: wasi-sdk (REQUIRED - missing wasi-sdk makes configure fail).

cmake_minimum_required(VERSION 3.14)
project(basic_wasm_apps LANGUAGES C)

set(WAMR_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
list(APPEND CMAKE_MODULE_PATH "${WAMR_ROOT_DIR}/build-scripts")
find_package(WASISDK REQUIRED)

set(CMAKE_SYSTEM_PROCESSOR wasm32)
set(CMAKE_SYSROOT "${WAMR_ROOT_DIR}/wamr-sdk/app/libc-builtin-sysroot")
set(CMAKE_C_COMPILER "${WASISDK_CC_COMMAND}")
set(CMAKE_C_COMPILER_TARGET "wasm32")
set(CMAKE_C_FLAGS "-nostdlib -Qunused-arguments -O0 -z stack-size=4096")
set(CMAKE_EXE_LINKER_FLAGS
    "-Wl,--initial-memory=65536,                 \
     -Wl,--no-entry,--strip-all,                 \
     -Wl,--allow-undefined-file=${WAMR_ROOT_DIR}/wamr-sdk/app/libc-builtin-sysroot/share/defined-symbols.txt, \
     -Wl,--export=generate_float,                \
     -Wl,--export=float_to_string,               \
     -Wl,--export=calculate,                     \
     -Wl,--export=mul7,                          \
     -Wl,--allow-undefined"
)

add_executable(testapp.wasm testapp.c)
target_link_libraries(testapp.wasm)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/testapp.wasm" DESTINATION wasm-apps)
