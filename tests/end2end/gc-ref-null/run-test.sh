#!/bin/bash
# End-to-end test for ref.null GC/non-GC parsing
#
# Usage: run-test.sh <wat_file> <enable_gc> <compile_expect> <run_expect>
#   wat_file: WAT source file name (in test-cases/)
#   enable_gc: "true" or "false"
#   compile_expect: "success" or "fail"
#   run_expect: "success", "fail", or "skip"

set -e

# Load common helpers
source "${E2E_COMMON}/test-helpers.sh"

# Parse arguments
WAT_FILE="${1:?Missing WAT file argument}"
ENABLE_GC="${2:?Missing enable_gc argument}"
COMPILE_EXPECT="${3:?Missing compile_expect argument}"
RUN_EXPECT="${4:?Missing run_expect argument}"

# Validate arguments
validate_bool "${ENABLE_GC}" "enable_gc"
validate_expect "${COMPILE_EXPECT}" "compile_expect"
if [ "${RUN_EXPECT}" != "skip" ]; then
    validate_expect "${RUN_EXPECT}" "run_expect"
fi

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

if "${WAMRC}" ${GC_FLAG} -o "${AOT_FILE}" "${WASM_FILE}" 2>"${WORK_DIR}/wamrc.err"; then
    COMPILE_RESULT="success"
else
    COMPILE_RESULT="fail"
fi

# Verify compilation result
if [ "${COMPILE_RESULT}" != "${COMPILE_EXPECT}" ]; then
    log_error "Compilation expectation mismatch"
    echo "  Expected: ${COMPILE_EXPECT}"
    echo "  Got: ${COMPILE_RESULT}"
    if [ "${COMPILE_RESULT}" = "fail" ]; then
        echo "  Error output:"
        cat "${WORK_DIR}/wamrc.err"
    fi
    exit 1
fi

log_success "Compilation: ${COMPILE_RESULT} (as expected)"

# Step 3: Run AOT (if needed)
if [ "${RUN_EXPECT}" = "skip" ]; then
    log_success "Test passed (execution skipped)"
    exit 0
fi

if [ "${COMPILE_RESULT}" = "fail" ]; then
    log_success "Test passed (compilation failed as expected)"
    exit 0
fi

log_step "Running AOT module"
if "${IWASM}" "${AOT_FILE}" >"${WORK_DIR}/iwasm.out" 2>"${WORK_DIR}/iwasm.err"; then
    RUN_RESULT="success"
else
    RUN_RESULT="fail"
fi

# Verify run result
if [ "${RUN_RESULT}" != "${RUN_EXPECT}" ]; then
    log_error "Execution expectation mismatch"
    echo "  Expected: ${RUN_EXPECT}"
    echo "  Got: ${RUN_RESULT}"
    echo "  Output:"
    cat "${WORK_DIR}/iwasm.out"
    echo "  Error:"
    cat "${WORK_DIR}/iwasm.err"
    exit 1
fi

log_success "Test passed (execution: ${RUN_RESULT})"
exit 0
