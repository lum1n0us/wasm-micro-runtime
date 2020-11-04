# Copyright (C) 2019 Intel Corporation. All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required (VERSION 3.0)

project(bwa_wasm)

################  WASI SYSROOT  ################
find_path(WASI_SYSROOT
  wasi-sysroot
  PATHS /opt/wasi-sdk-11.0/share /opt/wasi-sdk/share
  REQUIRED
)

if(NOT WASI_SYSROOT)
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

if(NOT CLANG_11)
  message(FATAL_ERROR
    "can not find clang-11. "
    "please install it with a script from https://apt.llvm.org/llvm.sh "
    "and install llvm-11 likes ./llvm.sh 11"
  )
endif()

################  EMCC ################
if(NOT DEFINED ENV{EMSDK})
  message(FATAL_ERROR
    "can not find emsdk. "
    "please refer to https://emscripten.org/docs/getting_started/downloads.html "
    "and install it, "
    "or active emsdk by 'source ./emsdk_env.sh'"
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

#######################################
include(ExternalProject)

################ HEADERS ################
ExternalProject_Add(headers_from_emcc
   PREFIX headers
   SOURCE_DIR "$ENV{EMSDK}/upstream/emscripten/system/include/SSE"
   CONFIGURE_COMMAND ""
   BUILD_COMMAND  ""
   INSTALL_COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/include/SSE
                     && cmake -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/include/pthread/sys
                     && cmake -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/include/emscripten
                     # copy emscripten SSE header files
                     && cmake -E copy $ENV{EMSDK}/upstream/emscripten/system/include/SSE/immintrin.h ${CMAKE_CURRENT_SOURCE_DIR}/include/SSE/
                     # SSE
                     && cmake -E copy $ENV{EMSDK}/upstream/emscripten/system/include/SSE/xmmintrin.h ${CMAKE_CURRENT_SOURCE_DIR}/include/SSE/
                     # SSE2
                     && cmake -E copy $ENV{EMSDK}/upstream/emscripten/system/include/SSE/emmintrin.h ${CMAKE_CURRENT_SOURCE_DIR}/include/SSE/
                     # SSE4.1
                     && cmake -E copy $ENV{EMSDK}/upstream/emscripten/system/include/SSE/smmintrin.h ${CMAKE_CURRENT_SOURCE_DIR}/include/SSE/
                     # a fake empty header to aovid further depenency
                     && cmake -E touch ${CMAKE_CURRENT_SOURCE_DIR}/include/emscripten/emscripten.h
                     # copy emscripten pthread related header files
                     && cmake -E copy $ENV{EMSDK}/upstream/emscripten/system/include/libc/pthread.h ${CMAKE_CURRENT_SOURCE_DIR}/include/pthread/
                     && cmake -E copy $ENV{EMSDK}/upstream/emscripten/system/include/libc/signal.h ${CMAKE_CURRENT_SOURCE_DIR}/include/pthread/
                     && cmake -E copy $ENV{EMSDK}/upstream/emscripten/system/include/libc/netdb.h ${CMAKE_CURRENT_SOURCE_DIR}/include/pthread/
                     && cmake -E copy $ENV{EMSDK}/upstream/emscripten/system/include/libc/sys/wait.h ${CMAKE_CURRENT_SOURCE_DIR}/include/pthread/sys/
                     && cmake -E copy $ENV{EMSDK}/upstream/emscripten/system/include/libc/sys/socket.h ${CMAKE_CURRENT_SOURCE_DIR}/include/pthread/sys/
)

################ libz ################
ExternalProject_Add(libz_src
  PREFIX libz
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG        master
  GIT_PROGRESS   ON
  GIT_SHALLOW    ON
  SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/libz
  CONFIGURE_COMMAND ""
  BUILD_COMMAND  ""
  INSTALL_COMMAND ""
)

################ bwa ################
ExternalProject_Add(bwa
  PREFIX bwa
  GIT_REPOSITORY https://github.com/lh3/bwa.git
  GIT_TAG        master
  GIT_PROGRESS   ON
  GIT_SHALLOW    ON
  SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/bwa
  DEPENDS        libz_src headers_from_emcc
  UPDATE_COMMAND git clean -fd && git checkout -- *
                  && cmake -E echo "Copying pre-installed CMakeLists.txt"
                  && cmake -E copy ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.bwa_wasm.txt CMakeLists.txt
  INSTALL_COMMAND cmake -E copy ./bwa.opt.wasm ${CMAKE_CURRENT_SOURCE_DIR}/build/bwa.wasm
)
