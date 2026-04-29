# Unit Testing Guide for WAMR

This guide provides comprehensive instructions for contributors on creating, running, and maintaining unit tests for WAMR. It covers everything from setting up your environment to writing new test suites and generating coverage reports.

> **For AI Agents**: All commands in this guide show the raw command syntax. See [AGENTS.md](../../AGENTS.md) for platform-specific execution requirements (e.g., devcontainer on Linux).

**Table of Contents:**
- [Development Environment](#development-environment)
- [Quick Start: Your First Unit Test](#quick-start-your-first-unit-test)
- [General Guidelines](#general-guidelines)
- [Writing CMakeLists.txt](#writing-cmakeliststxt-for-the-test-suite)
- [Generating WASM Files](#generating-wasm-files)
- [Initializing Submodules](#initializing-submodules)
- [Compiling and Running Tests](#compiling-and-running-test-cases)
- [Code Coverage](#collecting-code-coverage-data)
- [Example Directory Structure](#example-directory-structure)
- [Best Practices](#additional-notes)
- [Troubleshooting](#troubleshooting)
- [Related Documentation](#related-documentation)

---

## Development Environment

### Prerequisites

The WAMR devcontainer provides all necessary tools:
- Consistent toolchain (compilers, CMake, CTest)
- Pre-installed dependencies (googletest, WASI SDK, LLVM)
- Correct library versions
- Pre-configured build environment

**For platform-specific setup and command execution requirements, see [AGENTS.md](../../AGENTS.md) and [doc/dev-in-container.md](../../doc/dev-in-container.md).**

Before writing unit tests:
1. Read [AGENTS.md](../../AGENTS.md) for environment setup requirements
2. Familiarize yourself with existing tests in `tests/unit/`
3. Choose the appropriate test framework for your needs (see below)

### Choosing a Unit Test Framework

WAMR supports two unit testing approaches. Choose the one that best fits your testing needs:

#### Google Test (Recommended for Most Cases)

**Best for:**
- Comprehensive test coverage similar to spec tests
- Testing complete features or modules
- Integration-style unit tests
- Testing across multiple functions

**Characteristics:**
- C++ test framework with rich assertion library
- Test fixtures for setup/teardown
- Parameterized tests for testing multiple inputs
- Better for testing behavior and workflows
- Examples: Most tests in `tests/unit/`

**Example:**
```cpp
#include "gtest/gtest.h#include "wasm_runtime.h
TEST(InterpreterTest, ExecutesI32AddCorrectly) {
    wasm_module_t module = load_module("add.wasm");
    EXPECT_NE(nullptr, module);
    int result = call_function(module, "add", 2, 3);
    EXPECT_EQ(5, result);
}
```

#### CMocka (For Fine-Grained Testing)

**Best for:**
- Function-level unit tests
- Tests requiring stubs and mocks
- Precise control over test environment
- Testing individual functions in isolation

**Characteristics:**
- C test framework with mocking support
- Built-in function stubbing and mocking
- Fine-grained control over test execution
- Better for testing internal implementation details
- Examples: `tests/unit/mem-alloc/`

**Example:**
```c
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

// Mock function
void* __wrap_malloc(size_t size) {
    check_expected(size);
    return mock_ptr_type(void*);
}

// Test with mock
static void test_allocation_with_mock(void **state) {
    expect_value(__wrap_malloc, size, 1024);
    will_return(__wrap_malloc, test_buffer);
    
    void *result = my_allocate_function(1024);
    assert_ptr_equal(result, test_buffer);
}
```

**Choosing Guidelines:**

| Criteria | Google Test | CMocka |
|----------|-------------|--------|
| Testing scope | Feature/module level | Function level |
| Mock/stub needs | Not required | Frequently needed |
| Language | C++ (tests can test C code) | Pure C |
| Learning curve | Moderate | Moderate |
| WAMR usage | Majority of tests | mem-alloc, specific internals |

**When in doubt, start with Google Test.** It's more flexible for most testing scenarios and matches the testing style used throughout WAMR.

---

## Quick Start: Your First Unit Test

Follow these steps to create and run a simple unit test:

### Step 1: Create Directory Structure

```bash
# Create test directory
mkdir -p tests/unit/my-feature

# Your test directory should contain:
# my-feature/
# ├── CMakeLists.txt         # Build configuration
# ├── my_feature_test.cc     # Test source (C++ with googletest)
# └── wasm-apps/             # Optional: WASM test fixtures
#     ├── CMakeLists.txt
#     └── example.c
```

### Step 2: Write Your Test

Create `my_feature_test.cc`:

```cpp
#include "gtest/gtest.h#include "wasm_runtime.h
// Test fixture for setup/teardown
class MyFeatureTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize WAMR runtime
        wasm_runtime_init();
    }

    void TearDown() override {
        // Cleanup
        wasm_runtime_destroy();
    }
};

// Test case
TEST_F(MyFeatureTest, BasicTest) {
    // Your test code here
    EXPECT_TRUE(true);
}
```

### Step 3: Create CMakeLists.txt

Create `CMakeLists.txt`:

```cmake
# Include common test utilities
include("../unit_common.cmake")

# Add test executable
add_executable(my_feature_test my_feature_test.cc)

# Link WAMR runtime and googletest
target_sources(my_feature_test PRIVATE ${WAMR_RUNTIME_LIB_SOURCE})
target_link_libraries(my_feature_test gtest_main)

# Register test with CTest
add_test(NAME my_feature_test COMMAND my_feature_test)
```

### Step 4: Build and Run

```bash
# Configure build
cd tests/unit && cmake -S . -B build

# Build tests
cd tests/unit && cmake --build build

# Run your test
cd tests/unit && ctest --test-dir build -R my_feature --output-on-failure
```

**Expected output:**
```
Test project /workspaces/wamr/tests/unit/build
    Start 1: my_feature_test
1/1 Test #1: my_feature_test ..................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 1
```

You've now created and run your first unit test!

---

## General Guidelines

### Directory Organization

**Always create a dedicated directory** for new tests under `tests/unit/`:

```
tests/unit/
├── my-feature/           # New test directory
│   ├── CMakeLists.txt
│   ├── my_feature_test.cc
│   └── wasm-apps/        # Optional: test fixtures
```

**Naming conventions:**
- Directory names: lowercase with hyphens (e.g., `exception-handling`, `linear-memory-aot`)
- Test source files: lowercase with underscores (e.g., `exception_test.cc`, `memory_test.cc`)
- Test executables: match the source file name without extension (e.g., `exception_test`)

**Why?** Consistent naming makes it easier to:
- Locate tests related to specific features
- Run tests by name pattern with `ctest -R <pattern>`
- Understand project structure at a glance

### Code Reuse

**Reuse and extend existing test code when possible:**

1. **Check for similar tests first:** Look in existing test directories for related functionality
2. **Extract into test utilities:** If you copy-paste code more than twice, add it to `common/` utilities
3. **Extend fixtures instead of duplicating:** Use googletest's test fixture inheritance

**Example of reusing fixtures:**
```cpp
// Reuse existing RuntimeTest fixture from common/test_helper.h
#include "test_helper.h
class MyFeatureTest : public RuntimeTest {
    // Inherits SetUp() and TearDown() that initialize/cleanup WAMR
protected:
    // Add feature-specific setup
    void SetUpFeature() { /* ... */ }
};
```

**What does "patch them" mean?**
- Modify existing test cases by adding new test scenarios
- Extend test fixtures with additional setup/teardown
- Add helper functions to existing utility files

**When NOT to reuse:**
- Your feature has fundamentally different setup requirements
- Combining tests would make them harder to understand
- The existing test is poorly structured (in this case, refactor it separately)

### WASM Files

**Never commit precompiled `.wasm` files to the repository.** Instead:

**Generate `.wasm` files at build time** from source:
- From `.wat` files (WebAssembly text format) - use `wat2wasm`
- From `.c`/`.cc` files - use WASI SDK toolchain
- Using CMake's `ExternalProject_Add` (see [Generating WASM Files](#generating-wasm-files))

**Why?** 
- `.wasm` files are binary artifacts that bloat the repository
- Source files (`.wat`, `.c`) are human-readable and reviewable
- Generated files can be optimized for different target platforms
- Keeps the repository size manageable

**Exception:** If you need pre-built WASM for CI speed, document WHY in a README and get maintainer approval.

### Test Framework

**Use CTest as the test runner:**

WAMR uses CTest (part of CMake) to discover, run, and report test results. This is already configured in `tests/unit/CMakeLists.txt`.

**Why CTest?**
- Integrated with CMake build system
- Supports parallel test execution (`ctest -j`)
- Standard test filtering and output formatting
- Works across all platforms (Linux, macOS, Windows)
- CI/CD integration without additional tooling

**Use Google Test for test implementation:**

Write test logic using googletest (C++) framework:
- `TEST()` macros for simple tests
- `TEST_F()` for fixture-based tests  
- `EXPECT_*` / `ASSERT_*` for assertions
- Test fixtures for setup/teardown

**Why Google Test?**
- Rich assertion library
- Test fixtures for shared setup/teardown
- Parameterized tests for testing multiple inputs
- Death tests for error handling
- Wide adoption and excellent documentation

---

## Writing CMakeLists.txt for the Test Suite

When creating a `CMakeLists.txt` file for your test suite, follow these best practices to maintain consistency and avoid common pitfalls.

### Rule 1: Do Not Fetch Googletest Again

**Don't do this:**
```cmake
# BAD: Fetching googletest again
include(FetchContent)
FetchContent_Declare(googletest ...)
```

**Why?** The root `tests/unit/CMakeLists.txt` already fetches and configures googletest globally. Re-fetching causes:
- Longer build times
- Version conflicts
- CMake configuration errors
- Duplicate symbols at link time

**The googletest library is already available** as `gtest` and `gtest_main` targets for linking.

### Rule 2: Include unit_common.cmake

**Always include this first:**
```cmake
include("../unit_common.cmake")
```

**What does this provide?**
- WAMR include paths (core/iwasm, core/shared)
- Common compiler flags (warnings, optimizations)
- Platform-specific configurations
- `WAMR_RUNTIME_LIB_SOURCE` variable with all runtime sources

**Why?** This ensures:
- Consistent build configuration across all tests
- No duplicated boilerplate code
- Automatic updates when common config changes

### Rule 3: Use WAMR_RUNTIME_LIB_SOURCE Variable

**Don't do this:**
```cmake
# BAD: Manually listing runtime sources
target_sources(my_test PRIVATE
    ${IWASM_DIR}/common/wasm_runtime_common.c
    ${IWASM_DIR}/interpreter/wasm_runtime.c
    ${IWASM_DIR}/interpreter/wasm_interp_classic.c
    # ... 50+ more files ...
)
```

**Do this instead:**
```cmake
# GOOD: Use the pre-defined variable
target_sources(my_test PRIVATE ${WAMR_RUNTIME_LIB_SOURCE})
```

**Why?**
- The variable includes ALL necessary runtime sources
- Automatically updated when new files are added to the runtime
- Prevents hard-to-debug missing symbol errors
- Keeps your CMakeLists.txt concise

### Rule 4: Find LLVM Only When Needed

**Only include LLVM if your test specifically needs it:**

```cmake
# Only add this if testing AOT/JIT features
find_package(LLVM REQUIRED CONFIG)
llvm_map_components_to_libnames(llvm_libs core native)
target_link_libraries(my_test ${llvm_libs})
```

**When do you need LLVM?**
- Testing AOT compiler features
- Testing JIT execution
- Testing LLVM IR generation

**When you DON'T need LLVM:**
- Testing interpreter-only features
- Testing runtime APIs
- Testing memory management

**Why avoid unnecessary LLVM?**
- Significantly slower builds
- Large dependency (100+ MB)
- Not available on all platforms
- Most unit tests don't need it

### Rule 5: Use Test-Specific Compilation Flags

**Don't do this:**
```cmake
# BAD: Setting global flags in tests/unit/my-feature/CMakeLists.txt
add_compile_options(-DWASM_ENABLE_GC=1 -DWASM_ENABLE_SIMD=1)
```

**Do this instead:**
```cmake
# GOOD: Target-specific flags
target_compile_definitions(my_test PRIVATE
    WASM_ENABLE_GC=1
    WASM_ENABLE_SIMD=1
)
```

**Why?**
- Global flags affect ALL tests in the build, causing:
  - Unexpected test failures in unrelated tests
  - Hard-to-debug configuration issues
  - CI failures that don't reproduce locally
- Target-specific flags only affect your test

**How to know what flags you need?**
Look at the feature you're testing in `core/config.h` or existing CMakeLists.txt files.

### Complete Example CMakeLists.txt

```cmake
# Minimal test suite CMakeLists.txt
cmake_minimum_required(VERSION 3.14)

# Include common configuration (MUST be first)
include("../unit_common.cmake")

# Create test executable
add_executable(my_feature_test
    my_feature_test.cc
)

# Add WAMR runtime sources
target_sources(my_feature_test PRIVATE ${WAMR_RUNTIME_LIB_SOURCE})

# Link googletest (already available from parent)
target_link_libraries(my_feature_test gtest_main)

# Set feature-specific flags if needed
target_compile_definitions(my_feature_test PRIVATE
    WASM_ENABLE_INTERP=1
)

# Register test with CTest
add_test(NAME my_feature_test COMMAND my_feature_test)

# Optional: Build WASM test fixtures
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/wasm-apps)
    include(ExternalProject)
    ExternalProject_Add(
        my_feature_wasm_apps
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wasm-apps
        BUILD_ALWAYS YES
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -S ${CMAKE_CURRENT_SOURCE_DIR}/wasm-apps 
                                           -B build
                                           -DWASI_SDK_PREFIX=${WASI_SDK_DIR}
                                           -DCMAKE_TOOLCHAIN_FILE=${WASISDK_TOOLCHAIN}
        BUILD_COMMAND ${CMAKE_COMMAND} --build build
        INSTALL_COMMAND ${CMAKE_COMMAND} --install build 
                                         --prefix ${CMAKE_CURRENT_BINARY_DIR}/wasm-apps
    )
    add_dependencies(my_feature_test my_feature_wasm_apps)
endif()
```

**This example demonstrates:**
- Proper include order
- Minimal dependencies
- Target-specific configuration
- Optional WASM app generation
- CTest registration

---

## Generating WASM Files

Unit tests often need WASM modules as test fixtures. Generate these at build time from source files.

### Why Generate at Build Time?

**Benefits:**
- WASM sources (`.c`, `.wat`) are human-readable and reviewable
- Tests can be rebuilt for different target architectures
- No binary bloat in the repository
- Changes to WASM test apps are visible in git diffs

**Trade-offs:**
- Slightly longer initial build time
- Requires WASI SDK (pre-installed in devcontainer)

### Directory Structure for WASM Apps

Create a `wasm-apps/` subdirectory in your test directory:

```
tests/unit/my-feature/
├── CMakeLists.txt              # Main test build config
├── my_feature_test.cc          # Test code
└── wasm-apps/                  # WASM test fixtures
    ├── CMakeLists.txt          # WASM build config
    ├── simple.c                # C source → simple.wasm
    ├── complex.c               # C source → complex.wasm
    └── hand_written.wat        # WAT text → hand_written.wasm
```

### Step 1: Create wasm-apps/CMakeLists.txt

This file defines how to build your WASM test fixtures:

```cmake
cmake_minimum_required(VERSION 3.13)
project(my_feature_wasm_apps)

# Build simple.c → simple.wasm
add_executable(simple simple.c)
set_target_properties(simple PROPERTIES SUFFIX .wasm)
install(TARGETS simple DESTINATION .)

# Build complex.c → complex.wasm with specific flags
add_executable(complex complex.c)
set_target_properties(complex PROPERTIES SUFFIX .wasm)
target_compile_options(complex PRIVATE -O2 -g)
install(TARGETS complex DESTINATION .)

# Convert hand_written.wat → hand_written.wasm
# Note: Requires wat2wasm tool (from WABT)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/hand_written.wasm
    COMMAND wat2wasm ${CMAKE_CURRENT_SOURCE_DIR}/hand_written.wat
                     -o ${CMAKE_CURRENT_BINARY_DIR}/hand_written.wasm
    DEPENDS hand_written.wat
    COMMENT "Converting WAT to WASM)
add_custom_target(hand_written_wasm ALL 
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/hand_written.wasm
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hand_written.wasm DESTINATION .)
```

**Key points:**
- Use WASI SDK toolchain (automatically configured when using ExternalProject)
- Set `.wasm` suffix explicitly
- Install to `.` (will be redirected by ExternalProject)

### Step 2: Integrate in Main CMakeLists.txt

Add this to your test's `CMakeLists.txt`:

```cmake
# After defining your test executable...

# Build WASM apps as an external project
include(ExternalProject)
ExternalProject_Add(
    my_feature_wasm_apps                    # Project name
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wasm-apps
    BUILD_ALWAYS YES                         # Rebuild if sources change
    CONFIGURE_COMMAND ${CMAKE_COMMAND} 
        -S ${CMAKE_CURRENT_SOURCE_DIR}/wasm-apps 
        -B build
        -DWASI_SDK_PREFIX=${WASI_SDK_DIR}           # WASI SDK location
        -DCMAKE_TOOLCHAIN_FILE=${WASISDK_TOOLCHAIN} # WASI toolchain
    BUILD_COMMAND ${CMAKE_COMMAND} --build build
    INSTALL_COMMAND ${CMAKE_COMMAND} 
        --install build 
        --prefix ${CMAKE_CURRENT_BINARY_DIR}/wasm-apps  # Output directory
)

# Ensure WASM apps are built before test executable runs
add_dependencies(my_feature_test my_feature_wasm_apps)
```

**What this does:**
1. Configures `wasm-apps/` as a separate CMake project
2. Uses WASI SDK toolchain to compile C/C++ to WASM
3. Installs built `.wasm` files to `build/my-feature/wasm-apps/`
4. Ensures WASM files are built before running tests

### Step 3: Access WASM Files in Tests

The built WASM files will be in your test's build directory:

```cpp
TEST_F(MyFeatureTest, LoadWASM) {
    // Path relative to test executable location
    const char *wasm_path = "wasm-apps/simple.wasm";
    
    // Or use absolute path constructed at compile time
    #define WASM_APPS_DIR "${CMAKE_CURRENT_BINARY_DIR}/wasm-apps    
    // Load the WASM module
    FILE *file = fopen(wasm_path, "rb");
    ASSERT_NE(file, nullptr) << "Failed to open " << wasm_path;
    
    // ... rest of test ...
}
```

**Tip:** Define the path as a CMake variable and pass it to your test:

```cmake
# In CMakeLists.txt
target_compile_definitions(my_feature_test PRIVATE
    WASM_APPS_DIR="${CMAKE_CURRENT_BINARY_DIR}/wasm-apps)
```

```cpp
// In test file
TEST_F(MyFeatureTest, LoadWASM) {
    std::string wasm_path = std::string(WASM_APPS_DIR) + "/simple.wasm";
    // ...
}
```

### Example C Source for WASM Test Fixtures

Create `wasm-apps/simple.c`:

```c
// Simple WASM app for testing basic functionality
#include <stdio.h>

// Exported function that tests can call
__attribute__((export_name("add")))
int add(int a, int b) {
    return a + b;
}

__attribute__((export_name("print_hello")))
void print_hello() {
    printf("Hello from WASM!\n");
}

// Main function (optional, for testing as standalone WASM)
int main() {
    printf("Test WASM module initialized\n");
    return 0;
}
```

### Example WAT File

Create `wasm-apps/minimal.wat`:

```wat
(module
  ;; Export a simple function for testing
  (func $add (export "add") (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.add
  )
  
  ;; Memory for testing memory operations
  (memory (export "memory") 1)
)
```

### Verifying WASM Files Were Built

After building, check the output:

```bash
ls -la tests/unit/build/my-feature/wasm-apps/```

**Expected output:**
```
simple.wasm
complex.wasm
hand_written.wasm
```

### Troubleshooting WASM Generation

**Problem:** WASM files not found at runtime

**Solution:** Check build directory structure:
```bash
find tests/unit/build -name '*.wasm'```

**Problem:** WASI SDK not found

**Solution:** The devcontainer has WASI SDK pre-installed. If running outside the container, set:
```bash
export WASI_SDK_DIR=/opt/wasi-sdk
```

**Problem:** wat2wasm command not found

**Solution:** Install WABT tools (pre-installed in devcontainer):
```bash
which wat2wasm"  # Should print path
```

---

## Initializing Submodules

Test suite `llm-enhanced-test` is maintained in separate repository and included as git submodule. You need to initialize it before building.

```bash
git submodule update --init --recursive
```

Alternatively, if you haven't cloned the repository yet, use `--recursive` when cloning:

```bash
git clone --recursive https://github.com/bytecodealliance/wasm-micro-runtime.git
```

---

## Compiling and Running Test Cases

All commands in this section MUST use the container wrapper for AI agents.

### Basic Test Workflow

Follow these steps to build and run unit tests:

#### Step 1: Configure Build

Generate CMake build configuration:

```bash
# For AI agents
cd tests/unit && cmake -S . -B build
# For humans in VS Code devcontainer
cd tests/unit && cmake -S . -B build
```

**What this does:**
- Generates build system files (Makefiles or Ninja files)
- Discovers all test subdirectories
- Configures googletest
- Sets up WAMR runtime library

**Options:**
- `-DFULL_TEST=OFF` (default): Build all tests except `llm-enhanced-test` (faster)
- `-DFULL_TEST=ON`: Include `llm-enhanced-test` (requires submodule initialization)
- `-DCOLLECT_CODE_COVERAGE=1`: Enable code coverage instrumentation

**Example with options:**
```bash
cd tests/unit && cmake -S . -B build -DFULL_TEST=ON -DCOLLECT_CODE_COVERAGE=1```

#### Step 2: Build Tests

Compile all test executables:

```bash
# For AI agents
cd tests/unit && cmake --build build -j\$(nproc)
# For humans in VS Code devcontainer
cd tests/unit && cmake --build build -j$(nproc)
```

**What this does:**
- Compiles all test source files
- Builds WASM test fixtures (if configured)
- Links against googletest and WAMR runtime
- Creates test executables in `build/<test-name>/`

**Options:**
- `-j$(nproc)`: Use all CPU cores for parallel compilation (faster)
- `-j8`: Use specific number of cores
- `--target <test_name>`: Build only specific test
- `--verbose`: Show compiler commands

**Build time expectations:**
- First build: 2-5 minutes (compiles WAMR runtime)
- Incremental builds: 5-30 seconds (only changed files)
- Full rebuild: 1-3 minutes

#### Step 3: Run All Tests

Execute all tests with CTest:

```bash
# For AI agents
cd tests/unit && ctest --test-dir build --output-on-failure
# For humans in VS Code devcontainer
cd tests/unit && ctest --test-dir build --output-on-failure
```

**What this does:**
- Runs all registered test executables
- Shows progress: `Test #1: interpreter_test ........ Passed`
- Displays detailed output only for failed tests
- Returns non-zero exit code if any test fails

**Expected output:**
```
Test project /workspaces/wamr/tests/unit/build
      Start  1: interpreter_test
 1/15 Test  #1: interpreter_test .................   Passed    0.03 sec
      Start  2: aot_test
 2/15 Test  #2: aot_test .........................   Passed    0.12 sec
      ...
100% tests passed, 0 tests failed out of 15

Total Test time (real) =   2.45 sec
```

### Running Specific Tests

#### List Available Tests

See all test names:

```bash
cd tests/unit && ctest --test-dir build -N```

**Example output:**
```
Test project /workspaces/wamr/tests/unit/build
  Test  #1: interpreter_test
  Test  #2: aot_test
  Test  #3: linear_memory_test
  Test  #4: gc_test
  Test  #5: exception_handling_test
  ...

Total Tests: 15
```

#### Run by Test Name

Use `-R` (regex pattern) to filter tests:

```bash
# Run single test by exact name
cd tests/unit && ctest --test-dir build -R interpreter_test --output-on-failure
# Run all tests containing "memorycd tests/unit && ctest --test-dir build -R memory --output-on-failure
# Run AOT-related tests
cd tests/unit && ctest --test-dir build -R 'aot|compilation' --output-on-failure```

**Pattern matching examples:**
- `interpreter` → matches `interpreter_test`
- `memory` → matches `linear_memory_test`, `memory64_test`, `shared_heap_test`
- `aot` → matches `aot_test`, `aot_stack_frame_test`, `linear_memory_aot_test`

#### Run with Verbose Output

Show all test output, even for passing tests:

```bash
cd tests/unit && ctest --test-dir build -R my_test --verbose```

**When to use verbose mode:**
- Debugging test failures
- Verifying test behavior
- Checking performance characteristics

### Parallel Test Execution

Speed up test runs by running multiple tests simultaneously:

```bash
# Use all CPU cores
cd tests/unit && ctest --test-dir build --output-on-failure -j\$(nproc)
# Use specific number of parallel jobs
cd tests/unit && ctest --test-dir build --output-on-failure -j8```

**Benefits:**
- Significantly faster on multi-core systems
- Typical speedup: 3-8x on 8-core system

**Caveats:**
- Tests must be independent (no shared state)
- Some tests may have race conditions (rare)
- If you see flaky failures, try running serially

### Advanced Test Filtering

#### Exclude Tests

Run all tests EXCEPT specific ones:

```bash
# Exclude slow tests
cd tests/unit && ctest --test-dir build -E 'llm-enhanced' --output-on-failure
# Exclude multiple patterns
cd tests/unit && ctest --test-dir build -E 'gc|memory64' --output-on-failure```

#### Run Specific Test Range

Run tests by number:

```bash
# Run tests 1-10
cd tests/unit && ctest --test-dir build -I 1,10 --output-on-failure
# Run test #5 only
cd tests/unit && ctest --test-dir build -I 5,5 --output-on-failure```

#### Repeat Tests

Test for flakiness by repeating tests:

```bash
# Repeat test 10 times, stop on first failure
cd tests/unit && ctest --test-dir build -R my_test --repeat until-fail:10
# Repeat test 10 times, run all iterations
cd tests/unit && ctest --test-dir build -R my_test --repeat after-timeout:10```

**Use cases:**
- Detecting race conditions
- Verifying fixes for flaky tests
- Stress testing

### Rerun Failed Tests Only

After a test run with failures, quickly rerun only the failed tests:

```bash
# First run (some tests fail)
cd tests/unit && ctest --test-dir build --output-on-failure
# Rerun only failed tests
cd tests/unit && ctest --test-dir build --rerun-failed --output-on-failure```

### Clean and Rebuild

If tests behave strangely, try a clean rebuild:

```bash
# Remove build directory completely
cd tests/unit && rm -rf build && cmake -S . -B build && cmake --build build -j\$(nproc)
# Or clean within build directory
cd tests/unit/build && cmake --build . --target clean && cmake --build . -j\$(nproc)```

**When to clean rebuild:**
- After modifying CMakeLists.txt
- After changing build configuration options
- After pulling major changes from upstream
- When seeing unexplained link errors

---

## Collecting Code Coverage Data

Code coverage shows which lines of WAMR source code are executed by your tests. This helps identify untested code paths.

### Prerequisites

The devcontainer has all necessary tools pre-installed:
- `lcov` - Coverage data collection and report generation
- `genhtml` - HTML report generator
- `gcc`/`clang` with coverage support

### Step-by-Step Coverage Workflow

#### Step 1: Build with Coverage Instrumentation

Enable coverage flags during CMake configuration:

```bash
# For AI agents
cd tests/unit && cmake -S . -B build -DCOLLECT_CODE_COVERAGE=1
# For humans in VS Code devcontainer
cd tests/unit && cmake -S . -B build -DCOLLECT_CODE_COVERAGE=1
```

**What this does:**
- Adds `-fprofile-arcs -ftest-coverage` compiler flags
- Links against `gcov` library
- Instruments code to record execution counts

**Note:** Coverage builds are slower and produce larger binaries. Use only for coverage analysis, not regular development.

#### Step 2: Build Tests

Compile with coverage instrumentation:

```bash
cd tests/unit && cmake --build build -j\$(nproc)```

**Build artifacts:**
- `.gcno` files: Coverage notes (created at compile time)
- `.gcda` files: Coverage data (created at runtime, next step)

#### Step 3: Run Tests

Execute tests to generate coverage data:

```bash
cd tests/unit && ctest --test-dir build --output-on-failure```

**What this creates:**
- `.gcda` files in build directories (one per source file)
- These files contain execution counts for each line

**Important:** Only run the tests you want coverage for. If you want coverage for specific features, run only those tests:

```bash
# Coverage for interpreter only
cd tests/unit && ctest --test-dir build -R interpreter```

#### Step 4: Collect Coverage Data

Use `lcov` to gather coverage information from `.gcda` files:

```bash
cd tests/unit && lcov --capture --directory build --output-file coverage.all.info```

**What this does:**
- Scans build directory for `.gcda` files
- Aggregates execution counts
- Produces `coverage.all.info` with raw coverage data

**Expected output:**
```
Capturing coverage data from build
Found 450 entries
Writing data to coverage.all.info
Summary coverage rate:
  lines......: 67.3% (15234 of 22650 lines)
  functions..: 72.1% (1234 of 1712 functions)
```

#### Step 5: Filter Coverage Data

Extract only WAMR source code coverage (exclude test code and system headers):

```bash
cd tests/unit && lcov --extract coverage.all.info '*/core/iwasm/*' '*/core/shared/*' --output-file coverage.info```

**What this does:**
- Filters out test code from coverage report
- Filters out system headers (glibc, etc.)
- Keeps only WAMR runtime code
- Produces filtered `coverage.info`

**Why filter?**
- Focus on code you actually wrote
- Remove noise from report
- More accurate coverage percentage

**Expected output:**
```
Reading tracefile coverage.all.info
Extracting coverage data
Writing data to coverage.info
Summary coverage rate:
  lines......: 72.8% (12456 of 17100 lines)
  functions..: 75.3% (987 of 1310 functions)
```

#### Step 6: Generate HTML Report

Create browsable HTML coverage report:

```bash
cd tests/unit && genhtml coverage.info --output-directory coverage-report```

**What this creates:**
- `coverage-report/` directory with HTML files
- `index.html` - Main coverage dashboard
- Per-file coverage reports with line-by-line highlighting

**Expected output:**
```
Reading data file coverage.info
Found 342 entries
Writing directory view page
Overall coverage rate:
  lines......: 72.8% (12456 of 17100 lines)
  functions..: 75.3% (987 of 1310 functions)
```

#### Step 7: View Coverage Report

Open the report in your browser:

```bash
# On host machine (outside container)
open tests/unit/coverage-report/index.html

# Or start a simple HTTP server
cd tests/unit/coverage-report && python3 -m http.server 8000# Then visit http://localhost:8000 in your browser
```

**Report features:**
- **Green lines**: Executed by tests (covered)
- **Red lines**: Not executed (uncovered)
- **Orange lines**: Partially covered (conditional branches)
- Click on files to see line-by-line coverage
- Sort by coverage percentage to find gaps

### Quick Summary of Coverage

Get a text summary without generating HTML:

```bash
cd tests/unit && lcov --summary coverage.info```

**Example output:**
```
Summary coverage rate:
  lines......: 72.8% (12456 of 17100 lines)
  functions..: 75.3% (987 of 1310 functions)
  branches...: 65.2% (8932 of 13691 branches)
```

### Coverage for Specific Components

Generate coverage for a specific test:

```bash
# Build with coverage
cd tests/unit && cmake -S . -B build -DCOLLECT_CODE_COVERAGE=1 && cmake --build build
# Run specific test
cd tests/unit && ctest --test-dir build -R interpreter_test
# Generate coverage for that test only
cd tests/unit && lcov --capture --directory build/interpreter --output-file interpreter_coverage.info
# View summary
cd tests/unit && lcov --summary interpreter_coverage.info```

### Resetting Coverage Data

Clear coverage data between test runs:

```bash
# Remove all .gcda files
cd tests/unit && find build -name '*.gcda' -delete
# Or use lcov
cd tests/unit && lcov --zerocounters --directory build```

**When to reset:**
- Before running a different set of tests
- After modifying source code
- When generating coverage for CI

### Coverage Goals

**WAMR coverage targets:**
- Core runtime (`core/iwasm/interpreter`): Aim for 80%+ line coverage
- AOT compiler (`core/iwasm/aot`): Aim for 70%+ (complex codegen)
- Platform code (`core/shared/platform`): Aim for 60%+ (many platform-specific branches)

**What coverage doesn't tell you:**
- Whether tests are meaningful (can have 100% coverage with bad tests)
- Whether error paths are properly tested
- Whether tests check correct behavior (only that code ran)

**Use coverage as a guide, not a goal.** Focus on testing behavior, not hitting coverage numbers.

### Troubleshooting Coverage

**Problem:** No `.gcda` files generated

**Solution:** Verify coverage flags are enabled:
```bash
cd tests/unit/build && cmake -LA | grep COVERAGE```
Should show: `COLLECT_CODE_COVERAGE:BOOL=ON`

**Problem:** `lcov` reports 0% coverage

**Solution:** Tests may not have run. Check test output:
```bash
cd tests/unit && ctest --test-dir build --verbose```

**Problem:** Coverage report shows system headers

**Solution:** Check your filter pattern in step 5:
```bash
# Be more specific
cd tests/unit && lcov --extract coverage.all.info '*/wasm-micro-runtime/core/*' --output-file coverage.info```

**Problem:** Coverage percentage seems too low

**Solution:** 
- Check that you're filtering correctly (excluding test code)
- Some code may be conditionally compiled (feature flags)
- Platform-specific code may not run on your test platform

---

## Example Directory Structure

Here’s an example of how your test suite directory might look:

```
new-feature/
├── CMakeLists.txt
├── new_feature_test.cc
├── wasm-apps/
|   ├── CMakeLists.txt
│   ├── example.c
│   └── example.wat
```

---

## Best Practices

Follow these practices to write effective, maintainable unit tests.

### Test Structure and Organization

**Use the Arrange-Act-Assert pattern:**

```cpp
TEST_F(MyFeatureTest, DescriptiveTestName) {
    // Arrange: Set up test conditions
    int input = 42;
    int expected = 84;
    
    // Act: Execute the code being tested
    int result = my_function(input);
    
    // Assert: Verify the result
    EXPECT_EQ(expected, result);
}
```

**One logical assertion per test:**
```cpp
// BAD: Testing multiple unrelated things
TEST_F(InterpreterTest, Everything) {
    EXPECT_TRUE(load_module());      // Module loading
    EXPECT_EQ(42, call_function());  // Function execution
    EXPECT_TRUE(validate_memory());  // Memory validation
}

// GOOD: Separate tests for separate concerns
TEST_F(InterpreterTest, LoadsValidModule) {
    EXPECT_TRUE(load_module());
}

TEST_F(InterpreterTest, CallsFunctionWithCorrectResult) {
    EXPECT_EQ(42, call_function());
}

TEST_F(InterpreterTest, ValidatesMemoryBounds) {
    EXPECT_TRUE(validate_memory());
}
```

**Use descriptive test names:**
```cpp
// BAD: Unclear what's being tested
TEST(InterpreterTest, Test1) { ... }
TEST(InterpreterTest, Test2) { ... }

// GOOD: Name describes behavior
TEST(InterpreterTest, LoadsModuleSuccessfully) { ... }
TEST(InterpreterTest, RejectsModuleWithInvalidMagic) { ... }
TEST(InterpreterTest, ExecutesI32AddInstructionCorrectly) { ... }
```

**Test naming convention:**
- `<Component>Test` for test suite name
- `<Action><Condition><ExpectedResult>` for test name
- Examples: `LoadsValidModule`, `RejectsInvalidInput`, `CalculatesCorrectResult`

### Test Independence

**Tests must not depend on execution order:**

```cpp
// BAD: Test depends on previous test
static int global_state = 0;

TEST(BadTest, First) {
    global_state = 42;
}

TEST(BadTest, Second) {
    EXPECT_EQ(42, global_state);  // Fails if run alone!
}

// GOOD: Each test is self-contained
TEST_F(GoodTest, First) {
    int local_state = 42;
    EXPECT_EQ(42, local_state);
}

TEST_F(GoodTest, Second) {
    int local_state = 42;
    EXPECT_EQ(42, local_state);
}
```

**Clean up resources in TearDown:**
```cpp
class ResourceTest : public ::testing::Test {
protected:
    wasm_module_t module = nullptr;
    
    void SetUp() override {
        wasm_runtime_init();
        module = load_test_module();
    }
    
    void TearDown() override {
        if (module) {
            wasm_runtime_unload(module);
        }
        wasm_runtime_destroy();
    }
};
```

### Test Coverage Strategy

**Test the happy path (normal operation):**
```cpp
TEST_F(MemoryTest, AllocatesRequestedSize) {
    void *ptr = wasm_runtime_malloc(1024);
    EXPECT_NE(nullptr, ptr);
    wasm_runtime_free(ptr);
}
```

**Test error conditions:**
```cpp
TEST_F(MemoryTest, RejectsZeroSizeAllocation) {
    void *ptr = wasm_runtime_malloc(0);
    EXPECT_EQ(nullptr, ptr);
}

TEST_F(MemoryTest, RejectsExcessiveSizeAllocation) {
    void *ptr = wasm_runtime_malloc(SIZE_MAX);
    EXPECT_EQ(nullptr, ptr);
}
```

**Test boundary conditions:**
```cpp
TEST_F(MemoryTest, AllocatesMinimumSize) {
    void *ptr = wasm_runtime_malloc(1);
    EXPECT_NE(nullptr, ptr);
    wasm_runtime_free(ptr);
}

TEST_F(MemoryTest, AllocatesMaximumAllowedSize) {
    void *ptr = wasm_runtime_malloc(WASM_MAX_ALLOCATION);
    EXPECT_NE(nullptr, ptr);
    wasm_runtime_free(ptr);
}
```

**Test feature interactions:**
```cpp
TEST_F(RuntimeTest, MultipleModulesShareMemory) {
    wasm_module_t module1 = load_module("module1.wasm");
    wasm_module_t module2 = load_module("module2.wasm");
    
    // Test that modules can interact correctly
    EXPECT_TRUE(call_function(module1, "write_shared"));
    EXPECT_EQ(42, call_function(module2, "read_shared"));
}
```

### Documentation and Comments

**Add comments for complex setup:**
```cpp
TEST_F(AOTTest, CompilesWithOptimizations) {
    // Configure AOT compiler with O3 optimizations
    // This requires specific LLVM flags and target triple
    aot_comp_data_t comp_data = create_comp_data();
    comp_data->opt_level = 3;
    comp_data->size_level = 0;
    comp_data->target_triple = "x86_64-unknown-linux-gnu";
    
    // Compile module with these settings
    EXPECT_TRUE(compile_module(comp_data));
}
```

**Document why behavior is expected:**
```cpp
TEST_F(ValidationTest, RejectsInvalidTypeSection) {
    uint8_t invalid_module[] = { 
        0x00, 0x61, 0x73, 0x6d,  // WASM magic
        0x01, 0x00, 0x00, 0x00,  // Version 1
        0x01, 0x05,              // Type section, length 5
        0x01,                    // 1 type
        0x60, 0xFF              // Function type with invalid form
    };
    
    // Validation must reject: 0xFF is not a valid type form
    // Valid forms are: 0x60 (function), 0x5F (cont), 0x5E (rec)
    EXPECT_FALSE(validate_module(invalid_module, sizeof(invalid_module)));
}
```

**Reference specs or issues:**
```cpp
TEST_F(ExceptionTest, CatchesThrowRef) {
    // Tests exception handling as specified in:
    // https://github.com/WebAssembly/exception-handling/blob/main/proposals/exception-handling/Exceptions.md
    
    // Issue #1234: Previously failed to catch exception from nested call
    EXPECT_TRUE(test_nested_throw_catch());
}
```

### Reuse Test Utilities

**Leverage common utilities:**
```cpp
#include "test_helper.h"      // Common test helpers
#include "mock_allocator.h"   // Memory allocation mocking
#include "test_fixture.h"     // Shared test fixtures

TEST_F(CommonRuntimeTest, UsesSharedFixture) {
    // CommonRuntimeTest provides initialized WAMR runtime
    // from test_fixture.h
    EXPECT_TRUE(wasm_runtime_is_initialized());
}
```

**Create helper functions for repeated setup:**
```cpp
class MyFeatureTest : public ::testing::Test {
protected:
    // Helper to load test module with common configuration
    wasm_module_t LoadTestModule(const char *name) {
        char path[256];
        snprintf(path, sizeof(path), WASM_APPS_DIR "/%s.wasm", name);
        return load_module_from_file(path);
    }
    
    // Helper to call exported function and get result
    int32_t CallExportedFunction(const char *name, int32_t arg) {
        wasm_function_inst_t func = lookup_function(inst, name);
        uint32_t argv[1] = { (uint32_t)arg };
        wasm_runtime_call_wasm(inst, exec_env, func, 1, argv);
        return (int32_t)argv[0];
    }
};
```

### Performance Considerations

**Keep unit tests fast:**
```cpp
// BAD: Slow test that blocks development
TEST(SlowTest, ProcessesLargeDataset) {
    for (int i = 0; i < 1000000; i++) {
        process(create_large_object());  // Takes 10 seconds
    }
}

// GOOD: Fast test with representative data
TEST(FastTest, ProcessesTypicalDataset) {
    for (int i = 0; i < 100; i++) {
        process(create_small_object());  // Takes 10 milliseconds
    }
}
```

**Use mocks for expensive operations:**
```cpp
// Instead of actually compiling WASM every test:
TEST_F(AOTTest, UsesCache) {
    MockCompiler compiler;
    EXPECT_CALL(compiler, compile(_))
        .WillOnce(Return(cached_binary));
    
    EXPECT_TRUE(test_compilation_cache(&compiler));
}
```

**Benchmark tests go in `tests/benchmarks/`:**
Unit tests are for correctness, not performance measurement.

### Integration with CI

**Tests must be deterministic:**
- No random values (or use fixed seeds)
- No timing dependencies
- No network calls
- No dependency on execution order

**Tests must be portable:**
- Work on Linux, macOS, Windows
- Work on different architectures (x86_64, ARM)
- Don't assume specific file paths (use CMake variables)

**Tests must be maintainable:**
- Clear failure messages
- Easy to understand what's being tested
- Self-contained (don't require external setup)

---

## Troubleshooting

Common issues when working with unit tests and their solutions.

### Build Issues

**Problem:** CMake can't find googletest

**Solution:** Ensure you're at the correct directory level:
```bash
# WRONG: In a subdirectory
cd tests/unit/my-feature && cmake -S . -B build
# RIGHT: At tests/unit level
cd tests/unit && cmake -S . -B build```

**Problem:** Undefined references to WAMR functions

**Solution:** Make sure you're using `WAMR_RUNTIME_LIB_SOURCE`:
```cmake
# Add to your CMakeLists.txt
target_sources(my_test PRIVATE ${WAMR_RUNTIME_LIB_SOURCE})
```

**Problem:** WASI SDK not found

**Solution:** The devcontainer has WASI SDK pre-installed. Verify:
```bash
echo \$WASI_SDK_DIR"  # Should print path
```

### Test Execution Issues

**Problem:** Test can't find WASM files

**Solution:** Check the file path and ensure WASM apps were built:
```bash
find tests/unit/build -name '*.wasm'```

Update test to use correct path:
```cpp
// Use CMake-generated path
const char *wasm_path = WASM_APPS_DIR "/simple.wasm";
```

**Problem:** Test passes locally but fails in CI

**Solution:** Common causes:
- Test depends on execution order (run with `--repeat until-fail:10`)
- Test has race condition (run with ThreadSanitizer)
- Test assumes specific paths (use CMake variables)
- Test has memory leak (run with Valgrind)

**Problem:** Test times out

**Solution:** 
- Check for infinite loops in test code
- Use gdb to debug where it's stuck:
```bash
cd tests/unit/build && timeout 10 gdb --batch -ex 'run' -ex 'bt' ./my_test```

### Coverage Issues

**Problem:** Coverage report shows 0%

**Solution:** Verify coverage was enabled and tests ran:
```bash
# Check coverage flags
cd tests/unit/build && cmake -LA | grep COVERAGE
# Check .gcda files were created
find tests/unit/build -name '*.gcda' | wc -l```

**Problem:** Coverage excludes my test

**Solution:** Your test code should be excluded. Coverage tracks WAMR source, not tests:
```bash
# Filter should target core/, not tests/
lcov --extract coverage.all.info "*/core/iwasm/*" "*/core/shared/*" --output-file coverage.info
```

### Container Issues

**Problem:** Commands fail with container-related errors

**Solution:** See [AGENTS.md](../../AGENTS.md) for platform-specific execution requirements and [doc/dev-in-container.md](../../doc/dev-in-container.md#troubleshooting) for detailed container troubleshooting.

**Problem:** Permission denied errors

**Solution:** File ownership mismatch between host and container:
```bash
# On host
sudo chown -R $USER:$USER tests/unit/build/
```

---

## Related Documentation

### WAMR Development Guides

- **[doc/dev-in-container.md](../../doc/dev-in-container.md)** - Devcontainer setup (read this first)
- **[doc/testing.md](../../doc/testing.md)** - Comprehensive testing guide (unit, spec, integration)
- **[doc/architecture-overview.md](../../doc/architecture-overview.md)** - Understanding WAMR structure
- **[doc/building.md](../../doc/building.md)** - Building iwasm and wamrc
- **[AGENTS.md](../../AGENTS.md)** - AI agent development workflows

### Test-Related Resources

- **[tests/wamr-test-suites/](../wamr-test-suites/)** - WebAssembly spec tests
- **[samples/](../../samples/)** - Integration test examples
- **[tests/benchmarks/](../benchmarks/)** - Performance benchmarks

### External Resources

- **[Google Test Documentation](https://google.github.io/googletest/)** - Test framework reference
- **[CMake CTest Documentation](https://cmake.org/cmake/help/latest/manual/ctest.1.html)** - Test runner reference
- **[lcov Documentation](http://ltp.sourceforge.net/coverage/lcov.php)** - Coverage tool reference
- **[WebAssembly Specification](https://webassembly.github.io/spec/)** - Official WASM spec

---

**Documentation Version**: 2.0.0  
**Last Updated**: 2026-04-03  
**Maintained By**: WAMR Development Team

This guide is designed for clarity and completeness. If you find ambiguities or have suggestions for improvement, please open an issue or submit a pull request.
