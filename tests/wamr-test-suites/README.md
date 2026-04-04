# WAMR Test Suites

Comprehensive guide to WAMR's WebAssembly specification conformance testing.

## Table of Contents

- [Overview](#overview)
- [Using the Devcontainer](#-using-the-devcontainer)
- [Test Categories](#test-categories)
- [Quick Start](#quick-start)
- [Running Tests](#running-tests)
- [Adding New Proposal Spec Tests](#adding-new-proposal-spec-tests)
- [Test Script Options](#test-script-options)
- [Understanding Test Results](#understanding-test-results)
- [Troubleshooting](#troubleshooting)
- [Best Practices](#best-practices)
- [CI Integration](#ci-integration)
- [Reference](#reference)

---

## Overview

The WAMR test suites verify WebAssembly specification conformance for the WAMR runtime. These tests ensure that WAMR correctly implements the WebAssembly core specification and various proposals.

### What are WAMR Test Suites?

- **Spec Conformance Tests**: Test compliance with the official WebAssembly specification
- **Proposal Tests**: Validate implementation of WebAssembly proposals (SIMD, threads, GC, etc.)
- **WASI Tests**: Verify WASI (WebAssembly System Interface) compliance
- **Compiler Tests**: Test the ahead-of-time compiler (wamrc)

### Relationship to Other Tests

- **Unit Tests** (`core/unit-test/`): Test individual WAMR components in isolation
- **Regression Tests**: Prevent specific bugs from reoccurring
- **Spec Tests** (this directory): Comprehensive WebAssembly specification conformance

---

## ⚠️ Using the Devcontainer

**IMPORTANT**: All test commands must be run through the devcontainer for consistency and proper tooling.

### Running Commands in the Container

Wrap all commands with `./scripts/in-container.sh`:

```bash
# Pattern
./scripts/in-container.sh "<command>"

# Example: Run spec tests
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp"
```

### Why the Container is Required

- **Consistent Environment**: Same tools, versions, and dependencies across all machines
- **Pre-installed Tools**: wabt, spec test repositories, build tools
- **Reproducible Results**: CI and local testing use identical environments
- **Isolation**: Prevents conflicts with host system tools

### Quick Container Check

Verify the container is available:

```bash
./scripts/in-container.sh --status
```

For more details, see `doc/dev-in-container.md`.

---

## Test Categories

The test suite is organized into subdirectories by test type:

### `spec-test-script/`

**WebAssembly Core Specification Tests**

Tests the core WebAssembly specification and various proposals:
- Core spec (integers, floats, memory, control flow, etc.)
- SIMD proposal (vector operations)
- Threading proposal (shared memory, atomics)
- GC proposal (garbage collection)
- Memory64 proposal (64-bit memory addressing)
- Multi-memory proposal (multiple memory instances)
- Exception handling proposal
- Extended const expressions

Contains patch files (e.g., `gc_ignore_cases.patch`) for tests with known issues or expected failures.

### `wasi-test-script/`

**WASI Specification Tests**

Tests WASI (WebAssembly System Interface) compliance:
- File system operations
- Environment variables
- Command-line arguments
- Clock and time functions

### `wamr-compiler-test-script/`

**Compiler (wamrc) Tests**

Tests the WAMR ahead-of-time compiler:
- AOT compilation correctness
- Optimization passes
- Target architecture code generation

### `requirement-engineering-test-script/`

**Requirements-Based Tests**

Tests organized by functional requirements:
- Systematic verification of WAMR capabilities
- Traceability from requirements to tests

---

## Quick Start

### Run Spec Tests with Fast Interpreter

```bash
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp"
```

This will:
1. Create a `workspace/` directory
2. Download the WebAssembly spec test repository
3. Download wabt (WebAssembly Binary Toolkit)
4. Build `iwasm` with fast interpreter mode
5. Run all spec tests

### View Available Options

```bash
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh --help"
```

---

## Running Tests

### Run All Test Suites

```bash
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh"
```

### Run Specific Test Suite

```bash
# Spec tests only
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec"

# WASI tests only
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s wasi_certification"

# Compiler tests only
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s wamr_compiler"
```

### Run with Different Execution Modes

```bash
# Fast interpreter (recommended for development)
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp"

# Classic interpreter
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t classic-interp"

# AOT (Ahead-of-Time) compilation
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot"

# JIT (Just-in-Time) compilation
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t jit"

# Fast JIT
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-jit"

# Test all modes
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec"
```

### Enable WebAssembly Features

```bash
# Enable SIMD
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -S"

# Enable threading (pthreads)
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -p"

# Enable GC (garbage collection)
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -G"

# Enable Memory64
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -W"

# Enable multi-memory
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -E"

# Enable exception handling
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -e"

# Combine multiple features
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -S -p"
```

### Target Different Architectures

```bash
# x86_64 (default)
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -m x86_64"

# x86_32
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -m x86_32"

# ARM architectures
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -m armv7"
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -m aarch64"

# RISC-V
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -m riscv64"
```

### Use Pre-built wabt

Skip building wabt from source (faster startup):

```bash
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -b"
```

### Clean Previous Results

```bash
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -c"
```

---

## Adding New Proposal Spec Tests

This section covers how to integrate new WebAssembly proposal tests into the WAMR test suite.

### When to Add New Proposal Tests

Add proposal tests when:
- Implementing a new WebAssembly proposal in WAMR
- A proposal advances to Phase 3 or Phase 4 (candidate recommendation or standardized)
- Expanding coverage for existing but partially-tested proposals

### Understanding the Proposal Lifecycle

WebAssembly proposals follow a standardization process:
1. **Phase 0**: Pre-proposal (idea discussion)
2. **Phase 1**: Feature proposal (design review)
3. **Phase 2**: Spec text (formal specification)
4. **Phase 3**: Implementation phase (spec complete, ready for implementation)
5. **Phase 4**: Standardized (merged into core spec)

**Best Practice**: Integrate tests when implementing Phase 3+ proposals.

### Directory Structure for Proposals

Proposal tests are located in `spec-test-script/`:

```
tests/wamr-test-suites/spec-test-script/
├── all.py                              # Main test runner
├── runtest.py                          # Test execution logic
├── ignore_cases.patch                  # Core spec ignored tests
├── simd_ignore_cases.patch            # SIMD-specific ignored tests
├── gc_ignore_cases.patch              # GC-specific ignored tests
├── memory64_ignore_cases.patch        # Memory64-specific ignored tests
├── thread_proposal_ignore_cases.patch # Threading-specific ignored tests
├── exception_handling.patch           # Exception handling adjustments
└── ...
```

### Step-by-Step: Adding a New Proposal

#### 1. Check Proposal Status

Verify the proposal is ready for implementation:

```bash
# Visit the proposal repository
# Example: https://github.com/WebAssembly/memory64
# Example: https://github.com/WebAssembly/gc
```

Check:
- Current phase (should be Phase 3+)
- Test availability in the WebAssembly testsuite repository
- Implementation status in other runtimes

#### 2. Locate Proposal Spec Tests

Proposal tests are typically in the WebAssembly testsuite repository:

```bash
# The test_wamr.sh script clones the testsuite automatically
# It will be in: workspace/spec/test/
# Proposal-specific tests are in subdirectories or proposal branches
```

For proposals not yet merged:
```bash
# Check the WebAssembly/testsuite repository
# Look for proposal branches or proposal-specific test directories
# Example: https://github.com/WebAssembly/testsuite/tree/memory64
```

#### 3. Enable the Proposal in the Build

Update WAMR's CMake configuration to enable the proposal:

```bash
# Example: Enable Memory64
./scripts/in-container.sh "cd /wamr && cmake -B build -DWAMR_BUILD_MEMORY64=1"

# Example: Enable GC
./scripts/in-container.sh "cd /wamr && cmake -B build -DWAMR_BUILD_GC=1"

# Example: Enable multi-memory
./scripts/in-container.sh "cd /wamr && cmake -B build -DWAMR_BUILD_MULTI_MEMORY=1"
```

The test script (`test_wamr.sh`) automatically handles build flags based on command-line options (e.g., `-W` for memory64, `-G` for GC).

#### 4. Create Ignore Cases Patch (If Needed)

Some tests may fail initially or be unsupported. Create a patch file to ignore them:

```bash
cd tests/wamr-test-suites/spec-test-script/

# Create a patch file named <proposal>_ignore_cases.patch
# Example format:
cat > my_proposal_ignore_cases.patch << 'EOF'
--- a/test/core/my_proposal_test.wast
+++ b/test/core/my_proposal_test.wast
@@ -100,6 +100,7 @@
 ;; Test case that fails
+(; temporarily disabled
 (assert_return
   (invoke "my_function" (i64.const 0))
   (i32.const 1))
+;)
EOF
```

Study existing patches (e.g., `memory64_ignore_cases.patch`, `gc_ignore_cases.patch`) for patterns.

#### 5. Update Test Runner (If Needed)

If the proposal requires special handling, update `spec-test-script/runtest.py` or `all.py`:

```python
# Example: Add proposal-specific test filtering
if args.memory64:
    apply_patch("memory64_ignore_cases.patch")
    test_suite.append("memory64")
```

Most proposals integrate automatically through the existing test infrastructure.

#### 6. Run the Proposal Tests

```bash
# Example: Test Memory64 proposal
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -W"

# Example: Test GC proposal
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -G"

# Example: Test with AOT compilation
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -W"
```

#### 7. Analyze Test Results

Review the output:

```
Running proposal tests...
Passed: 145/150
Failed: 5/150
Ignored: 0
```

Investigate failures:
- Expected failures (known limitations) → Add to ignore patch
- Bugs in implementation → Fix in WAMR source code
- Test issues → Report upstream to WebAssembly/testsuite

#### 8. Document the Proposal Support

Update relevant documentation:
- Add proposal to `doc/building.md` build options
- Update `README.md` with proposal status
- Document any limitations or caveats

### Example: Memory64 Proposal Tests

Complete example of integrating Memory64 proposal tests:

```bash
# 1. Check the proposal
# Visit: https://github.com/WebAssembly/memory64

# 2. Enable Memory64 in the test run
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -W"

# 3. The test script will:
#    - Build iwasm with WAMR_BUILD_MEMORY64=1
#    - Apply memory64_ignore_cases.patch
#    - Run memory64-specific tests from the spec

# 4. Review results
#    - Check workspace/spec-test-fast-interp/report.txt
#    - Investigate any unexpected failures

# 5. Test with AOT for comprehensive coverage
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -W"

# 6. Verify combinations work
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -W -S"
```

### Example: Exception Handling Proposal

```bash
# 1. Enable exception handling
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -e"

# 2. The exception_handling.patch is applied automatically
# 3. Tests are run against the exception handling proposal spec

# 4. Test with different modes
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -e"
```

### Troubleshooting Proposal Integration

**Problem**: Tests not found

```bash
# Solution: Ensure the spec test repository is up-to-date
./scripts/in-container.sh "cd tests/wamr-test-suites && rm -rf workspace"
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -W"
```

**Problem**: All tests fail with build errors

```bash
# Solution: Verify the proposal CMake flag is correct
# Check product-mini/platforms/linux/CMakeLists.txt for available options
```

**Problem**: Tests pass locally but fail in CI

```bash
# Solution: Ensure the CI configuration enables the proposal
# Check .github/workflows/ for workflow configurations
```

---

## Test Script Options

Complete reference for `test_wamr.sh` command-line options:

### Test Suite Selection

- `-s <suite>` / `--suite <suite>`: Run specific test suite
  - `spec`: WebAssembly spec tests (default)
  - `wasi_certification`: WASI certification tests
  - `wamr_compiler`: Compiler (wamrc) tests
  - `standalone`: Standalone test cases
  - `malformed`: Malformed wasm module tests
  - `unit`: Unit tests

### Execution Mode

- `-t <mode>` / `--target <mode>`: Set execution mode
  - `classic-interp`: Classic interpreter
  - `fast-interp`: Fast interpreter (default)
  - `aot`: Ahead-of-time compilation
  - `jit`: Just-in-time compilation
  - `fast-jit`: Fast JIT
  - `multi-tier-jit`: Multi-tier JIT

### Architecture

- `-m <arch>` / `--target-arch <arch>`: Target architecture
  - `x86_64`, `x86_32`
  - `armv7`, `armv7_vfp`, `thumbv7`, `thumbv7_vfp`
  - `aarch64`, `aarch64_vfp`
  - `riscv32`, `riscv32_ilp32f`, `riscv32_ilp32d`
  - `riscv64`, `riscv64_lp64f`, `riscv64_lp64d`

### WebAssembly Features

- `-S` / `--enable-simd`: Enable SIMD
- `-p` / `--enable-pthread`: Enable threading (pthreads)
- `-G` / `--enable-gc`: Enable GC proposal
- `-W` / `--enable-memory64`: Enable Memory64 proposal
- `-E` / `--enable-multi-memory`: Enable multi-memory proposal
- `-M` / `--enable-multi-module`: Enable multi-module feature
- `-e` / `--enable-exception-handling`: Enable exception handling proposal
- `-N` / `--enable-extended-const`: Enable extended const expressions
- `-w` / `--enable-wasi-threads`: Enable WASI threads

### Build Options

- `-b` / `--use-binary`: Use wabt binary release (don't compile from source)
- `-g` / `--debug`: Build iwasm with debug symbols
- `-A <path>` / `--wamrc-path <path>`: Use specified wamrc instead of building

### Test Execution

- `-P` / `--parallel`: Run spec tests in parallel
- `-c` / `--clean`: Clean previous results (don't run tests)
- `-r <requirement> [IDs...]` / `--requirement <requirement> [IDs...]`: Run specific requirement tests

### Advanced Options

- `-x` / `--test-sgx`: Test with SGX
- `-X` / `--enable-xip`: Enable XIP (execute-in-place)
- `-v` / `--verify-gc-heap`: Enable GC heap verification
- `-C` / `--coverage`: Enable code coverage collection
- `-T <sanitizers>` / `--sanitizer <sanitizers>`: Use sanitizers (ubsan, asan, tsan, posan)
- `-Q` / `--enable-qemu`: Enable QEMU
- `-F <path>` / `--firmware-path <path>`: Set firmware path for QEMU
- `-j <platform>` / `--platform <platform>`: Set platform to test

---

## Understanding Test Results

### Test Output Format

```
========================================
Running spec tests in fast-interp mode
========================================

Building iwasm...
[100%] Built target iwasm

Running test suite...
Test: core/i32.wast                    PASSED
Test: core/i64.wast                    PASSED
Test: core/f32.wast                    PASSED
Test: core/memory.wast                 FAILED
Test: core/simd_i32x4.wast             PASSED

========================================
Test Summary
========================================
Total:    250
Passed:   248
Failed:   2
Ignored:  0
Pass Rate: 99.2%
```

### Pass/Fail Criteria

- **PASSED**: All assertions in the test file succeeded
- **FAILED**: One or more assertions failed
- **IGNORED**: Test was skipped (typically via ignore patch)

### Test Artifacts

Test results are stored in the `workspace/` directory:

```
workspace/
├── spec-test-fast-interp/
│   ├── report.txt              # Test summary
│   ├── test.log                # Detailed test output
│   └── failed_tests.txt        # List of failed tests
├── spec/                       # WebAssembly spec repository
└── wabt/                       # WebAssembly Binary Toolkit
```

### Analyzing Failures

1. **Check the report**:
   ```bash
   cat workspace/spec-test-fast-interp/report.txt
   ```

2. **Review detailed logs**:
   ```bash
   cat workspace/spec-test-fast-interp/test.log | grep -A 10 "FAILED"
   ```

3. **Investigate specific test**:
   ```bash
   # Look at the failing test file
   cat workspace/spec/test/core/memory.wast
   ```

4. **Run single test**:
   ```bash
   # Most test runners support running individual tests
   # Check spec-test-script/runtest.py for options
   ```

### Common Failure Patterns

- **Assertion mismatch**: Expected value differs from actual
  - Usually indicates implementation bug

- **Timeout**: Test took too long
  - May indicate infinite loop or performance issue

- **Crash**: Runtime crashed during test
  - Serious bug requiring investigation

- **Parse error**: Test file couldn't be parsed
  - May indicate wabt version incompatibility

---

## Troubleshooting

### Test Failures After Adding New Feature

**Symptom**: Tests that previously passed now fail.

**Solution**:
```bash
# 1. Clean and rebuild
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -c"
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp"

# 2. Test without new feature flag
# Determine if the feature broke existing functionality

# 3. Run with debug build
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -g"
```

### Timeout on Slow Machines

**Symptom**: Tests timeout before completion.

**Solution**:
```bash
# Don't run in parallel mode
# Remove -P flag if using it

# Use fewer test modes
# Run interp OR aot, not both

# Use pre-built wabt
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -b"
```

### Missing Dependencies

**Symptom**: "command not found" or "repository not found" errors.

**Solution**:
```bash
# Ensure you're using the devcontainer
./scripts/in-container.sh --status

# If container isn't available, see doc/dev-in-container.md

# Rebuild container if needed
docker-compose -f .devcontainer/docker-compose.yml build
```

### Container Issues

**Symptom**: Container won't start or commands fail.

**Solution**:
```bash
# Check container status
docker ps -a | grep wamr

# View container logs
docker logs <container-id>

# Restart container
docker-compose -f .devcontainer/docker-compose.yml restart

# Rebuild container (last resort)
docker-compose -f .devcontainer/docker-compose.yml down
docker-compose -f .devcontainer/docker-compose.yml build --no-cache
```

### Patch Application Failures

**Symptom**: "patch does not apply" errors.

**Solution**:
```bash
# The spec test repository may have changed
# Update the patch file in spec-test-script/

# To regenerate a patch:
cd workspace/spec
# Make manual edits to test files
git diff > ../../spec-test-script/my_proposal_ignore_cases.patch
```

### Inconsistent Results Between Modes

**Symptom**: Tests pass in interpreter but fail in AOT/JIT.

**Solution**:
```bash
# This indicates a code generation bug
# File a bug report with reproduction steps

# Test each mode individually
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp"
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot"
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t jit"
```

---

## Best Practices

### Before Submitting a Pull Request

1. **Run tests with your changes**:
   ```bash
   ./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp"
   ```

2. **Test multiple modes** (if your change affects codegen):
   ```bash
   ./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot"
   ./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t jit"
   ```

3. **Test with relevant features enabled**:
   ```bash
   # If your change affects SIMD
   ./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -S"
   ```

### Implementing New Proposals

1. **Add tests early**: Integrate spec tests as you implement the proposal
2. **Use ignore patches**: Mark unsupported tests as ignored rather than letting them fail
3. **Document limitations**: Clearly document what's not yet supported
4. **Test incrementally**: Run tests frequently during development

### Feature Flags

When enabling features:
```bash
# Test feature alone
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -S"

# Test feature combinations
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -S -p"

# Test without feature (regression check)
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot"
```

### Performance Testing

```bash
# Use AOT for production-like performance
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot"

# Enable optimizations
# (Built automatically by test script with -O3)
```

### Debugging Test Failures

```bash
# 1. Build with debug symbols
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -g"

# 2. Run single failing test
# (Examine workspace/spec-test-fast-interp/failed_tests.txt)

# 3. Use gdb for crashes
./scripts/in-container.sh "cd tests/wamr-test-suites/workspace && gdb --args ./iwasm test.wasm"

# 4. Enable runtime logging
# Set WASM_ENABLE_LOG=1 in CMakeLists.txt
```

---

## CI Integration

### How Tests Run in CI

WAMR's CI (GitHub Actions) runs spec tests automatically on every pull request.

**CI Workflow** (`.github/workflows/`):
- Builds WAMR in multiple configurations
- Runs spec tests with different modes (interp, AOT, JIT)
- Tests with various feature flags (SIMD, threads, GC, etc.)
- Tests on different architectures (x86_64, ARM, RISC-V via QEMU)

### Mandatory Tests

All PRs must pass:
- Core spec tests (fast-interp mode)
- Core spec tests (AOT mode)
- WASI certification tests

Feature-specific PRs must also pass:
- Relevant proposal tests (e.g., GC tests for GC changes)
- Tests with the feature enabled and disabled

### Checking CI Test Results

1. **View workflow runs**: Go to the PR's "Checks" tab on GitHub
2. **Check specific jobs**: Click on individual workflow jobs
3. **Review logs**: Expand test steps to see detailed output
4. **Download artifacts**: CI may upload test reports as artifacts

### Local Reproduction of CI Failures

```bash
# CI uses the same test scripts
# Reproduce CI environment using the devcontainer

# Example: Reproduce "spec-test-aot" CI job
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot"

# Example: Reproduce "spec-test-simd" CI job
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -S"
```

---

## Reference

### Related Documentation

- **`doc/testing.md`**: Comprehensive testing overview
- **`doc/building.md`**: Build options and CMake flags
- **`doc/dev-in-container.md`**: Devcontainer setup and usage
- **`AGENTS.md`**: AI assistant guidelines (includes testing requirements)
- **`tests/unit-test/README.md`**: Unit testing guide

### External Resources

- **WebAssembly Specification**: https://webassembly.github.io/spec/
- **WebAssembly Proposals**: https://github.com/WebAssembly/proposals
- **WebAssembly Test Suite**: https://github.com/WebAssembly/testsuite
- **WASI Specification**: https://github.com/WebAssembly/WASI
- **wabt (WebAssembly Binary Toolkit)**: https://github.com/WebAssembly/wabt

### Test Suite Repository Structure

```
tests/wamr-test-suites/
├── README.md                              # This file
├── test_wamr.sh                           # Main test script
├── spec-test-script/                      # Spec test runner and patches
│   ├── runtest.py                         # Test execution logic
│   ├── all.py                             # Test suite configuration
│   ├── *_ignore_cases.patch               # Known test exclusions
│   └── ...
├── wasi-test-script/                      # WASI test runner
├── wamr-compiler-test-script/             # Compiler tests
├── requirement-engineering-test-script/   # Requirements tests
└── workspace/                             # Generated test workspace
    ├── spec/                              # WebAssembly spec repo
    ├── wabt/                              # wabt tools
    └── spec-test-*/                       # Test results per mode
```

### Quick Reference Commands

```bash
# Most common commands

# Run spec tests (fast interpreter)
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp"

# Run spec tests (AOT)
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot"

# Run with SIMD
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -S"

# Run with threading
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -p"

# Run WASI tests
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s wasi_certification"

# Clean workspace
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -c"

# View help
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh --help"
```

---

**For questions or issues with the test suite, see `doc/testing.md` or file an issue on GitHub.**

