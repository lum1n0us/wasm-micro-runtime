# Copyright (C) 2026 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Standalone CMake project for the import-func-callback sample's wasm
# application. Built by the parent project via samples_build_wasm_app()
# (ExternalProject).
#
# Tools: wasi-sdk (REQUIRED - missing wasi-sdk makes configure fail).

cmake_minimum_required(VERSION 3.14)
project(import_func_callback_wasm_apps LANGUAGES NONE)

set(WAMR_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
list(APPEND CMAKE_MODULE_PATH "${WAMR_ROOT_DIR}/build-scripts")
find_package(WASISDK REQUIRED)

set(WASM_OUT "${CMAKE_CURRENT_BINARY_DIR}/test.wasm")

add_custom_command(
  OUTPUT ${WASM_OUT}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ${WASISDK_CC_COMMAND}
          -nostdlib
          --target=wasm32
          -Wl,--no-entry
          -Wl,--export=test
          -Wl,--allow-undefined
          -o ${WASM_OUT}
          "${CMAKE_CURRENT_SOURCE_DIR}/test.c"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test.c"
  COMMENT "Building test.wasm"
  VERBATIM
)
add_custom_target(import_func_callback_wasm ALL DEPENDS ${WASM_OUT})

install(FILES ${WASM_OUT} DESTINATION wasm-apps)
