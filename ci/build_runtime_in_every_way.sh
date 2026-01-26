#!/usr/bin/env bash

#
# Enhanced build-for-cov/try_run.sh with improved structure and fail-fast error handling
# Compatible with .github/scripts/codeql_buildscript.sh patterns
#

set -o errexit
set -o errtrace
set -o nounset
set -o pipefail
# set -o verbose
# set -o xtrace

# ANSI color codes for highlighting
readonly RED='\033[0;31m'
readonly GREEN='\033[0;32m'
readonly YELLOW='\033[1;33m'
readonly BLUE='\033[0;34m'
readonly NC='\033[0m' # No Color
readonly BOLD='\033[1m'

# ==============================================================================
# Utility Functions
# ==============================================================================

# Adds some sparkle to build announcements ✨
announce_build_wizardry() {
    local build_name="$1"
    echo -e "${BLUE}${BOLD}🚀 Conjuring build magic for: ${build_name}${NC}"
}

# Because nobody likes silent failures 💥
rage_quit_on_failure() {
    local build_name="$1"
    local exit_code="$2"
    local phase="${3:-build}"

    echo -e "${RED}${BOLD}💀 BUILD EXPLOSION DETECTED! 💥${NC}"
    echo -e "${RED}${phase^} phase for '${build_name}' crashed and burned with exit code: ${exit_code}${NC}"
    echo -e "${YELLOW}Time to debug like a detective! 🔍${NC}"
    exit $exit_code
}

# Victory dance for successful builds 🎉
celebrate_build_triumph() {
    local build_name="$1"
    echo -e "${GREEN}${BOLD}✅ Victory! ${build_name} built successfully! 🎊${NC}"
}

# ==============================================================================
# Flag Definitions - The Sacred Scrolls of Configuration 📜
# ==============================================================================

# The holy grail of common flags - shared by all mere mortals
declare -r UNIVERSAL_CMAKE_INCANTATIONS=(
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    -DCMAKE_BUILD_TYPE=Debug
    -DCMAKE_CXX_FLAGS="-O0 -fno-inline-functions -g3"
    -DCMAKE_C_FLAGS="-O0 -fno-inline-functions -g3"
    -G"Ninja"
)

# The mystical toolchain of coverage enlightenment
declare -r COVERAGE_TOOLCHAIN_SPELL=(
    -DCMAKE_TOOLCHAIN_FILE=../../tests/fuzz/wasm-mutator-fuzz/clang_toolchain.cmake
)

declare -r WASM_PROPOSALS=(
    -DWAMR_BUILD_BULK_MEMORY=1
    -DWAMR_BUILD_BULK_MEMORY_OPT=1
    -DWAMR_BUILD_EXTENDED_CONST_EXPR=1
    -DWAMR_BUILD_CALL_INDIRECT_OVERLONG=1
    -DWAMR_BUILD_MEMORY64=1
    -DWAMR_BUILD_REF_TYPES=1
    -DWAMR_BUILD_SHARED_MEMORY=1
    -DWAMR_BUILD_SIMD=1
    -DWAMR_BUILD_CUSTOM_NAME_SECTION=1
    -DWAMR_BUILD_LOAD_CUSTOM_SECTION=1
    -DWAMR_BUILD_GC=1
    -DWAMR_BUILD_GC_HEAP_VERIFY=1
    -DWMAR_BUILD_GC_PERF_PROFILING=1
    -DWAMR_BUILD_STRINGREF=1
    -DWAMR_STRINGREF_IMPL_SOURCE=STUB
    -DWAMR_BUILD_TAIL_CALL=1
    -DWAMR_BUILD_LIME1=1
    -DWAMR_BUILD_EXCE_HANDLING=1
    -DWAMR_BUILD_MULTI_MEMORY=1
)

declare -r WASI_SUPPORT=(
    -DWAMR_BUILD_LIBC_WASI=1
    -DWAMR_BUILD_LIB_WASI_THREADS=1
    -DWAMR_BUILD_THREAD_MGR=1
    -DWAMR_BUILD_LIB_PTHREAD=1
    -DWAMR_BUILD_LIB_PTHREAD_SEMAPHORE=1
)

declare -r NATIVE_SUPPORT=(
    -DWAMR_BUILD_LIBC_BUILTIN=1
    -DWAMR_BUILD_THREAD_MGR=1
    -DWAMR_BUILD_LIB_PTHREAD=1
    -DWAMR_BUILD_LIB_PTHREAD_SEMAPHORE=1
    -DWAMR_BUILD_LIBC_EMCC=1
    -DWAMR_BUILD_LIBC_UVWASI=1
)

declare -r WASI_NN_SUPPORT=(
    -DWAMR_BUILD_WASI_NN=1
    -DWAMR_BUILD_WASI_EPHEMERAL_NN=1
    -DWAMR_BUILD_WASI_NN_LLAMACPP=1
)

# FIXME: Static PGO build doesn't use coverage toolchain due to os_atomic_cmpxchg() issue
declare -r PGO_SUPPORT=(
    -DWAMR_BUILD_STATIC_PGO=1
)

declare -r RUNTIME_SPECIFIC=(
    -DWAMR_BUILD_GLOBAL_HEAP_POOL=1
    -DWAMR_BUILD_GLOBAL_HEAP_SIZE=1
    -DWAMR_BUILD_MODULE_INST_CONTEXT=1
    -DWAMR_BUILD_SHRUNK_MEMORY=1
    -DWAMR_BUILD_ALLOC_WITH_USAGE=1
    -DWAMR_BUILD_ALLOC_WITH_USER_DATA=1
    -DWAMR_BUILD_COPY_CALL_STACK=1
    -DWAMR_BUILD_DEBUG_INTERP=1
    -DWAMR_BUILD_DUMP_CALL_STACK=1
    -DWAMR_BUILD_INVOKE_NATIVE_GENERAL=1
    -DWAMR_BUILD_LINUX_PERF=1
    -DWAMR_BUILD_MEMORY_PROFILING=1
    -DWAMR_BUILD_MEMORY_TRACING=1
    -DWAMR_BUILD_MULTI_MODULE=1
    -DWAMR_BUILD_PERF_PROFILING=1
    -DWAMR_BUILD_SHARED_HEAP=1
    -DWAMR_BUILD_STACK_GUARD_SIZE=1
    -DWAMR_BUILD_INSTRUCTION_METERING=1
    -DWAMR_BUILD_WASM_CACHE=1
)

declare -r IWASM_AOT=(
    -DWAMR_BUILD_AOT=1
    ${WASM_PROPOSALS[@]}
    ${WASI_SUPPORT[@]}
    ${NATIVE_SUPPORT[@]}
    ${RUNTIME_SPECIFIC[@]}
    ${PGO_SUPPORT[@]}
    # unsupported in AOT build yet
    -DWAMR_BUILD_EXCE_HANDLING=0
    -DWAMR_BUILD_MULTI_MEMORY=0
)

declare -r IWASM_CLASSIC_INTERP=(
    -DWAMR_BUILD_AOT=0
    -DWAMR_BUILD_FAST_INTERP=0
    ${WASM_PROPOSALS[@]}
    ${WASI_SUPPORT[@]}
    ${NATIVE_SUPPORT[@]}
    ${RUNTIME_SPECIFIC[@]}
    # unsupported in classic interp build yet
    -DWAMR_BUILD_SIMD=0
)

declare -r IWASM_FAST_INTERP=(
    -DWAMR_BUILD_AOT=0
    ${WASM_PROPOSALS[@]}
    ${WASI_SUPPORT[@]}
    ${NATIVE_SUPPORT[@]}
    ${RUNTIME_SPECIFIC[@]}
)

declare -r IWASM_LLVM_JIT=(
    -DWAMR_BUILD_AOT=0
    -DWAMR_BUILD_JIT=1
    -DWAMR_BUILD_LAZY_JIT=1
    ${WASM_PROPOSALS[@]}
    ${WASI_SUPPORT[@]}
    ${NATIVE_SUPPORT[@]}
    ${RUNTIME_SPECIFIC[@]}
    # unsupported in LLVM JIT build yet
    -DWAMR_BUILD_EXCE_HANDLING=0
    -DWAMR_BUILD_MEMORY64=0
    -DWAMR_BUILD_MULTI_MEMORY=0
    -DWAMR_BUILD_MULTI_MODULE=0
)

declare -r IWASM_FAST_JIT=(
    -DWAMR_BUILD_AOT=0
    -DWAMR_BUILD_FAST_JIT=1
    ${WASM_PROPOSALS[@]}
    ${WASI_SUPPORT[@]}
    ${NATIVE_SUPPORT[@]}
    ${RUNTIME_SPECIFIC[@]}
    # unsupported in FAST JIT build yet
    -DWAMR_BUILD_EXCE_HANDLING=0
    -DWAMR_BUILD_GC=0
    -DWAMR_BUILD_MEMORY64=0
    -DWAMR_BUILD_MULTI_MEMORY=0
    -DWAMR_BUILD_MULTI_MODULE=0
    -DWAMR_BUILD_LINUX_PERF=0
    -DWAMR_BUILD_STRINGREF=0
    -DWAMR_BUILD_SHARED_HEAP=0
)

# # iwasm-a: The kitchen sink configuration - everything but the kitchen sink 🚰
# declare -r IWASM_ALPHA_ARSENAL=(
#     -DWAMR_BUILD_AOT=0
#     -DWAMR_BUILD_FAST_INTERP=1
#     -DWAMR_BUILD_BULK_MEMORY=1
#     -DWAMR_BUILD_CUSTOM_NAME_SECTION=1
#     -DWAMR_BUILD_EXTENDED_CONST_EXPR=1
#     -DWAMR_BUILD_LIBC_WASI=1
#     -DWAMR_BUILD_LIB_WASI_THREADS=1
#     -DWAMR_BUILD_LOAD_CUSTOM_SECTION=1
#     -DWAMR_BUILD_MEMORY64=1
#     -DWAMR_BUILD_REF_TYPES=1
#     -DWAMR_BUILD_SHARED_MEMORY=1
#     -DWAMR_BUILD_SIMD=1
#     -DWAMR_BUILD_GLOBAL_HEAP_POOL=1
#     -DWAMR_BUILD_GLOBAL_HEAP_SIZE=1
#     -DWAMR_BUILD_LIBC_BUILTIN=1
#     -DWAMR_BUILD_MODULE_INST_CONTEXT=1
#     -DWAMR_BUILD_SHRUNK_MEMORY=1
#     -DWAMR_BUILD_THREAD_MGR=1
#     -DWAMR_BUILD_GC=1
#     -DWAMR_BUILD_STRINGREF=1
#     -DWAMR_STRINGREF_IMPL_SOURCE=STUB
#     -DWAMR_BUILD_TAIL_CALL=1
#     -DWAMR_BUILD_ALLOC_WITH_USAGE=1
#     -DWAMR_BUILD_ALLOC_WITH_USER_DATA=1
#     -DWAMR_BUILD_BULK_MEMORY_OPT=1
#     -DWAMR_BUILD_CALL_INDIRECT_OVERLONG=1
#     -DWAMR_BUILD_COPY_CALL_STACK=1
#     -DWAMR_BUILD_DEBUG_INTERP=1
#     -DWAMR_BUILD_DUMP_CALL_STACK=1
#     -DWAMR_BUILD_GC_HEAP_VERIFY=1
#     -DWAMR_BUILD_INVOKE_NATIVE_GENERAL=1
#     -DWAMR_BUILD_LIB_PTHREAD=1
#     -DWAMR_BUILD_LIB_PTHREAD_SEMAPHORE=1
#     -DWAMR_BUILD_LIME1=1
#     -DWAMR_BUILD_LINUX_PERF=1
#     -DWAMR_BUILD_MEMORY_PROFILING=1
#     -DWAMR_BUILD_MEMORY_TRACING=1
#     -DWAMR_BUILD_MULTI_MODULE=1
#     -DWAMR_BUILD_PERF_PROFILING=1
#     -DWAMR_BUILD_SHARED_HEAP=1
#     -DWAMR_BUILD_STACK_GUARD_SIZE=1
#     -DWAMR_BUILD_EXCE_HANDLING=1
#     -DWAMR_BUILD_MULTI_MEMORY=1
#     -DWAMR_BUILD_INSTRUCTION_METERING=1
#     -DWAMR_BUILD_LIBC_EMCC=1
#     -DWAMR_BUILD_LIBC_UVWASI=1
#     -DWAMR_BUILD_MINI_LOADER=0
#     -DWAMR_BUILD_WASM_CACHE=1
#     -DWMAR_BUILD_GC_PERF_PROFILING=1
# )

# # iwasm-b: AOT powerhouse configuration 💪
# declare -r IWASM_BRAVO_BATTALION=(
#     -DWAMR_BUILD_AOT=1
#     -DWAMR_BUILD_SIMD=1
#     -DWAMR_BUILD_SPEC_TEST=1
#     -DWAMR_BUILD_LINUX_PERF=1
#     -DWAMR_BUILD_DUMP_CALL_STACK=1
#     -DWAMR_BUILD_PERF_PROFILING=1
#     -DWAMR_BUILD_AOT_STACK_FRAME=1
#     -DWAMR_BUILD_SHARED_MEMORY=1
#     -DWAMR_BUILD_COPY_CALL_STACK=1
#     -DWAMR_BUILD_LOAD_CUSTOM_SECTION=1
#     -DWAMR_BUILD_CUSTOM_NAME_SECTION=1
#     -DWAMR_BUILD_STRINGREF=1
#     -DWAMR_STRINGREF_IMPL_SOURCE=STUB
#     -DWAMR_BUILD_MEMORY_PROFILING=1
#     -DWAMR_BUILD_MEMORY_TRACING=1
#     -DWAMR_BUILD_MULTI_MODULE=1
#     -DWAMR_BUILD_GC=1
# )

# # iwasm-c: WASI-NN neural network configuration 🤖
# declare -r IWASM_CHARLIE_COGNITION=(
#     -DWAMR_BUILD_WASI_NN=1
#     -DWAMR_BUILD_WASI_EPHEMERAL_NN=1
#     -DWAMR_BUILD_WASI_NN_LLAMACPP=1
# )

# # iwasm-d: Static PGO configuration ⚡
# declare -r IWASM_DELTA_DYNAMO=(
#     -DWAMR_BUILD_STATIC_PGO=1
# )

# # iwasm-e: Fast/JIT configuration 🏎️
# declare -r IWASM_ECHO_EXPRESS=(
#     -DWAMR_BUILD_FAST_JIT=1
#     -DWAMR_BUILD_JIT=1
#     -DWAMR_BUILD_LAZY_JIT=1
#     -DWAMR_BUILD_SIMD=0
#     -DWAMR_BUILD_FAST_INTERP=0
#     -DWAMR_BUILD_SHARED_MEMORY=1
# )

# wamrc-a: Compiler configuration 🔧
declare -r WAMRC_ALPHA_ALCHEMY=()

# ==============================================================================
# Build Functions - Where the Magic Happens ✨
# ==============================================================================

# The ultimate iwasm building contraption
orchestrate_iwasm_symphony() {
    local build_codename="$1"
    local -n specific_spells_ref=$2
    local use_coverage_toolchain="$3"

    announce_build_wizardry "$build_codename"

    # Prepare the build directory - out with the old! 🧹
    rm -rf "$build_codename"

    # Assemble the mystical flag array
    local combined_incantations=()
    combined_incantations+=("${UNIVERSAL_CMAKE_INCANTATIONS[@]}")

    # Add coverage toolchain spell if requested
    if [[ "$use_coverage_toolchain" == "true" ]]; then
        combined_incantations+=("${COVERAGE_TOOLCHAIN_SPELL[@]}")
    fi

    combined_incantations+=("${specific_spells_ref[@]}")

    # Invoke the cmake configuration with proper error handling
    set +e  # Temporarily disable errexit to catch errors
    cmake \
        -S ../product-mini/platforms/linux \
        -B "$build_codename" \
        "${combined_incantations[@]}"
    local cmake_config_result=$?
    set -e  # Re-enable errexit

    if [[ $cmake_config_result -ne 0 ]]; then
        rage_quit_on_failure "$build_codename" "$cmake_config_result" "configuration"
    fi

    # Now attempt the build
    set +e  # Temporarily disable errexit to catch errors
    cmake --build "$build_codename" --target vmlib
    local cmake_build_result=$?
    set -e  # Re-enable errexit

    if [[ $cmake_build_result -ne 0 ]]; then
        rage_quit_on_failure "$build_codename" "$cmake_build_result" "compilation"
    fi

    celebrate_build_triumph "$build_codename"
}

# The wamrc compiler building apparatus
forge_wamrc_masterpiece() {
    local build_codename="$1"
    local -n specific_spells_ref=$2
    local use_coverage_toolchain="$3"

    announce_build_wizardry "$build_codename"

    # Clear the slate
    rm -rf "$build_codename"

    # Construct the flag ensemble
    local combined_incantations=()
    combined_incantations+=("${UNIVERSAL_CMAKE_INCANTATIONS[@]}")

    # Add coverage toolchain spell if requested
    if [[ "$use_coverage_toolchain" == "true" ]]; then
        combined_incantations+=("${COVERAGE_TOOLCHAIN_SPELL[@]}")
    fi

    combined_incantations+=("${specific_spells_ref[@]}")

    # Cast the cmake configuration spell
    set +e  # Temporarily disable errexit to catch errors
    cmake \
        -S ../wamr-compiler \
        -B "$build_codename" \
        "${combined_incantations[@]}"
    local cmake_config_result=$?
    set -e  # Re-enable errexit

    if [[ $cmake_config_result -ne 0 ]]; then
        rage_quit_on_failure "$build_codename" "$cmake_config_result" "configuration"
    fi

    # Now attempt the build
    set +e  # Temporarily disable errexit to catch errors
    cmake --build "$build_codename" --target vmlib aotclib
    local cmake_build_result=$?
    set -e  # Re-enable errexit

    if [[ $cmake_build_result -ne 0 ]]; then
        rage_quit_on_failure "$build_codename" "$cmake_build_result" "compilation"
    fi

    celebrate_build_triumph "$build_codename"
}

# ==============================================================================
# Main Execution - Let the Build Fest Begin! 🎪
# ==============================================================================

echo -e "${BOLD}${BLUE}🎭 Welcome to the Grand Build Spectacular! 🎭${NC}"
echo -e "${YELLOW}Preparing to build multiple WAMR configurations...${NC}"

orchestrate_iwasm_symphony "iwasm-aot" IWASM_AOT "false"

# iwasm-a: The comprehensive coverage configuration
orchestrate_iwasm_symphony "iwasm-ci" IWASM_CLASSIC_INTERP "true"

# iwasm-b: The AOT powerhouse
orchestrate_iwasm_symphony "iwasm-fi" IWASM_FAST_INTERP "true"

# iwasm-c: The neural network specialist
orchestrate_iwasm_symphony "iwasm-lj" IWASM_LLVM_JIT "true"

# iwasm-d: The PGO optimization build
orchestrate_iwasm_symphony "iwasm-fj" IWASM_FAST_JIT "aot"

# iwasm-e: The JIT speed demon
orchestrate_iwasm_symphony "iwasm-nn" WASI_NN_SUPPORT "true"

# wamrc-a: The compiler creation station
forge_wamrc_masterpiece "wamrc-a" WAMRC_ALPHA_ALCHEMY "true"

echo -e "${GREEN}${BOLD}🎉 ALL BUILDS COMPLETED SUCCESSFULLY! 🏆${NC}"
echo -e "${BLUE}The build festival has concluded. Time for some well-deserved rest! 😴${NC}"
