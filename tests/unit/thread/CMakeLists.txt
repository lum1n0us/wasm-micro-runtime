# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

wamr_unit_test_suite_run_modes(thread MODES classic-interp aot)
if(NOT WAMR_UNIT_TEST_SUITE_ENABLED)
  return()
endif()

add_subdirectory(pthread)
add_subdirectory(wasi-thread)
