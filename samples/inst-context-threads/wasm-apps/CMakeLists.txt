# Copyright (C) 2026 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Standalone CMake project for the inst-context-threads sample's wasm
# application. Built by the parent project via samples_build_wasm_app()
# (ExternalProject).
#
# Tools: wasi-sdk (REQUIRED - missing wasi-sdk makes configure fail).

cmake_minimum_required(VERSION 3.14)
project(inst_context_threads_wasm_apps LANGUAGES NONE)

set(WAMR_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
list(APPEND CMAKE_MODULE_PATH "${WAMR_ROOT_DIR}/build-scripts")
find_package(WASISDK REQUIRED)

set(WASM_OUT "${CMAKE_CURRENT_BINARY_DIR}/testapp.wasm")

# wasm32-wasi-threads needs the wasi-libc pthread model; compile and link in
# a single clang invocation (like the removed build.sh) with the wasi-sdk
# pthread toolchain flags.
add_custom_command(
  OUTPUT ${WASM_OUT}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ${WASISDK_CC_COMMAND}
          --target=wasm32-wasi-threads
          -pthread
          -Wl,--import-memory
          -Wl,--export-memory
          -Wl,--max-memory=655360
          -o ${WASM_OUT}
          "${CMAKE_CURRENT_SOURCE_DIR}/testapp.c"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/testapp.c"
  COMMENT "Building testapp.wasm"
  VERBATIM
)
add_custom_target(inst_context_threads_wasm ALL DEPENDS ${WASM_OUT})

install(FILES ${WASM_OUT} DESTINATION wasm-apps)
