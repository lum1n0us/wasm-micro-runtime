#!/bin/bash

# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

source /opt/emsdk/emsdk_env.sh

PLATFORM=$(uname -s | tr A-Z a-z)

WORK_DIR=$PWD
OUT_DIR=$PWD/out
WAMRC_CMD=$PWD/../../../wamr-compiler/build/wamrc
JETSTREAM_DIR=$PWD/perf-automation/benchmarks/JetStream2/wasm

mkdir -p ${OUT_DIR}

if [[ $1 != "--no-simd" ]];then
    NATIVE_SIMD_FLAGS="-msse2 -msse3 -msse4"
    WASM_SIMD_FLAGS="-msimd128"
else
    NATIVE_SIMD_FLAGS=""
    WASM_SIMD_FLAGS=""
fi

if [ ! -d perf-automation ]; then
    git clone https://github.com/mozilla/perf-automation.git
    echo "Patch source files .."
    cd perf-automation
    patch -p1 -N < ../jetstream.patch
fi

cd ${JETSTREAM_DIR}

# Build gcc-loops

echo "Build gcc-loops with g++ .."
g++ -O3 ${NATIVE_SIMD_FLAGS} -o ${OUT_DIR}/gcc-loops_native gcc-loops.cpp

echo "Build gcc-loops with em++ .."
em++ -O3 -s STANDALONE_WASM=1 ${WASM_SIMD_FLAGS} \
         -s INITIAL_MEMORY=1048576 \
         -s TOTAL_STACK=32768 \
         -s "EXPORTED_FUNCTIONS=['_main']" \
         -s ERROR_ON_UNDEFINED_SYMBOLS=0 \
         -o ${OUT_DIR}/gcc-loops.wasm gcc-loops.cpp

echo "Compile gcc-loops.wasm to gcc-loops.aot"
${WAMRC_CMD} -o ${OUT_DIR}/gcc-loops.aot ${OUT_DIR}/gcc-loops.wasm

if [[ ${PLATFORM} == "linux" ]]; then
    echo "Compile gcc-loops.wasm to gcc-loops_segue.aot"
    ${WAMRC_CMD} --enable-segue -o ${OUT_DIR}/gcc-loops_segue.aot ${OUT_DIR}/gcc-loops.wasm
fi

# Build quicksort

echo "Build quicksort with gcc .."
gcc -O3 ${NATIVE_SIMD_FLAGS} -o ${OUT_DIR}/quicksort_native quicksort.c

echo "Build quicksort with emcc .."
emcc -O3 -s STANDALONE_WASM=1 ${WASM_SIMD_FLAGS} \
         -s INITIAL_MEMORY=1048576 \
         -s TOTAL_STACK=32768 \
         -s "EXPORTED_FUNCTIONS=['_main']" \
         -o ${OUT_DIR}/quicksort.wasm quicksort.c

echo "Compile quicksort.wasm to quicksort.aot"
${WAMRC_CMD} -o ${OUT_DIR}/quicksort.aot ${OUT_DIR}/quicksort.wasm

if [[ ${PLATFORM} == "linux" ]]; then
    echo "Compile quicksort.wasm to quicksort_segue.aot"
    ${WAMRC_CMD} --enable-segue -o ${OUT_DIR}/quicksort_segue.aot ${OUT_DIR}/quicksort.wasm
fi

# Build HashSet

echo "Build HashSet with g++ .."
g++ -O3 ${NATIVE_SIMD_FLAGS} -o ${OUT_DIR}/HashSet_native HashSet.cpp \
        -lstdc++

echo "Build HashSet with em++ .."
em++ -O3 -s STANDALONE_WASM=1 ${WASM_SIMD_FLAGS} \
         -s INITIAL_MEMORY=1048576 \
         -s TOTAL_STACK=32768 \
         -s "EXPORTED_FUNCTIONS=['_main']" \
         -o ${OUT_DIR}/HashSet.wasm HashSet.cpp

echo "Compile HashSet.wasm to HashSet.aot"
${WAMRC_CMD} -o ${OUT_DIR}/HashSet.aot ${OUT_DIR}/HashSet.wasm

if [[ ${PLATFORM} == "linux" ]]; then
    echo "Compile HashSet.wasm to HashSet_segue.aot"
    ${WAMRC_CMD} --enable-segue -o ${OUT_DIR}/HashSet_segue.aot ${OUT_DIR}/HashSet.wasm
fi

# Build float-mm

cd ${JETSTREAM_DIR}/../simple

echo "Build float-mm with gcc .."
gcc -O3 ${NATIVE_SIMD_FLAGS} -o ${OUT_DIR}/float-mm_native float-mm.c

echo "Build float-mm with emcc .."
emcc -O3 -s STANDALONE_WASM=1 ${WASM_SIMD_FLAGS} \
         -s INITIAL_MEMORY=1048576 \
         -s TOTAL_STACK=32768 \
         -s "EXPORTED_FUNCTIONS=['_main']" \
         -o ${OUT_DIR}/float-mm.wasm float-mm.c

echo "Compile float-mm.wasm to float-mm.aot"
${WAMRC_CMD} -o ${OUT_DIR}/float-mm.aot ${OUT_DIR}/float-mm.wasm

if [[ ${PLATFORM} == "linux" ]]; then
    echo "Compile float-mm.wasm to float-mm_segue.aot"
    ${WAMRC_CMD} --enable-segue -o ${OUT_DIR}/float-mm_segue.aot ${OUT_DIR}/float-mm.wasm
fi

# Build tsf

cd ${JETSTREAM_DIR}/TSF

tsf_srcs="tsf_asprintf.c tsf_buffer.c tsf_error.c tsf_reflect.c tsf_st.c \
          tsf_type.c tsf_io.c tsf_native.c tsf_generator.c tsf_st_typetable.c \
          tsf_parser.c tsf_buf_writer.c tsf_buf_reader.c tsf_primitive.c \
          tsf_type_table.c tsf_copier.c tsf_destructor.c tsf_gpc_code_gen.c \
          gpc_code_gen_util.c gpc_threaded.c gpc_intable.c gpc_instruction.c \
          gpc_program.c gpc_proto.c gpc_stack_height.c tsf_serial_in_man.c \
          tsf_serial_out_man.c tsf_type_in_map.c tsf_type_out_map.c \
          tsf_stream_file_input.c tsf_stream_file_output.c tsf_sort.c \
          tsf_version.c tsf_named_type.c tsf_io_utils.c tsf_zip_attr.c \
          tsf_zip_reader.c tsf_zip_writer.c tsf_zip_abstract.c tsf_limits.c \
          tsf_ra_type_man.c tsf_adaptive_reader.c tsf_sha1.c tsf_sha1_writer.c \
          tsf_fsdb.c tsf_fsdb_protocol.c tsf_define_helpers.c tsf_ir.c \
          tsf_ir_different.c tsf_ir_speed.c"

echo "Build tsf with gcc .."
gcc \
    -o ${OUT_DIR}/tsf_native -O3 ${NATIVE_SIMD_FLAGS} \
    -I. -DTSF_BUILD_SYSTEM=1 \
    ${tsf_srcs} -lm

echo "Build tsf standalone with wasi-sdk .."
/opt/wasi-sdk/bin/clang -O3 ${WASM_SIMD_FLAGS} -z stack-size=1048576 \
    -Wl,--initial-memory=52428800 \
    -Wl,--export=main \
    -Wl,--export=__heap_base,--export=__data_end \
    -I. -DTSF_BUILD_SYSTEM=1 \
    -Wl,--allow-undefined \
    -o ${OUT_DIR}/tsf.wasm \
    ${tsf_srcs}

echo "Compile tsf.wasm to tsf.aot"
${WAMRC_CMD} -o ${OUT_DIR}/tsf.aot ${OUT_DIR}/tsf.wasm

if [[ ${PLATFORM} == "linux" ]]; then
    echo "Compile tsf.wasm to tsf_segue.aot"
    ${WAMRC_CMD} --enable-segue -o ${OUT_DIR}/tsf_segue.aot ${OUT_DIR}/tsf.wasm
fi
