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

# If required, https://wiki.ith.intel.com/spaces/SecTools/pages/2260108142/Enterprise+Coverity+Help+Topics#EnterpriseCoverityHelpTopics-ExcludeCode can be used to exclude files or directories from analysis

# A for ANALYZE
cov-analyze --dir ${OUTPUT_COV_EMIT} \
  --security --concurrency --enable-constraint-fpp --enable-fnptr --enable-virtual \
  --disable ASSERT_SIDE_EFFECT \
  --disable AUTO_CAUSES_COPY \
  --disable BAD_CHECK_OF_WAIT_COND \
  --disable BAD_SHIFT \
  --disable COPY_INSTEAD_OF_MOVE \
  --disable CUDA.COLLECTIVE_WARP_SHUFFLE_WIDTH \
  --disable CUDA.CUDEVICE_HANDLES \
  --disable CUDA.DEVICE_DEPENDENT \
  --disable CUDA.DEVICE_DEPENDENT_CALLBACKS \
  --disable CUDA.DIVERGENCE_AT_COLLECTIVE_OPERATION \
  --disable CUDA.ERROR_INTERFACE \
  --disable CUDA.ERROR_KERNEL_LAUNCH \
  --disable CUDA.FORK \
  --disable CUDA.INACTIVE_THREAD_AT_COLLECTIVE_WARP \
  --disable CUDA.INITIATION_OBJECT_DEVICE_THREAD_BLOCK \
  --disable CUDA.INVALID_MEMORY_ACCESS \
  --disable CUDA.SHARE_FUNCTION \
  --disable CUDA.SHARE_OBJECT_STREAM_ASSOCIATED \
  --disable CUDA.SPECIFIERS_INCONSISTENCY \
  --disable CUDA.SYNCHRONIZE_TERMINATION \
  --disable INEFFICIENT_RESERVE \
  --disable MISSING_COMMA \
  --disable MISSING_MOVE_ASSIGNMENT \
  --disable OVERLAPPING_COPY \
  --disable STREAM_FORMAT_STATE \
  --disable UNINTENDED_INTEGER_DIVISION \
  --strip-path `pwd`

# C for COMMIT
cov-commit-defects --dir ${OUTPUT_COV_EMIT} --stream "Wamr-v2" --url https://coverity.devtools.intel.com/prod21 --auth-key-file ${AUTH_KEY} --strip-path `pwd` --noxrefs

# https://wiki.ith.intel.com/spaces/SecTools/pages/2260108142/Enterprise+Coverity+Help+Topics#EnterpriseCoverityHelpTopics-Excludeunwantedheaderfilesgettingcommitted tells us the best way to avoid unwanted header files getting committed is from WebUI

set +eux
