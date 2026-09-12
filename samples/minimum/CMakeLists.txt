# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.20)

project(minimum C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(WAMR_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)
set(WAMR_BUILD_INTERP 1)
include(${WAMR_ROOT}/build-scripts/runtime_lib.cmake)
add_library(vmlib ${WAMR_RUNTIME_LIB_SOURCE})
