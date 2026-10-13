# Copyright (C) 2026 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

if(WAMR_BUILD_TARGET STREQUAL "X86_32")
  set(LINEAR_MEMORY_AOT_TARGET_FLAGS --target=i386)
else()
  set(LINEAR_MEMORY_AOT_TARGET_FLAGS)
endif()

add_subdirectory(memory32-sw)
add_subdirectory(memory32-sw-shared-heap)

# X86_32 has no hardware bound check and no memory64
if(NOT WAMR_BUILD_TARGET STREQUAL "X86_32")
  add_subdirectory(memory32-hw)
  add_subdirectory(memory32-hw-shared-heap)
  add_subdirectory(memory64-sw)
  add_subdirectory(memory64-sw-shared-heap)
endif()
