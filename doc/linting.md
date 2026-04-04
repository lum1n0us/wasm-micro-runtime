# Linting and Pre-Commit Checklist

This document provides a comprehensive guide for code quality checks that must be performed before committing changes to WAMR. Following these checks ensures code quality, prevents regressions, and maintains consistency across the codebase.

---

## Overview

### What is Pre-Commit Checking?

Pre-commit checking is the practice of validating code quality, formatting, and correctness before committing changes to version control. This ensures that:

- **Code meets quality standards** - Formatting is consistent, style guidelines are followed
- **Tests pass** - New changes don't break existing functionality
- **Regressions are prevented** - Previously fixed bugs don't reappear
- **CI/CD succeeds** - Local validation matches what will run in continuous integration

### Why These Checks are Mandatory

Running these checks before committing:

1. **Catches issues early** - Faster to fix locally than after pushing
2. **Saves CI resources** - Avoids wasting CI time on preventable failures
3. **Maintains code quality** - Ensures consistent standards across the codebase
4. **Reduces review time** - Reviewers can focus on logic, not formatting
5. **Prevents breaking changes** - Tests validate that functionality still works

### Integration with CI/CD

All checks documented here run automatically in the CI pipeline. When CI fails, you can reproduce the failure locally using the exact same commands and environment (via the devcontainer). This ensures "works on my machine" issues don't occur.

---

## Container Requirement

**CRITICAL**: All code quality checks, builds, and tests MUST run inside the devcontainer to ensure environment consistency.

### Using the Container Wrapper

All commands in this document use the wrapper script:

```bash
./scripts/in-container.sh "<command>"
```

This script automatically:
- Detects existing devcontainers
- Starts the container if not running
- Executes commands in the correct environment
- Returns proper exit codes for error handling

**Detailed guide**: [doc/dev-in-container.md](dev-in-container.md)

**Never run build, test, or format commands directly on your host system.** Always use the wrapper.

---

## Quick Pre-Commit Checklist

Use this checklist before every commit:

**For All Changes:**
- [ ] Code format check passes (clang-format-14)
- [ ] No new compiler warnings
- [ ] Code builds successfully

**For Code Changes:**
- [ ] Unit tests pass (if available for changed component)
- [ ] No memory leaks in critical paths

**For VMcore Changes:**
- [ ] Spec tests pass in affected modes (fast-interp, jit, aot)

**For Bug Fixes:**
- [ ] Regression test added
- [ ] Regression tests pass

**Optional but Recommended:**
- [ ] Static analysis passes (scan-build)
- [ ] Changed code reviewed for common issues

---

## 1. Code Format Check

WAMR uses **clang-format-14** to enforce consistent code formatting according to the K&R style defined in `.clang-format`.

### Check Single File

Check if a file needs formatting (dry-run mode):

```bash
./scripts/in-container.sh "clang-format-14 --dry-run --Werror core/iwasm/common/wasm_runtime_common.c"
```

**Expected output:**
- If formatted correctly: (no output, exit code 0)
- If needs formatting: Shows diff and exits with error code

### Check Multiple Files

Check all C/C++ files in a directory:

```bash
./scripts/in-container.sh "find core/iwasm/interpreter -name '*.c' -o -name '*.h' | xargs clang-format-14 --dry-run --Werror"
```

### Check Git Staged Changes

Check only files you're about to commit:

```bash
./scripts/in-container.sh "git diff --cached --name-only --diff-filter=ACM | grep '\.\(c\|h\|cpp\|hpp\)$' | xargs -r clang-format-14 --dry-run --Werror"
```

### Check All Modified Files

Check all files changed in your current branch compared to main:

```bash
./scripts/in-container.sh "git diff main...HEAD --name-only --diff-filter=ACM | grep '\.\(c\|h\|cpp\|hpp\)$' | xargs -r clang-format-14 --dry-run --Werror"
```

### Auto-Fix Formatting

Apply formatting automatically to fix issues:

```bash
# Fix single file
./scripts/in-container.sh "clang-format-14 -i core/iwasm/common/wasm_runtime_common.c"

# Fix all staged files
./scripts/in-container.sh "git diff --cached --name-only --diff-filter=ACM | grep '\.\(c\|h\|cpp\|hpp\)$' | xargs -r clang-format-14 -i"

# Fix all modified files in branch
./scripts/in-container.sh "git diff main...HEAD --name-only --diff-filter=ACM | grep '\.\(c\|h\|cpp\|hpp\)$' | xargs -r clang-format-14 -i"
```

After auto-fixing, review the changes with `git diff` before committing.

### CI Format Check Command

The CI pipeline runs format checks like this:

```bash
./scripts/in-container.sh "git diff --name-only HEAD~1 | grep '\.\(c\|h\|cpp\|hpp\)$' | xargs -r clang-format-14 --dry-run --Werror"
```

### Troubleshooting Format Issues

**Issue**: clang-format command not found

```bash
# Verify clang-format-14 is installed
./scripts/in-container.sh "which clang-format-14"

# Check version
./scripts/in-container.sh "clang-format-14 --version"
```

**Issue**: Format check fails but you see no diff

```bash
# Run without --Werror to see the actual diff
./scripts/in-container.sh "clang-format-14 --dry-run core/iwasm/common/wasm_runtime_common.c"
```

**Issue**: Some files should not be formatted

Exclude third-party code, generated files, or special cases using `grep -v`:

```bash
./scripts/in-container.sh "git diff --cached --name-only | grep '\.\(c\|h\)$' | grep -v 'third_party/' | xargs -r clang-format-14 --dry-run --Werror"
```

---

## 2. Unit Tests Check

Unit tests validate individual components and functions in isolation. WAMR uses CMake's CTest framework for unit testing.

### Building with Tests

First, configure and build WAMR with unit tests enabled:

```bash
# Configure with unit tests
./scripts/in-container.sh "cmake -B build -DWAMR_BUILD_UNIT_TEST=1"

# Build
./scripts/in-container.sh "cmake --build build -j$(nproc)"
```

### Running All Unit Tests

Run the entire unit test suite:

```bash
./scripts/in-container.sh "cd build && ctest --output-on-failure"
```

**Expected output:**
```
Test project /workspaces/ai-thoughts/build
    Start 1: test_wasm_runtime
1/5 Test #1: test_wasm_runtime ................   Passed    0.12 sec
    Start 2: test_shared_utils
2/5 Test #2: test_shared_utils ................   Passed    0.08 sec
...
100% tests passed, 0 tests failed out of 5
```

### Running Specific Tests

Run tests matching a pattern:

```bash
# Run specific test by name
./scripts/in-container.sh "cd build && ctest -R test_wasm_runtime --verbose"

# Run tests for a specific component
./scripts/in-container.sh "cd build && ctest -R 'test_(runtime|memory)' --verbose"
```

### Running Tests with Verbose Output

For debugging test failures:

```bash
./scripts/in-container.sh "cd build && ctest --verbose --output-on-failure"
```

### Interpreting Test Results

**Success**: All tests pass, exit code 0
```
100% tests passed, 0 tests failed out of 5
Total Test time (real) = 0.82 sec
```

**Failure**: One or more tests fail, exit code non-zero
```
The following tests FAILED:
      3 - test_memory_management (Failed)
Errors while running CTest
```

Check the detailed output to see assertion failures, crashes, or timeouts.

### Troubleshooting Test Failures

**Issue**: Tests fail after code changes

1. Check test output for assertion failures or error messages
2. Run the specific failing test with `--verbose` for more details
3. Verify your changes didn't break the tested functionality
4. Update test expectations if behavior intentionally changed

**Issue**: Tests timeout

```bash
# Increase timeout for specific test
./scripts/in-container.sh "cd build && ctest -R test_name --timeout 300"
```

**Issue**: Tests pass locally but fail in CI

- Ensure you're using the devcontainer (not host environment)
- Check if CI uses different CMake flags
- Verify test doesn't depend on timing or ordering

---

## 3. Spec Tests Check

WebAssembly spec tests validate conformance with the WebAssembly specification. Run these when making changes to the VM core, implementing new proposals, or modifying execution behavior.

### When to Run Spec Tests

Run spec tests if you:
- Modify VMcore interpreter logic (`core/iwasm/interpreter/`)
- Change JIT or AOT compilation (`core/iwasm/compilation/`, `core/iwasm/aot/`)
- Implement new WebAssembly proposals (SIMD, GC, threads, etc.)
- Touch memory management or validation logic
- Fix bugs related to spec compliance

### Running Spec Tests

The spec test suite is in `tests/wamr-test-suites/`. Tests run against different execution modes.

**Run spec tests with fast-interp mode:**

```bash
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp"
```

**Run spec tests with classic-interp mode:**

```bash
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t classic-interp"
```

**Run spec tests with JIT mode:**

```bash
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t jit"
```

**Run spec tests with AOT mode:**

```bash
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot"
```

### Expected Pass Rates

WAMR has near-complete spec compliance. Expected pass rates:

- **fast-interp**: 99%+ (core spec)
- **classic-interp**: 99%+ (core spec)
- **jit**: 98%+ (most proposals supported)
- **aot**: 98%+ (most proposals supported)

Some tests may be expected to fail for:
- Features not yet implemented (certain proposals)
- Known limitations documented in issue tracker
- Platform-specific restrictions

### Running Specific Spec Test Suites

Test specific WebAssembly proposals:

```bash
# SIMD spec tests
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -S"

# GC spec tests
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -G"

# Multi-memory spec tests
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -E"
```

### Handling Spec Test Failures

**Check test report:**

```bash
./scripts/in-container.sh "cat tests/wamr-test-suites/workspace/report/spec_test_report.txt"
```

**Analyze specific failure:**

1. Note which test failed (e.g., `br_if.wast`)
2. Look at the failure reason (assertion, trap, wrong result)
3. Run the specific test manually for debugging:

```bash
./scripts/in-container.sh "cd tests/wamr-test-suites/workspace && ./iwasm --interp test.wasm"
```

**Expected failures:**

Some spec tests are known to fail due to unimplemented proposals or known limitations. Check existing issues before treating as new bug.

---

## 4. Regression Tests Check

Regression tests verify that previously fixed bugs don't reoccur. Run these when fixing bugs or making changes that could affect existing fixes.

### When to Run Regression Tests

Run regression tests if you:
- Fixed a reported bug
- Made changes to VMcore execution logic
- Modified memory management or bounds checking
- Changed validation or verification code
- Refactored significant portions of runtime code

### Running All Regression Tests

**Build test infrastructure:**

```bash
./scripts/in-container.sh "cd tests/regression/ba-issues && ./build_wamr.sh"
```

**Run all regression tests:**

```bash
./scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py"
```

**Expected output:**
```
==== Test results ====
   Total: 22
  Passed: 22
  Failed: 0
  Left issues in folder: no more
  Cases in JSON but not found in folder: no more
```

### Running Specific Regression Tests

Test specific issue numbers:

```bash
# Single issue
./scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py --issues 2833"

# Multiple issues
./scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py -i 2833,2834,2835"
```

### Adding New Regression Tests

When you fix a bug, always add a regression test:

**Step 1: Create test directory**

```bash
./scripts/in-container.sh "cd tests/regression/ba-issues && ./helper.sh 3022"
```

**Step 2: Add test artifacts**

Place your WASM test file and any other needed files in `tests/regression/ba-issues/issues/issue-3022/`.

**Step 3: Configure test**

Add an entry to `tests/regression/ba-issues/running_config.json`:

```json
{
    "deprecated": false,
    "ids": [3022],
    "runtime": "iwasm-default",
    "file": "test.wasm",
    "mode": "fast-interp",
    "options": "--heap-size=0 -f main",
    "argument": "",
    "expected return": {
        "ret code": 0,
        "stdout content": "",
        "description": "Issue #3022: should not crash on xyz"
    }
}
```

**Step 4: Verify test passes**

```bash
./scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py -i 3022"
```

**Step 5: Commit with your fix**

```bash
git add tests/regression/ba-issues/issues/issue-3022/
git add tests/regression/ba-issues/running_config.json
git commit -m "fix: resolve issue #3022 with regression test"
```

**Detailed guide**: [tests/regression/README.md](../tests/regression/README.md)

### Troubleshooting Regression Test Failures

**View detailed failure log:**

```bash
./scripts/in-container.sh "cd tests/regression/ba-issues && cat issues_tests.log"
```

**Test fails after code changes:**

1. Check if your changes reintroduced the bug (fix your code)
2. Check if expected behavior changed (update test config)
3. Check if test setup is incorrect (fix test artifacts)

---

## 5. Compiler Warnings Check

WAMR aims for warning-free builds. New code should not introduce compiler warnings.

### Building with Warnings as Errors

Configure build to treat warnings as errors:

```bash
./scripts/in-container.sh "cmake -B build -DCMAKE_C_FLAGS='-Wall -Werror' -DCMAKE_CXX_FLAGS='-Wall -Werror'"
./scripts/in-container.sh "cmake --build build"
```

If the build succeeds, you have no warnings.

### Common Warnings and Fixes

**Unused variable:**
```c
// Warning: unused variable 'result'
int result = some_function();

// Fix: use it or mark as unused
(void)result;  // or __attribute__((unused))
```

**Uninitialized variable:**
```c
// Warning: 'ptr' may be used uninitialized
int *ptr;
if (condition) ptr = &value;
return *ptr;  // Used even if condition was false

// Fix: initialize or ensure all paths set it
int *ptr = NULL;
```

**Implicit function declaration:**
```c
// Warning: implicit declaration of function 'foo'
result = foo();

// Fix: include proper header
#include "foo.h"
```

**Pointer type mismatch:**
```c
// Warning: incompatible pointer types
uint32 *ptr = (int *)some_ptr;

// Fix: use correct type or explicit cast with comment
uint32 *ptr = (uint32 *)some_ptr;  /* Safe: validated earlier */
```

### Platform-Specific Warnings

Some warnings only appear on specific platforms:

```bash
# Check x86_64 build
./scripts/in-container.sh "cmake -B build-x64 -DWAMR_BUILD_TARGET=X86_64 -DCMAKE_C_FLAGS='-Wall -Werror' && cmake --build build-x64"

# Check ARM build (if cross-compiling)
./scripts/in-container.sh "cmake -B build-arm -DWAMR_BUILD_TARGET=ARMV7 -DCMAKE_C_FLAGS='-Wall -Werror' && cmake --build build-arm"
```

---

## 6. Static Analysis (Optional but Recommended)

Static analysis tools find potential bugs without running code.

### Using Clang Static Analyzer

Configure build with static analyzer:

```bash
./scripts/in-container.sh "cmake -B build-analyze -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++"
./scripts/in-container.sh "cd build-analyze && scan-build -o ../analysis-results make -j$(nproc)"
```

### Interpreting Results

The analyzer generates HTML reports in `analysis-results/`:

```bash
./scripts/in-container.sh "ls -la analysis-results/"
```

View reports to see:
- Null pointer dereferences
- Memory leaks
- Use-after-free
- Dead code
- Logic errors

**Note**: Static analysis may report false positives. Review each finding carefully.

---

## 7. Memory Leak Check (For Critical Changes)

Use Valgrind to detect memory leaks when making changes to memory management.

### Running Tests Under Valgrind

```bash
# Build with debug symbols
./scripts/in-container.sh "cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug"
./scripts/in-container.sh "cmake --build build-debug"

# Run iwasm under Valgrind
./scripts/in-container.sh "valgrind --leak-check=full --show-leak-kinds=all ./build-debug/product-mini/platforms/linux/build/iwasm test.wasm"
```

### Interpreting Leak Reports

**No leaks:**
```
HEAP SUMMARY:
    in use at exit: 0 bytes in 0 blocks
  total heap usage: 100 allocs, 100 frees, 10,240 bytes allocated

All heap blocks were freed -- no leaks are possible
```

**Leaks detected:**
```
LEAK SUMMARY:
   definitely lost: 1,024 bytes in 1 blocks
   indirectly lost: 512 bytes in 8 blocks
     possibly lost: 0 bytes in 0 blocks
```

Review the detailed backtrace to identify where memory was allocated but not freed.

### Common Leak Sources

- Missing `wasm_runtime_free()` calls
- Not destroying module instances
- Circular references in data structures
- Error paths that skip cleanup

---

## 8. Complete Pre-Commit Workflow

This section provides a step-by-step example showing all checks for a typical contribution.

### Scenario: Bug Fix in VMcore

You've fixed a bug in the interpreter. Here's the complete workflow:

**Step 1: Format Check**

```bash
# Check formatting of changed files
./scripts/in-container.sh "git diff main...HEAD --name-only --diff-filter=ACM | grep '\.\(c\|h\)$' | xargs -r clang-format-14 --dry-run --Werror"

# Auto-fix if needed
./scripts/in-container.sh "git diff main...HEAD --name-only --diff-filter=ACM | grep '\.\(c\|h\)$' | xargs -r clang-format-14 -i"
```

**Step 2: Build and Check Warnings**

```bash
./scripts/in-container.sh "cmake -B build -DCMAKE_C_FLAGS='-Wall -Werror' && cmake --build build -j$(nproc)"
```

**Step 3: Unit Tests**

```bash
./scripts/in-container.sh "cd build && ctest --output-on-failure"
```

**Step 4: Spec Tests (VMcore change)**

```bash
# Test the affected mode
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp"
```

**Step 5: Add and Run Regression Test**

```bash
# Create test
./scripts/in-container.sh "cd tests/regression/ba-issues && ./helper.sh 3022"

# Add test files and config (manual step)

# Build test infrastructure
./scripts/in-container.sh "cd tests/regression/ba-issues && ./build_wamr.sh"

# Run the new test
./scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py -i 3022"

# Run all regression tests
./scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py"
```

**Step 6: Stage and Commit**

```bash
git add core/iwasm/interpreter/wasm_interp_fast.c
git add tests/regression/ba-issues/issues/issue-3022/
git add tests/regression/ba-issues/running_config.json
git commit -m "fix: resolve interpreter bug in br_if instruction (issue #3022)

- Fixed incorrect branch target calculation
- Added regression test to prevent recurrence
- All spec and regression tests pass"
```

### Scenario: New Feature

You've added a new feature to the runtime:

**Step 1-3: Same as above (format, build, unit tests)**

**Step 4: Feature-Specific Tests**

```bash
# If adding SIMD support
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -S"

# If adding multi-memory support
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -E"
```

**Step 5: Integration Tests**

```bash
# Run relevant samples
./scripts/in-container.sh "cd samples/basic && ./build.sh && ./run.sh"
```

**Step 6: Documentation Check**

Ensure you've updated relevant documentation in `doc/` if the feature affects public APIs or configuration.

---

## 9. CI/CD Integration

### What Checks Run in CI

The CI pipeline typically runs:

1. **Code format check** - All changed files must be formatted
2. **Build check** - Multiple configurations (interp, jit, aot)
3. **Unit tests** - All unit tests must pass
4. **Spec tests** - Core WebAssembly spec compliance
5. **Regression tests** - All known bugs must stay fixed
6. **Platform builds** - Linux, macOS, Windows, embedded targets
7. **Sample builds** - Example applications must build and run

### Reproducing CI Failures Locally

When CI fails, reproduce locally:

**Step 1: Check CI logs for failing command**

Example CI failure:
```
Run: clang-format-14 --dry-run --Werror core/iwasm/common/wasm_runtime_common.c
Error: code not formatted correctly
```

**Step 2: Run same command locally**

```bash
./scripts/in-container.sh "clang-format-14 --dry-run --Werror core/iwasm/common/wasm_runtime_common.c"
```

**Step 3: Fix the issue**

```bash
./scripts/in-container.sh "clang-format-14 -i core/iwasm/common/wasm_runtime_common.c"
git add core/iwasm/common/wasm_runtime_common.c
git commit --amend --no-edit
git push --force-with-lease
```

### CI Failure Troubleshooting

**Format check fails in CI but passes locally:**

- Ensure you're using clang-format-14 (not a different version)
- Check you've committed the formatted files
- Verify `.clang-format` config hasn't changed

**Tests pass locally but fail in CI:**

- Check CI uses same container/environment (it should)
- Look for timing dependencies (tests should be deterministic)
- Check if test depends on specific CPU features or platform

**Build fails in CI for specific platform:**

- CI builds for multiple platforms (ARM, RISC-V, etc.)
- Cross-compile locally to reproduce:

```bash
./scripts/in-container.sh "cmake -B build-arm -DWAMR_BUILD_TARGET=ARMV7 && cmake --build build-arm"
```

**Timeout in CI:**

- CI may have shorter timeouts than local environment
- Optimize slow tests or mark as long-running

---

## 10. Troubleshooting

### Common Issues and Solutions

**Issue: Format check fails but no visible difference**

```bash
# View actual diff
./scripts/in-container.sh "clang-format-14 file.c | diff -u file.c -"

# Check for invisible characters (tabs vs spaces)
./scripts/in-container.sh "cat -A file.c | head -20"
```

**Issue: Tests fail locally but not in CI**

- You may be running on host instead of container
- Always use `./scripts/in-container.sh` wrapper
- Check container is up-to-date:

```bash
./scripts/in-container.sh --status
```

**Issue: Tests timeout**

```bash
# Increase timeout
./scripts/in-container.sh "cd build && ctest --timeout 600"

# Run specific test to debug
./scripts/in-container.sh "cd build && ctest -R test_name --verbose"
```

**Issue: Container problems**

```bash
# Check container status
./scripts/in-container.sh --status

# Restart container
docker restart <container-name>

# Rebuild container (VS Code)
# F1 -> "Dev Containers: Rebuild Container"
```

**Issue: Out of disk space**

```bash
# Clean build artifacts
./scripts/in-container.sh "rm -rf build"

# Clean Docker resources
docker system prune -a
```

**Issue: Regression test infrastructure not built**

```bash
# Error: iwasm-default not found
# Solution: Build test infrastructure first
./scripts/in-container.sh "cd tests/regression/ba-issues && ./build_wamr.sh"
```

---

## 11. Best Practices

### Run Checks Frequently

Don't wait until commit time to run checks:

- **During development**: Run format check and unit tests frequently
- **Before breaks**: Run full test suite before stepping away
- **Before committing**: Run all relevant checks

This catches issues early when they're fresh in your mind.

### Fix Issues Immediately

Don't accumulate formatting issues or failing tests:

- Fix format issues as they appear
- Fix failing tests before moving to next task
- Don't commit known failures with "TODO: fix later"

### Use Pre-Commit Hooks (Optional)

Set up Git pre-commit hooks to run checks automatically:

**Create `.git/hooks/pre-commit`:**

```bash
#!/bin/bash

echo "Running pre-commit checks..."

# Format check
echo "Checking code formatting..."
if ! ./scripts/in-container.sh "git diff --cached --name-only --diff-filter=ACM | grep '\.\(c\|h\)$' | xargs -r clang-format-14 --dry-run --Werror"; then
    echo "❌ Format check failed. Run: ./scripts/in-container.sh \"git diff --cached --name-only | grep '\.\(c\|h\)$' | xargs -r clang-format-14 -i\""
    exit 1
fi

# Unit tests (if build exists)
if [ -d "build" ]; then
    echo "Running unit tests..."
    if ! ./scripts/in-container.sh "cd build && ctest --output-on-failure --timeout 60"; then
        echo "❌ Unit tests failed"
        exit 1
    fi
fi

echo "✅ All pre-commit checks passed"
exit 0
```

Make it executable:

```bash
chmod +x .git/hooks/pre-commit
```

**Note**: Pre-commit hooks run on your local machine. They're optional but help catch issues even earlier.

### Review Changes Before Committing

Always review your changes:

```bash
# See what you're about to commit
git diff --cached

# Check for debug prints, TODOs, commented code
git diff --cached | grep -E '(printf|TODO|FIXME|XXX|/\*.*\*/)'
```

### Test Both Success and Failure Cases

When writing tests:

- Test the happy path (feature works correctly)
- Test error paths (feature fails gracefully)
- Test edge cases (boundary conditions, empty inputs, etc.)

---

## 12. For AI Agents

AI agents working on WAMR must follow these requirements:

### Always Run All Checks Before Claiming Completion

Never report work as "done" or "complete" without running and verifying:

1. Format check passes
2. Build succeeds without warnings
3. Unit tests pass (if applicable)
4. Spec tests pass (for VMcore changes)
5. Regression tests pass (for bug fixes)

### Parse Test Output to Verify Success

Don't just run tests - verify they passed:

```bash
# ❌ Wrong: Not checking result
./scripts/in-container.sh "cd build && ctest"
echo "Tests passed"  # Assumption without verification

# ✅ Correct: Check exit code
if ./scripts/in-container.sh "cd build && ctest --output-on-failure"; then
    echo "Tests passed"
else
    echo "Tests failed"
    exit 1
fi
```

### Don't Commit if Any Check Fails

If format check, tests, or builds fail:

1. Investigate the failure
2. Fix the issue
3. Re-run the check
4. Only then commit

Never commit with the intention to "fix later in another commit."

### Include Check Results in Work Summary

When reporting work completion, include verification:

```
✅ Code changes completed
✅ Format check passed (clang-format-14)
✅ Build succeeded with no warnings
✅ Unit tests passed (5/5)
✅ Spec tests passed (1247/1250, 3 expected failures)
✅ Regression test added and passed
✅ All checks complete - ready for review
```

### Use Container Wrapper for All Commands

Never run commands on host:

```bash
# ❌ Wrong
cd build && ctest

# ✅ Correct
./scripts/in-container.sh "cd build && ctest"
```

### Understand Test Failures

When tests fail:

1. Read the error message completely
2. Examine the test output
3. Understand what the test is checking
4. Determine if failure is legitimate or test needs updating
5. Fix the root cause, not just the symptom

### Add Regression Tests for Bug Fixes

Every bug fix should include a regression test:

1. Create test case that reproduces the bug
2. Verify test fails before your fix
3. Verify test passes after your fix
4. Commit test with fix

---

## 13. Reference

### Related Documentation

- **[doc/dev-in-container.md](dev-in-container.md)** - Container environment and wrapper script
- **[doc/building.md](building.md)** - Build instructions and configuration
- **[doc/testing.md](testing.md)** - Complete testing guide
- **[doc/code-quality.md](code-quality.md)** - Code quality standards and conventions
- **[doc/debugging.md](debugging.md)** - Debugging with GDB and Valgrind
- **[tests/regression/README.md](../tests/regression/README.md)** - Regression test details
- **[CONTRIBUTING.md](../CONTRIBUTING.md)** - Contribution guidelines
- **[AGENTS.md](../AGENTS.md)** - AI agent development guide

### CI Configuration

The CI pipeline configuration is in:
- `.github/workflows/` - GitHub Actions workflows
- `.devcontainer/` - Container configuration

### Format Configuration

WAMR's code style is defined in:
- `.clang-format` - clang-format configuration (based on Mozilla style with K&R modifications)

Key style points:
- 4 spaces for indentation (never tabs)
- 80 character line limit
- K&R brace style
- Space around pointer qualifiers

---

## Quick Command Reference

**Format checks:**
```bash
# Check staged files
./scripts/in-container.sh "git diff --cached --name-only | grep '\.\(c\|h\)$' | xargs -r clang-format-14 --dry-run --Werror"

# Auto-fix
./scripts/in-container.sh "git diff --cached --name-only | grep '\.\(c\|h\)$' | xargs -r clang-format-14 -i"
```

**Build and test:**
```bash
# Build with warnings as errors
./scripts/in-container.sh "cmake -B build -DCMAKE_C_FLAGS='-Wall -Werror' && cmake --build build -j$(nproc)"

# Run unit tests
./scripts/in-container.sh "cd build && ctest --output-on-failure"
```

**Spec tests:**
```bash
# Fast interpreter
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp"
```

**Regression tests:**
```bash
# Build test infrastructure
./scripts/in-container.sh "cd tests/regression/ba-issues && ./build_wamr.sh"

# Run all tests
./scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py"

# Run specific test
./scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py -i 3022"
```

**Container status:**
```bash
# Check container
./scripts/in-container.sh --status

# Debug detection
./scripts/in-container.sh --verbose --status
```

---

**Documentation Version**: 1.0.0  
**Last Updated**: 2026-04-03  
**Maintained By**: WAMR Development Team
