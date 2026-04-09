#!/bin/bash
# End-to-end test for AOT GC compatibility
#
# Tests that AOT files compiled with/without --enable-gc can only be loaded
# by iwasm binaries built with matching GC configuration.
#
# Usage: run-test.sh <wat_file> <enable_gc> <expect_load>
#   wat_file: WAT source file name (in test-cases/)
#   enable_gc: "true" or "false" - whether to use wamrc --enable-gc
#   expect_load: "success" or "fail" - whether AOT loading should succeed

set -e

# Load common helpers
source "${E2E_COMMON}/test-helpers.sh"

# Parse arguments
WAT_FILE="${1:?Missing WAT file argument}"
ENABLE_GC="${2:?Missing enable_gc argument}"
EXPECT_LOAD="${3:?Missing expect_load argument}"

# Validate arguments
validate_bool "${ENABLE_GC}" "enable_gc"
validate_expect "${EXPECT_LOAD}" "expect_load"

# Check required tools
check_tool "${WAMRC}" "wamrc"
check_tool "${IWASM}" "iwasm"
check_tool "${WAT2WASM}" "wat2wasm"

# Create work directory
WORK_DIR=$(mktemp -d)
trap "rm -rf ${WORK_DIR}" EXIT

WAT_PATH="test-cases/${WAT_FILE}"
WASM_FILE="${WORK_DIR}/${WAT_FILE%.wat}.wasm"
AOT_FILE="${WORK_DIR}/${WAT_FILE%.wat}.aot"

# Step 1: WAT → WASM
log_step "Compiling WAT to WASM"
if ! "${WAT2WASM}" "${WAT_PATH}" -o "${WASM_FILE}" 2>"${WORK_DIR}/wat2wasm.err"; then
    log_error "WAT compilation failed"
    cat "${WORK_DIR}/wat2wasm.err"
    exit 1
fi

# Step 2: WASM → AOT
log_step "Compiling WASM to AOT (enable_gc=${ENABLE_GC})"
GC_FLAG=""
if [ "${ENABLE_GC}" = "true" ]; then
    GC_FLAG="--enable-gc"
fi

if ! "${WAMRC}" ${GC_FLAG} -o "${AOT_FILE}" "${WASM_FILE}" 2>"${WORK_DIR}/wamrc.err"; then
    log_error "AOT compilation failed"
    cat "${WORK_DIR}/wamrc.err"
    exit 1
fi

log_success "AOT compilation successful"

# Step 3: Load and run AOT
log_step "Loading and running AOT module"
if "${IWASM}" "${AOT_FILE}" >"${WORK_DIR}/iwasm.out" 2>"${WORK_DIR}/iwasm.err"; then
    LOAD_RESULT="success"
else
    LOAD_RESULT="fail"
fi

# Verify load result
if [ "${LOAD_RESULT}" != "${EXPECT_LOAD}" ]; then
    log_error "Load expectation mismatch"
    echo "  Expected: ${EXPECT_LOAD}"
    echo "  Got: ${LOAD_RESULT}"

    if [ "${LOAD_RESULT}" = "fail" ]; then
        echo "  Error output:"
        cat "${WORK_DIR}/iwasm.err"

        # Check for expected error message when GC mismatch
        if grep -q "garbage collection is not enabled in this build" "${WORK_DIR}/iwasm.err"; then
            echo "  (Error message indicates GC feature mismatch - expected)"
        fi
    else
        echo "  Output:"
        cat "${WORK_DIR}/iwasm.out"
    fi
    exit 1
fi

log_success "Test passed (load: ${LOAD_RESULT})"

# If load succeeded, verify output is reasonable
if [ "${LOAD_RESULT}" = "success" ]; then
    # The test modules export "main" which returns 42
    # But we don't invoke it, so no output is expected
    log_success "AOT module loaded successfully"
fi

exit 0
