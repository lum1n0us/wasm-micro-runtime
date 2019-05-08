#
# INTEL CONFIDENTIAL
#
# Copyright 2017-2018 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials,
# and your use of them is governed by the express license under which they
# were provided to you (License). Unless the License provides otherwise, you
# may not use, modify, copy, publish, distribute, disclose or transmit this
# software or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.
#
cmake_minimum_required (VERSION 2.8.3)
message ("host-agent...")
project (host-agent)

if (NOT CONNECTION)
    message(FATAL_ERROR "-DCONNECTION=AEE_TCP or -DCONNECTION=AEE_UART")
endif (NOT CONNECTION)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-literal-suffix ")
SET(VERIFIER_FLAGS " -DNVALGRIND -DLINUX -DBV_USE_FOR_DEBUG")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")

if (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug)
endif (NOT CMAKE_BUILD_TYPE)

if (NOT TARGET_PLATFORM)
  set (TARGET_PLATFORM "linux")
endif (NOT TARGET_PLATFORM)
message ("TARGET_PLATFORM = " ${TARGET_PLATFORM})



set(REPO_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../../..)
set(SSG_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib)
set(WASM_DIR  ${REPO_ROOT_DIR}/core/iwasm)
set(APP_MGR_DIR ${REPO_ROOT_DIR}/core/app-mgr)
set(SHARED_DIR ${REPO_ROOT_DIR}/core/shared-lib)


include (${WASM_DIR}/lib/native-interface/native_interface.cmake)
include (${APP_MGR_DIR}/app-mgr-shared/app_mgr_shared.cmake)
include (${SHARED_DIR}/platform/${TARGET_PLATFORM}/shared_platform.cmake)
include (${SHARED_DIR}/utils/shared_utils.cmake)
include (${SHARED_DIR}/mem-alloc/mem_alloc.cmake)
include (${SHARED_DIR}/coap/lib_coap.cmake)
include (${SSG_LIB_DIR}/host_agent_lib.cmake)


set(AON_SOURCE
    ${AON_DIR}/src/libaon.c
    )

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/src
    )

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
endif ()


add_definitions(-Wall -Wno-pointer-sign -DRUN_ON_LINUX=1 -D${CONNECTION} -DMALLOC_MEMORY_FROM_SYSTEM)

file (GLOB_RECURSE HOST_AGENT_SRC src/*.c )


SET(SOURCES
    ${HOST_AGENT_SRC}
    ${PLATFORM_SHARED_SOURCE}
    ${UTILS_SHARED_SOURCE}
    ${MEM_ALLOC_DIR}/bh_memory.c
    ${NATIVE_INTERFACE_SOURCE}
    ${LIB_COAP_SOURCE}
    ${LIB_HOST_AGENT_SOURCE}
    )
    
add_executable(host-agent ${SOURCES})
target_link_libraries(host-agent pthread)
