# Testing WAMR

This guide covers all testing approaches for WAMR, including unit tests, WebAssembly spec tests, integration tests, benchmarks, and memory testing. All test commands run in the devcontainer environment for consistency and reproducibility.

---

## Prerequisites

Before running tests, ensure your environment is set up:

1. **Read [dev-in-container.md](dev-in-container.md)** for devcontainer setup
2. **Read [building.md](building.md)** for building iwasm and wamrc
3. **Verify container is available**: Run `scripts/in-container.sh --status`
4. **Build iwasm**: Tests require a working iwasm binary

Quick build command:
```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. && cmake --build . -j\$(nproc)"
```

---

## For AI Agents

**CRITICAL**: ALL test commands MUST run inside the devcontainer using the `scripts/in-container.sh` wrapper.

**Pattern for all test commands:**
```bash
scripts/in-container.sh "<command>"
```

The script automatically detects or starts the devcontainer and executes commands in the correct environment. Never run test commands directly on the host system.

**Error handling**: The wrapper properly propagates exit codes, enabling reliable test automation:
```bash
if scripts/in-container.sh "cd build && ctest"; then
    echo "Tests passed"
else
    echo "Tests failed"
    exit 1
fi
```

---

## Quick Start

Get running tests in under 60 seconds:

```bash
# Build iwasm with test support
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. -DWAMR_BUILD_INTERP=1 -DWAMR_BUILD_AOT=1 && cmake --build . -j\$(nproc)"

# Run unit tests
scripts/in-container.sh "cd tests/unit && mkdir -p build && cd build && cmake .. && cmake --build . && ctest --output-on-failure"

# Run a simple WASM spec test
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -b"

# Run regression tests (verify bug fixes)
scripts/in-container.sh "cd tests/regression/ba-issues && ./build_wamr.sh && ./run.py"
```

---

## Unit Tests

Unit tests verify WAMR's core components using Google Test framework (C++) and custom test harnesses (C). Tests are located in `tests/unit/`.

### Running All Unit Tests

Build and run all unit tests with CTest:

```bash
# Configure unit tests
scripts/in-container.sh "cd tests/unit && mkdir -p build && cd build && cmake .."

# Build all test executables
scripts/in-container.sh "cd tests/unit/build && cmake --build . -j\$(nproc)"

# Run all tests
scripts/in-container.sh "cd tests/unit/build && ctest --output-on-failure"
```

**Output format**: CTest shows which tests pass/fail with detailed output on failures.

### Running Specific Unit Tests

Run a single test by name:

```bash
# List available tests
scripts/in-container.sh "cd tests/unit/build && ctest -N"

# Run specific test by name
scripts/in-container.sh "cd tests/unit/build && ctest -R interpreter --verbose"

# Run all AOT tests
scripts/in-container.sh "cd tests/unit/build && ctest -R aot --verbose"

# Run all memory tests
scripts/in-container.sh "cd tests/unit/build && ctest -R 'linear-memory|memory64' --verbose"
```

**Test name patterns**: Use regex patterns with `-R` to match multiple tests.

### Test Categories

Available unit test suites:

| Test Suite | Description | Location |
|-----------|-------------|----------|
| `aot` | AOT compiler tests | `tests/unit/aot/` |
| `aot-stack-frame` | AOT stack frame validation | `tests/unit/aot-stack-frame/` |
| `compilation` | Compilation pipeline tests | `tests/unit/compilation/` |
| `custom-section` | Custom section handling | `tests/unit/custom-section/` |
| `exception-handling` | Exception handling tests | `tests/unit/exception-handling/` |
| `gc` | Garbage collection tests | `tests/unit/gc/` |
| `interpreter` | Interpreter tests | `tests/unit/interpreter/` |
| `libc-builtin` | Built-in libc tests | `tests/unit/libc-builtin/` |
| `linear-memory-aot` | AOT linear memory tests | `tests/unit/linear-memory-aot/` |
| `linear-memory-wasm` | WASM linear memory tests | `tests/unit/linear-memory-wasm/` |
| `linux-perf` | Linux perf integration | `tests/unit/linux-perf/` |
| `memory64` | 64-bit memory tests | `tests/unit/memory64/` |
| `running-modes` | Execution mode tests | `tests/unit/running-modes/` |
| `shared-heap` | Shared heap tests | `tests/unit/shared-heap/` |
| `shared-utils` | Utility function tests | `tests/unit/shared-utils/` |

### Parallel Test Execution

Speed up test runs by executing tests in parallel:

```bash
# Run tests using all CPU cores
scripts/in-container.sh "cd tests/unit/build && ctest --output-on-failure -j\$(nproc)"

# Run with specific core count
scripts/in-container.sh "cd tests/unit/build && ctest --output-on-failure -j8"
```

**Warning**: Some tests may conflict when run in parallel. If you see flaky failures, try running serially.

### Test Filtering

Advanced CTest filtering options:

```bash
# Exclude specific tests
scripts/in-container.sh "cd tests/unit/build && ctest -E 'gc|memory64' --output-on-failure"

# Run only tests matching label
scripts/in-container.sh "cd tests/unit/build && ctest -L fast --output-on-failure"

# Run tests 1-10
scripts/in-container.sh "cd tests/unit/build && ctest -I 1,10 --output-on-failure"

# Repeat tests to check for flakiness
scripts/in-container.sh "cd tests/unit/build && ctest -R interpreter --repeat until-fail:10"
```

### Verbose Test Output

Get detailed output for debugging test failures:

```bash
# Show all test output
scripts/in-container.sh "cd tests/unit/build && ctest --verbose"

# Show only failed test output
scripts/in-container.sh "cd tests/unit/build && ctest --output-on-failure"

# Show test command lines being executed
scripts/in-container.sh "cd tests/unit/build && ctest --verbose --output-on-failure"
```

### Clean and Rebuild Tests

If tests behave strangely, try a clean rebuild:

```bash
# Remove build directory and rebuild
scripts/in-container.sh "cd tests/unit && rm -rf build && mkdir build && cd build && cmake .. && cmake --build . -j\$(nproc)"

# Or clean within build directory
scripts/in-container.sh "cd tests/unit/build && cmake --build . --target clean && cmake --build . -j\$(nproc)"
```

---

## WASM Spec Tests

WebAssembly spec tests verify conformance to the official WebAssembly specification. WAMR uses the official test suite from the WebAssembly spec repository.

### Running Spec Tests

The `test_wamr.sh` script automates spec test execution:

```bash
# Run spec tests in fast interpreter mode
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp"

# Run spec tests in AOT mode
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot"

# Run spec tests in classic interpreter mode
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t classic-interp"

# Run spec tests in JIT mode
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t jit"
```

**Note**: First run downloads the spec test suite and builds necessary tools. This can take several minutes. Subsequent runs are much faster.

### Using Binary Release of WABT

Speed up test setup by using pre-built WABT binaries instead of compiling from source:

```bash
# Use binary release (-b flag)
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -b"
```

**Recommended for most use cases.** Saves 5-10 minutes on first run.

### Testing All Execution Modes

Test spec conformance across all execution modes:

```bash
# Test all modes (interpreter, fast-interp, AOT, JIT)
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec"
```

**Warning**: This takes significant time (30+ minutes) as it tests 4 different execution modes.

### Spec Tests with Features Enabled

Test with optional WebAssembly features:

```bash
# Test with SIMD enabled
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -S"

# Test with pthread support
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -p"

# Test with multi-threading (wasi-threads)
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -x"

# Test with garbage collection
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -G"
```

### Cross-Architecture Testing

Test on different target architectures:

```bash
# Test for x86_32 target
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -m x86_32"

# Test for x86_64 target (default)
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -m x86_64"
```

**Note**: x86_32 requires multilib support (pre-installed in devcontainer).

### Test Categories

Spec tests cover these WebAssembly features:

- Core WebAssembly instructions (arithmetic, memory, control flow)
- Memory operations (load, store, grow)
- Table operations
- Function calls and imports
- Validation rules
- SIMD instructions (when enabled)
- Threads and atomics (when enabled)
- Exception handling (when enabled)
- Reference types (when enabled)
- Bulk memory operations (when enabled)

### Spec Test Options

Common `test_wamr.sh` options:

| Option | Description | Example |
|--------|-------------|---------|
| `-s spec` | Run spec tests | Required |
| `-t MODE` | Execution mode: `fast-interp`, `classic-interp`, `aot`, `jit` | `-t aot` |
| `-b` | Use binary WABT release | `-b` |
| `-S` | Enable SIMD | `-S` |
| `-p` | Enable pthread | `-p` |
| `-x` | Enable wasi-threads | `-x` |
| `-G` | Enable garbage collection | `-G` |
| `-m ARCH` | Target architecture | `-m x86_32` |

### Understanding Test Results

Test output shows:

```
[PASS] test_name.wast
[FAIL] test_name.wast
  Expected: 42
  Got: 0
```

**Pass**: All assertions in the test passed.
**Fail**: One or more assertions failed. The output shows expected vs actual values.

---

## Regression Tests

Regression tests verify that previously fixed bugs do not reappear when code changes are made. These tests capture specific bugs that were found and fixed, ensuring the fixes continue to work correctly in future versions.

### What Are Regression Tests

**Purpose**: Prevent fixed bugs from returning when code changes are made. Each regression test represents a specific bug that was discovered, fixed, and now continuously validated.

**When to Add Regression Tests**: Create a regression test whenever you:
- Fix a bug reported in an issue tracker (GitHub, Bytecode Alliance)
- Discover and fix a security vulnerability
- Resolve a crash, hang, or incorrect behavior
- Fix a bug found by fuzzing or other automated testing

**Types of Regression Tests in WAMR**:
- **BA Issues** (`tests/regression/ba-issues/`) - Tests for bugs from Bytecode Alliance issue tracker
- **Future suites** - Security vulnerabilities (CVEs), performance regressions, spec compliance issues

**Best Practice**: Every bug fix should include a regression test to prevent the bug from returning unnoticed.

### Running Regression Tests

Run all regression tests to verify no fixed bugs have reappeared:

```bash
# Build all required iwasm variants and wamrc
scripts/in-container.sh "cd tests/regression/ba-issues && ./build_wamr.sh"

# Run all regression tests
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py"
```

**Expected output when all tests pass**:
```
==== Test results ====
   Total: 22
  Passed: 22
  Failed: 0
  Left issues in folder: no more
  Cases in JSON but not found in folder: no more
```

**Running specific issues**:

```bash
# Test a single issue
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py --issues 2833"

# Test multiple specific issues
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py -i 2833,2834,2835"
```

**Output explanation**:
- `Total`: Number of test configurations executed
- `Passed`: Tests where actual result matched expected result
- `Failed`: Tests where results didn't match
- `Left issues in folder`: Issue directories without JSON configuration
- `Cases in JSON but not found in folder`: JSON configs referencing missing directories

### Adding New Regression Tests

When you fix a bug, follow this workflow to add a regression test:

**Quick workflow**:

```bash
# 1. Create test case directory (example: issue #3022)
scripts/in-container.sh "cd tests/regression/ba-issues && ./helper.sh 3022"

# 2. Add test files
# Place your WASM file and any required files in issues/issue-3022/
# If you have a zip file: place it in the directory and run:
scripts/in-container.sh "cd tests/regression/ba-issues && ./helper.sh -x 3022"

# 3. Build iwasm variants (if not already built)
scripts/in-container.sh "cd tests/regression/ba-issues && ./build_wamr.sh"

# 4. Configure test execution
# Edit running_config.json to add test configuration:
# {
#     "deprecated": false,
#     "ids": [3022],
#     "runtime": "iwasm-default",
#     "file": "test.wasm",
#     "mode": "fast-interp",
#     "options": "--heap-size=0 -f main",
#     "argument": "",
#     "expected return": {
#         "ret code": 0,
#         "stdout content": "expected output",
#         "description": "Issue #3022: should not crash/should return correct value"
#     }
# }

# 5. Run test to verify
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py -i 3022"

# 6. Commit test with the fix
git add tests/regression/ba-issues/issues/issue-3022/
git add tests/regression/ba-issues/running_config.json
git commit -m "test: add regression test for issue #3022"
```

**Directory structure after adding test**:

```
tests/regression/ba-issues/
├── issues/
│   └── issue-3022/
│       ├── test.wasm        (reproduces the bug)
│       └── other-files...   (if needed)
└── running_config.json      (test configuration)
```

### Understanding Test Results

**All tests pass**:
```
==== Test results ====
   Total: 22
  Passed: 22
  Failed: 0
  Left issues in folder: no more
  Cases in JSON but not found in folder: no more
```

**Test failure**:
```
==== Test results ====
   Total: 22
  Passed: 21
  Failed: 1
```

Check the detailed failure log:
```bash
scripts/in-container.sh "cd tests/regression/ba-issues && cat issues_tests.log"
```

**Example failure log**:
```
=======================================================
Failing issue id: 2945.
run with command_lists: ['./build/build-iwasm-default/iwasm', '--heap-size=0', '-f', 'to_test', 'test.wasm']
exit code (actual, expected) : (1, 0)
stdout (actual, expected) : ('Exception: out of bounds', 'Success')
=======================================================
```

**Missing configuration**:
```
Left issues in folder: #3022
```

**Meaning**: You created `issues/issue-3022/` but forgot to add configuration to `running_config.json`.

**Fix**: Add JSON configuration entry for the test.

**Missing directory**:
```
Cases in JSON but not found in folder: #3022
```

**Meaning**: `running_config.json` references issue #3022 but the directory doesn't exist.

**Fix**: Create the directory with `./helper.sh 3022` or remove the JSON entry.

### Troubleshooting

**Build failures**:

```bash
# Ensure you're in the devcontainer
scripts/in-container.sh --status

# Clean and rebuild
scripts/in-container.sh "cd tests/regression/ba-issues && rm -rf build && ./build_wamr.sh"
```

**Test configuration errors**:

```bash
# Validate JSON syntax
scripts/in-container.sh "cd tests/regression/ba-issues && python3 -m json.tool running_config.json > /dev/null"

# Common issues:
# - Missing commas between array elements
# - Trailing comma after last element
# - Unescaped quotes in strings
```

**Wrong expected output**:

If a test fails because expected output has changed legitimately:
1. Verify the new behavior is correct
2. Update the `expected return` section in `running_config.json`
3. Document why the expected output changed in the description field

**Deprecated tests**:

Tests may become deprecated when:
- WebAssembly spec changes invalidate the test
- The WASM file is no longer valid according to current spec
- The issue was closed as "not a bug"

Process to deprecate:
1. Move test files: `mv issues/issue-47 issues-deprecated/`
2. Mark configuration: `"deprecated": true` in JSON
3. Update description to explain why deprecated

### Detailed Documentation

For comprehensive information about regression tests:

**See [tests/regression/README.md](../tests/regression/README.md)** for:
- Complete configuration reference
- All available runtime variants
- Advanced test scenarios (AOT compilation, multiple execution modes)
- Full troubleshooting guide

**See [tests/regression/ba-issues/README.md](../tests/regression/ba-issues/README.md)** for:
- Detailed JSON configuration syntax
- wamrc + iwasm test patterns
- Build configuration customization
- Helper script reference

---

## Integration Tests (Samples)

Sample applications serve as integration tests, demonstrating real-world WAMR usage patterns. Located in `samples/`.

### Running Sample Tests

Most samples can be tested by building and running:

```bash
# Test basic sample
scripts/in-container.sh "cd samples/basic && mkdir -p build && cd build && cmake .. && cmake --build . && ./basic"

# Test file I/O sample
scripts/in-container.sh "cd samples/file && mkdir -p build && cd build && cmake .. && cmake --build . && ./file-example"

# Test multi-threading sample
scripts/in-container.sh "cd samples/multi-thread && mkdir -p build && cd build && cmake .. && cmake --build . && ctest"

# Test WASM C API sample
scripts/in-container.sh "cd samples/wasm-c-api && mkdir -p build && cd build && cmake .. && cmake --build . && ctest"
```

### Available Sample Tests

Key integration test samples:

| Sample | Description | Location |
|--------|-------------|----------|
| `basic` | Basic iwasm usage | `samples/basic/` |
| `file` | File I/O with WASI | `samples/file/` |
| `multi-thread` | Multi-threading | `samples/multi-thread/` |
| `wasm-c-api` | WASM C API examples | `samples/wasm-c-api/` |
| `native-lib` | Native function calls | `samples/native-lib/` |
| `socket-api` | Socket operations | `samples/socket-api/` |
| `ref-types` | Reference types | `samples/ref-types/` |
| `spawn-thread` | Thread spawning | `samples/spawn-thread/` |

### Standalone Tests

Additional integration tests in `tests/standalone/`:

```bash
# Test module malloc behavior
scripts/in-container.sh "cd tests/standalone/test-module-malloc && mkdir -p build && cd build && cmake .. && make && ./test_module_malloc"

# Test native function invocation
scripts/in-container.sh "cd tests/standalone/test-invoke-native && mkdir -p build && cd build && cmake .. && make && ./test"

# Test running modes
scripts/in-container.sh "cd tests/standalone/test-running-modes/c-embed && mkdir -p build && cd build && cmake .. && make && ./test_running_modes"
```

---

## Benchmarks

Performance benchmarks measure WAMR's execution speed across different workloads and execution modes. Located in `tests/benchmarks/`.

### CoreMark Benchmark

CoreMark is an industry-standard CPU benchmark:

```bash
# Run CoreMark in interpreter mode
scripts/in-container.sh "cd tests/benchmarks/coremark && ./build.sh && ./run_interp.sh"

# Run CoreMark in AOT mode (faster)
scripts/in-container.sh "cd tests/benchmarks/coremark && ./build.sh && ./run_aot.sh"
```

**Output**: Shows CoreMark score and iterations per second. Higher is better.

### Sightglass Benchmark

Sightglass is a WebAssembly-focused benchmark suite:

```bash
# Run Sightglass in interpreter mode
scripts/in-container.sh "cd tests/benchmarks/sightglass && ./build.sh && ./run_interp.sh"

# Run Sightglass in AOT mode
scripts/in-container.sh "cd tests/benchmarks/sightglass && ./run_aot.sh"

# Test with PGO (Profile-Guided Optimization)
scripts/in-container.sh "cd tests/benchmarks/sightglass && ./test_pgo.sh"
```

**Output**: Shows execution time for various workloads (hash, random, etc.).

### PolyBench Benchmark

PolyBench contains scientific computing kernels:

```bash
# Run PolyBench in interpreter mode
scripts/in-container.sh "cd tests/benchmarks/polybench && ./build.sh && ./run_interp.sh"

# Run PolyBench in AOT mode
scripts/in-container.sh "cd tests/benchmarks/polybench && ./run_aot.sh"
```

**Output**: Shows execution time for linear algebra and image processing kernels.

### Other Benchmarks

Additional benchmarks available:

```bash
# Dhrystone benchmark
scripts/in-container.sh "cd tests/benchmarks/dhrystone && ./build.sh && ./run_aot.sh"

# JetStream benchmark
scripts/in-container.sh "cd tests/benchmarks/jetstream && ./build.sh && ./run_aot.sh"

# libsodium benchmark
scripts/in-container.sh "cd tests/benchmarks/libsodium && ./build.sh && ./run_aot.sh"
```

### Comparing Execution Modes

Benchmarks typically provide scripts to compare interpreter vs AOT vs JIT:

```bash
# Run all execution modes and compare
scripts/in-container.sh "cd tests/benchmarks/coremark && ./build.sh && ./run_interp.sh && ./run_aot.sh"
```

**Typical results**:
- Interpreter: Baseline (slowest)
- Fast JIT: 2-3x faster than interpreter
- AOT: 5-10x faster than interpreter
- LLVM JIT: Similar to AOT, slightly faster for some workloads

### Profile-Guided Optimization (PGO)

PGO uses runtime profiling data to optimize AOT compilation:

```bash
# Run PGO test (requires llvm-profdata)
scripts/in-container.sh "cd tests/benchmarks/sightglass && ./test_pgo.sh"
```

**Note**: PGO typically provides 10-20% additional performance improvement over standard AOT.

---

## Memory Testing with Valgrind

Valgrind detects memory leaks, use-after-free errors, buffer overflows, and other memory-related bugs. Pre-installed in the devcontainer.

### Basic Memory Testing

Run iwasm under Valgrind:

```bash
# Build debug version of iwasm
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build-debug && cd build-debug && cmake .. -DCMAKE_BUILD_TYPE=Debug && cmake --build . -j\$(nproc)"

# Run WASM file with memory leak detection
scripts/in-container.sh "valgrind --leak-check=full product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

**Output**: Valgrind reports any memory leaks, invalid memory access, or other issues.

### Full Memory Check

Enable all Valgrind checks for comprehensive testing:

```bash
# Full memory check with detailed output
scripts/in-container.sh "valgrind \
  --leak-check=full \
  --show-leak-kinds=all \
  --track-origins=yes \
  --verbose \
  --log-file=valgrind-report.txt \
  product-mini/platforms/linux/build-debug/iwasm test.wasm"

# View report
scripts/in-container.sh "cat valgrind-report.txt"
```

**Options explained**:
- `--leak-check=full`: Detailed leak information
- `--show-leak-kinds=all`: Show all types of leaks (definite, indirect, possible, reachable)
- `--track-origins=yes`: Track origin of uninitialized values
- `--verbose`: Detailed output
- `--log-file`: Save report to file

### Running Unit Tests Under Valgrind

Test WAMR's own code for memory issues:

```bash
# Run unit tests with Valgrind (slow but thorough)
scripts/in-container.sh "cd tests/unit/build && ctest -T memcheck"

# View memory check results
scripts/in-container.sh "cd tests/unit/build && cat Testing/Temporary/MemoryChecker.*.log"
```

**Warning**: Running all tests under Valgrind takes significant time (hours).

### Testing Specific Test Under Valgrind

Run a single test with memory checking:

```bash
# Run specific test under Valgrind
scripts/in-container.sh "cd tests/unit/build && valgrind --leak-check=full ./interpreter_test"
```

### Suppressing Known Issues

Use suppression files to ignore known false positives:

```bash
# Run with suppressions
scripts/in-container.sh "valgrind \
  --leak-check=full \
  --suppressions=tests/wamr-test-suites/tsan_suppressions.txt \
  product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

### Understanding Valgrind Output

Key sections in Valgrind output:

```
HEAP SUMMARY:
    in use at exit: 0 bytes in 0 blocks
  total heap usage: 100 allocs, 100 frees, 10,000 bytes allocated

LEAK SUMMARY:
    definitely lost: 0 bytes in 0 blocks
    indirectly lost: 0 bytes in 0 blocks
      possibly lost: 0 bytes in 0 blocks
    still reachable: 0 bytes in 0 blocks
         suppressed: 0 bytes in 0 blocks
```

**Clean run**: "0 bytes in 0 blocks" for all leak categories.
**Memory leak**: Non-zero values in "definitely lost" or "indirectly lost".
**False positive**: Non-zero values in "possibly lost" or "still reachable" (often safe).

---

## Writing Tests

Guidelines for adding new tests to WAMR.

### Test Location Conventions

Place tests in the appropriate directory:

| Test Type | Location | Build System |
|-----------|----------|--------------|
| Unit tests (C++) | `tests/unit/<component>/` | CMake + CTest |
| Spec tests | Managed by `test_wamr.sh` | Shell script |
| Integration tests | `samples/<feature>/` | CMake + CTest |
| Regression tests | `tests/regression/ba-issues/` | Python + JSON config |
| Benchmarks | `tests/benchmarks/<name>/` | Custom scripts |
| Standalone tests | `tests/standalone/<name>/` | CMake |

**Note**: When fixing a bug, always add a regression test. See the [Regression Tests](#regression-tests) section for details.

### Naming Conventions

Follow these naming patterns:

**Unit test files**:
- Test files: `<component>_test.cc` or `<component>_test.c`
- Test executables: `<component>_test`
- Example: `interpreter_test.cc` → `interpreter_test`

**Test functions**:
- Google Test: `TEST(TestSuiteName, TestName)`
- Example: `TEST(Interpreter, BasicExecution)`

**CTest test names**:
- Pattern: `test_<component>_<feature>`
- Example: `test_interpreter_memory_access`

### Example Unit Test (C++)

Create a new Google Test unit test:

```cpp
// tests/unit/my-feature/my_feature_test.cc
#include "gtest/gtest.h"
#include "wasm_runtime.h"

// Test fixture
class MyFeatureTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize WAMR
        wasm_runtime_init();
    }

    void TearDown() override {
        // Cleanup
        wasm_runtime_destroy();
    }
};

// Test case
TEST_F(MyFeatureTest, BasicFunctionality) {
    // Arrange
    int expected = 42;

    // Act
    int result = my_feature_function();

    // Assert
    EXPECT_EQ(expected, result);
}

TEST_F(MyFeatureTest, ErrorHandling) {
    // Test error conditions
    EXPECT_FALSE(my_feature_function_with_invalid_input());
}
```

### Example CMakeLists.txt for Unit Test

Add CMake configuration for your test:

```cmake
# tests/unit/my-feature/CMakeLists.txt
cmake_minimum_required(VERSION 3.14)

project(my_feature_test)

# Add WAMR include paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../core/iwasm/include)

# Add test executable
add_executable(my_feature_test my_feature_test.cc)

# Link against Google Test and WAMR
target_link_libraries(my_feature_test gtest gtest_main iwasm_static)

# Register with CTest
enable_testing()
add_test(NAME my_feature_test COMMAND my_feature_test)
```

### Example Integration Test

Create a sample that demonstrates feature usage:

```c
// samples/my-feature/test.c
#include <stdio.h>
#include "wasm_export.h"

int main(int argc, char *argv[])
{
    // Initialize WAMR
    wasm_runtime_init();

    // Load and instantiate module
    wasm_module_t module = load_wasm_module("test.wasm");
    if (!module) {
        printf("Failed to load module\n");
        return 1;
    }

    wasm_module_inst_t inst = wasm_runtime_instantiate(module, 8092, 8092, 
                                                        NULL, 0);
    if (!inst) {
        printf("Failed to instantiate module\n");
        return 1;
    }

    // Execute function
    wasm_function_inst_t func = wasm_runtime_lookup_function(inst, 
                                                              "test_function", 
                                                              NULL);
    if (!func) {
        printf("Failed to find function\n");
        return 1;
    }

    uint32_t argv_buf[1] = { 42 };
    if (!wasm_runtime_call_wasm(inst, NULL, func, 1, argv_buf)) {
        printf("Function call failed\n");
        return 1;
    }

    printf("Result: %d\n", argv_buf[0]);

    // Cleanup
    wasm_runtime_deinstantiate(inst);
    wasm_runtime_unload(module);
    wasm_runtime_destroy();

    return 0;
}
```

### Best Practices for Writing Tests

**Test Structure**:
- Use Arrange-Act-Assert pattern
- One assertion concept per test
- Clear test names that describe what is being tested

**Test Independence**:
- Tests should not depend on execution order
- Clean up resources in teardown
- Reset global state between tests

**Test Coverage**:
- Test happy paths (normal operation)
- Test error conditions (invalid input, out of memory, etc.)
- Test edge cases (boundary values, empty input, etc.)
- Test feature interactions (combinations of features)

**Performance**:
- Keep unit tests fast (milliseconds)
- Use mocks/stubs for expensive operations
- Save integration tests for full system behavior

**Documentation**:
- Add comments explaining complex test setup
- Document what behavior is being verified
- Include references to relevant specifications or issues

**Regression Testing**:
- When fixing a bug, add a regression test to prevent recurrence
- See [Regression Tests](#regression-tests) section for workflow
- Include the issue number in the test to track the bug fix

### Adding Tests to CI

Tests are automatically run by CI when:
- CMake test is registered with `add_test()`
- Test is in `tests/unit/` directory
- Test script is in `tests/wamr-test-suites/`

No additional CI configuration needed for most tests.

---

## Continuous Integration

WAMR uses GitHub Actions for continuous integration. Tests run automatically on every pull request and push to main branches.

### CI Test Matrix

The CI system tests multiple configurations:

**Platforms**:
- Linux (Ubuntu 20.04, 22.04)
- macOS (latest)
- Windows (latest)

**Execution Modes**:
- Classic interpreter
- Fast interpreter
- AOT
- LLVM JIT
- Fast JIT

**Architectures**:
- x86_64
- x86_32
- AArch64 (cross-compilation)
- ARM (cross-compilation)

**Features**:
- SIMD enabled/disabled
- Threads enabled/disabled
- Multi-module enabled/disabled
- Various WebAssembly proposals

### Running CI Tests Locally

Replicate CI behavior locally using the devcontainer:

```bash
# Run the same unit tests as CI
scripts/in-container.sh "cd tests/unit && mkdir -p build && cd build && cmake .. && cmake --build . -j\$(nproc) && ctest --output-on-failure"

# Run spec tests like CI (fast-interp mode)
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -b"

# Run spec tests like CI (AOT mode)
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -b"
```

### CI Workflow Files

GitHub Actions workflows are in `.github/workflows/`:

- `compilation_on_*.yml` - Build tests for various platforms
- `spec_test.yml` - WebAssembly spec test suite
- `unit_test.yml` - Unit test execution

**View CI results**: Check the Actions tab on GitHub after pushing changes.

### PR Testing Requirements

Pull requests must pass:
1. All unit tests
2. WebAssembly spec tests (interpreter and AOT)
3. Build tests for all platforms
4. Code formatting checks (clang-format)

**Test before submitting PR**: Run unit tests and spec tests locally to catch issues early.

---

## Troubleshooting

Common test issues and solutions.

### Tests Fail with "Cannot Find iwasm"

**Problem**: Tests can't locate the iwasm binary.

**Solution**: Build iwasm first:

```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. && cmake --build . -j\$(nproc)"
```

Or set the path explicitly:

```bash
scripts/in-container.sh "export IWASM_CMD=/workspaces/ai-thoughts/product-mini/platforms/linux/build/iwasm && cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp"
```

### Spec Tests Timeout

**Problem**: Spec tests take too long or hang.

**Solution**: Use binary WABT release instead of compiling:

```bash
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -b"
```

Or increase timeout (for slow systems):

```bash
scripts/in-container.sh "cd tests/wamr-test-suites && TIMEOUT=600 ./test_wamr.sh -s spec -t fast-interp"
```

### Unit Tests Crash

**Problem**: Unit tests segfault or crash.

**Solution**: Build in debug mode and run under GDB:

```bash
# Build debug version
scripts/in-container.sh "cd tests/unit && rm -rf build && mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug && cmake --build . -j\$(nproc)"

# Run failing test under GDB
scripts/in-container.sh "gdb --args tests/unit/build/interpreter_test"
```

Or run under Valgrind to detect memory issues:

```bash
scripts/in-container.sh "valgrind --leak-check=full tests/unit/build/interpreter_test"
```

### Valgrind Reports Leaks

**Problem**: Valgrind shows memory leaks.

**Solution**: Verify if leaks are in WAMR or test code:

```bash
# Run with full stack traces
scripts/in-container.sh "valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes product-mini/platforms/linux/build-debug/iwasm test.wasm 2>&1 | tee valgrind.log"

# Check if leaks are in your test WASM or in WAMR
scripts/in-container.sh "grep 'definitely lost' valgrind.log"
```

If leaks are in WAMR, file a bug report with the Valgrind output.

### Tests Fail Intermittently

**Problem**: Tests pass sometimes and fail other times (flaky tests).

**Solution**: Run tests multiple times to reproduce:

```bash
# Repeat test until it fails
scripts/in-container.sh "cd tests/unit/build && ctest -R flaky_test --repeat until-fail:100"
```

Check for:
- Race conditions (run with ThreadSanitizer)
- Uninitialized variables (run with Valgrind)
- System resource limits (increase memory/file descriptors)

### Build Fails with "Test Not Found"

**Problem**: CTest can't find test executables.

**Solution**: Ensure CMake configuration completed successfully:

```bash
# Reconfigure and rebuild
scripts/in-container.sh "cd tests/unit/build && cmake .. && cmake --build . -j\$(nproc)"

# Verify test executables exist
scripts/in-container.sh "cd tests/unit/build && find . -type f -executable -name '*test*'"
```

### Permission Errors During Tests

**Problem**: Tests fail with "Permission denied".

**Solution**: Check file ownership on host:

```bash
# View ownership
ls -la tests/unit/build/

# Fix ownership if needed (run on host)
sudo chown -R $USER:$USER tests/
```

Or clean build directory and rebuild:

```bash
scripts/in-container.sh "cd tests/unit && rm -rf build && mkdir build && cd build && cmake .. && cmake --build . -j\$(nproc)"
```

### Container Issues During Testing

**Problem**: `scripts/in-container.sh` fails or container not found.

**Solution**: See [dev-in-container.md Troubleshooting](dev-in-container.md#troubleshooting) for container-specific issues.

Quick fixes:

```bash
# Check container status
scripts/in-container.sh --status

# Force container detection
rm -f .devcontainer/.container-name
scripts/in-container.sh --verbose "pwd"

# Restart container if needed
docker restart $(docker ps -a | grep wamr | awk '{print $1}')
```

---

## Quick Reference

**Most Common Test Commands:**

```bash
# Build iwasm
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. && cmake --build . -j\$(nproc)"

# Run all unit tests
scripts/in-container.sh "cd tests/unit && mkdir -p build && cd build && cmake .. && cmake --build . -j\$(nproc) && ctest --output-on-failure"

# Run specific unit test
scripts/in-container.sh "cd tests/unit/build && ctest -R interpreter --verbose"

# Run spec tests (fast)
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -b"

# Run spec tests (AOT)
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -b"

# Run regression tests (verify bug fixes)
scripts/in-container.sh "cd tests/regression/ba-issues && ./build_wamr.sh && ./run.py"

# Run specific regression test
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py -i 2833"

# Run benchmark
scripts/in-container.sh "cd tests/benchmarks/coremark && ./build.sh && ./run_aot.sh"

# Memory check with Valgrind
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build-debug && cd build-debug && cmake .. -DCMAKE_BUILD_TYPE=Debug && cmake --build . -j\$(nproc) && valgrind --leak-check=full ./iwasm test.wasm"

# Check test container
scripts/in-container.sh --status
```

---

## Related Documentation

- **[building.md](building.md)** - Build instructions for iwasm and wamrc
- **[dev-in-container.md](dev-in-container.md)** - Devcontainer setup and usage
- **[debugging.md](debugging.md)** - Debugging WAMR with GDB and other tools
- **[code-quality.md](code-quality.md)** - Code formatting and quality standards
- **[tests/regression/README.md](../tests/regression/README.md)** - Regression testing overview and best practices
- **[tests/regression/ba-issues/README.md](../tests/regression/ba-issues/README.md)** - Detailed BA issue regression test guide

---

## External Resources

- [WebAssembly Specification](https://webassembly.github.io/spec/)
- [WebAssembly Test Suite](https://github.com/WebAssembly/testsuite)
- [Google Test Documentation](https://google.github.io/googletest/)
- [CMake CTest Documentation](https://cmake.org/cmake/help/latest/manual/ctest.1.html)
- [Valgrind User Manual](https://valgrind.org/docs/manual/manual.html)

---

**Documentation Version**: 1.0.0  
**Last Updated**: 2026-04-03  
**Maintained By**: WAMR Development Team
