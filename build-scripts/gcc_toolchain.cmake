# Copyright (C) 2025 Intel Corporation. All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# Check for GCC C compiler
find_program(GCC_C_COMPILER NAMES gcc)
if(NOT GCC_C_COMPILER)
    message(FATAL_ERROR "GCC C compiler not found. Please install GCC.")
else()
    message(STATUS "GCC C compiler found: ${GCC_C_COMPILER}")
    set(CMAKE_C_COMPILER ${GCC_C_COMPILER})
endif()

# Check for GCC C++ compiler
find_program(GCC_CXX_COMPILER NAMES g++)
if(NOT GCC_CXX_COMPILER)
    message(FATAL_ERROR "GCC C++ compiler not found. Please install GCC.")
else()
    message(STATUS "GCC C++ compiler found: ${GCC_CXX_COMPILER}")
    set(CMAKE_CXX_COMPILER ${GCC_CXX_COMPILER})
endif()

# Check for GCC assembler
find_program(GCC_ASM_COMPILER NAMES as)
if(NOT GCC_ASM_COMPILER)
    message(FATAL_ERROR "GCC assembler not found. Please install binutils (as).")
else()
    message(STATUS "GCC assembler found: ${GCC_ASM_COMPILER}")
    set(CMAKE_ASM_COMPILER ${GCC_ASM_COMPILER})
endif()
