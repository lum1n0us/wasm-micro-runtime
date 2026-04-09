cmake_minimum_required(VERSION 3.14)

message(STATUS "Configuring end-to-end tests")

# Check for wat2wasm (WABT)
find_program(WAT2WASM wat2wasm)
if(NOT WAT2WASM)
    message(WARNING "wat2wasm not found. E2E tests will be skipped. "
                    "Install WABT: https://github.com/WebAssembly/wabt")
    return()
endif()

# Detect platform
if(NOT WAMR_BUILD_PLATFORM)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(WAMR_BUILD_PLATFORM "linux")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(WAMR_BUILD_PLATFORM "darwin")
    else()
        message(WARNING "Unsupported platform for E2E tests: ${CMAKE_SYSTEM_NAME}")
        return()
    endif()
endif()

# Locate wamrc and iwasm
set(WAMRC_PATH "${CMAKE_BINARY_DIR}/wamr-compiler/wamrc")
set(IWASM_PATH "${CMAKE_BINARY_DIR}/product-mini/platforms/${WAMR_BUILD_PLATFORM}/iwasm")

# Set environment for test scripts
set(E2E_TEST_ENV
    "WAMRC=${WAMRC_PATH}"
    "IWASM=${IWASM_PATH}"
    "WAT2WASM=${WAT2WASM}"
    "E2E_COMMON=${CMAKE_CURRENT_SOURCE_DIR}/common"
)

# Add test suites
add_subdirectory(gc-ref-null)
add_subdirectory(aot-gc-compatibility)
