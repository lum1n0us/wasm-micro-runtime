#!/bin/bash

set -eux

AUTH_KEY=./.devcontainer/coverity/auth-key.txt

OUTPUT_BASE=build-4-cov

rm -rf ${OUTPUT_BASE}
mkdir ${OUTPUT_BASE}

OUTPUT_COV_EMIT=${OUTPUT_BASE}/cov-int

# B for BUILD
OUTPUT_IWASM_DEFAULT=${OUTPUT_BASE}/build-iwasm-default
cmake -S product-mini/platforms/linux -B ${OUTPUT_IWASM_DEFAULT}  -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
cov-build --dir ${OUTPUT_COV_EMIT} cmake --build ${OUTPUT_IWASM_DEFAULT} --target iwasm

# A for ANALYZE
cov-analyze --dir ${OUTPUT_COV_EMIT} --concurrency --security --rule --enable-constraint-fpp --enable-fnptr --enable-virtual --strip-path `pwd`

# C for COMMIT
cov-commit-defects --dir ${OUTPUT_COV_EMIT} --stream "Wamr-v2" --url https://coverity.devtools.intel.com/prod21 --auth-key-file ${AUTH_KEY} --strip-path `pwd` --noxrefs

set +eux
