# Unit Test Template for WAMR

This document suggests a template and guidelines for writing unit test cases for the WAMR (WebAssembly Micro Runtime) project. The template ensures consistency and adheres to the project's coding style.

This BKM takes _running-modes/wasm_running_modes_test.cc_ as a reference.

## CMake Configuration

- **Avoid Input Compilation Flags**: Do not accept input compilation flags. Use predefined macros or `add_definitions` to set necessary flags (e.g., `-DRUN_ON_LINUX`).
- **Dependencies**: Ensure all required dependencies (e.g., LLVM, gtest) are included and linked.
- **Test Discovery**: Use `gtest_discover_tests` for automatic test discovery.

### Example CMake Configuration

```cmake
# Define necessary flags
add_definitions(-DRUN_ON_LINUX)

# Include LLVM and gtest
find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Define the test executable
add_executable(wasm_running_modes_test ${unit_test_sources})

# Link against required libraries
target_link_libraries(wasm_running_modes_test ${LLVM_AVAILABLE_LIBS} gtest_main)

# Discover tests
gtest_discover_tests(wasm_running_modes_test)
```

## Test Structure

### Running Modes Loop

- Use a loop to iterate over all `RunningMode` values.
- Call `wasm_runtime_set_running_mode()` for each mode before executing the test logic.

### Shared Steps

- Implement shared steps for:
  1. **Loading**: Load the WASM module.
  2. **Instantiation**: Instantiate the module.
  3. **Lookup**: Lookup the function to execute.
  4. **Execution**: Execute the function and validate the result.

### Code Style

- **Modular Structure**: Use helper functions for repetitive tasks (e.g., `load_module`, `instantiate_module`, `lookup_function`, `execute_function`).
- **Concise Test Cases**: Keep test cases focused on specific functionality.
- **Error Handling**: Ensure proper error handling and assertions for each step.

## Example Unit Test

Below is an example unit test case that follows the template:

```cpp
#include "gtest/gtest.h"
#include "wasm_runtime.h"

// Helper function to set up the runtime for a specific running mode
void setup_runtime(RunningMode mode) {
    wasm_runtime_set_running_mode(mode);
}

// Helper function to load, instantiate, lookup, and execute a WASM module
void execute_wasm_module(const char* wasm_file, const char* func_name) {
    // Load the WASM module
    auto module = load_module(wasm_file);
    ASSERT_NE(module, nullptr);

    // Instantiate the module
    auto instance = instantiate_module(module);
    ASSERT_NE(instance, nullptr);

    // Lookup the function
    auto func = lookup_function(instance, func_name);
    ASSERT_NE(func, nullptr);

    // Execute the function
    auto result = execute_function(func);
    ASSERT_EQ(result, EXPECTED_RESULT);
}

// Test case
TEST(WasmRunningModesTest, ExecuteUnderAllModes) {
    const char* wasm_file = "test.wasm";
    const char* func_name = "test_func";

    // Loop through all running modes
    for (RunningMode mode : {MODE_INTERPRETER, MODE_JIT, MODE_FAST_JIT}) {
        setup_runtime(mode);
        execute_wasm_module(wasm_file, func_name);
    }
}
```

## Summary

By following this template, you can ensure that your unit test cases are consistent, modular, and adhere to the WAMR project's coding style. This approach simplifies testing across different running modes and ensures shared steps are reusable and maintainable.

Ultimately, all unit cases can be executed at once without re-compilations:

```bash
$ cmake -S . -B build
$ cmake --build build
$ ctest --test-dir build
```
