# BA Issues Regression Tests

This directory contains regression tests for bugs reported in the [Bytecode Alliance issue tracker](https://github.com/bytecodealliance/wasm-micro-runtime/issues). Each test case validates that a specific bug fix continues to work correctly.

---

## Quick Reference

**Adding a new test case requires these steps**:

1. [Create test directory](#1-create-test-directory-helper-shell-script) - Use `helper.sh` to create `issues/issue-XXXX/`
2. [Add test files](#2-add-test-files) - Place WASM files and test artifacts in the issue directory
3. [Build WAMR](#3-build-wamr-wamrc-and-iwasm-build-script) - Build iwasm variants and wamrc (may need to add new build configs)
4. [Configure test execution](#4-add-test-configuration-running_configjson) - Add entry to `running_config.json`
5. [Run and verify test](#5-run-tests-and-verify-results) - Execute tests and check results

**All commands should run inside the devcontainer**:
```bash
scripts/in-container.sh "<command>"
```

---

## Prerequisites

Before working with regression tests, ensure:

1. **Container is running**: Check with `scripts/in-container.sh --status`
2. **Build dependencies are available**: The devcontainer has all required tools
3. **You're in the correct directory**: `tests/regression/ba-issues/`

---

## 1. Create Test Directory (helper shell script)

The `helper.sh` script creates test case directories with the naming convention `issues/issue-XXXX` where XXXX is the issue number. It can also automatically extract zip files containing test artifacts.

### Usage

**Create a single test directory**:
```bash
# Inside the devcontainer
scripts/in-container.sh "cd tests/regression/ba-issues && ./helper.sh 2999"

# This creates: issues/issue-2999/
```

**Create multiple test directories**:
```bash
# Create issue-2944 through issue-2966 (inclusive range)
scripts/in-container.sh "cd tests/regression/ba-issues && ./helper.sh 2944 2966"

# This creates:
#   issues/issue-2944/
#   issues/issue-2945/
#   ...
#   issues/issue-2966/
```

**Extract zip files automatically**:
```bash
# Create directories AND extract any .zip files found in them
scripts/in-container.sh "cd tests/regression/ba-issues && ./helper.sh -x 2944 2966"

# This:
# 1. Creates issues/issue-XXXX/ for each number in range
# 2. Looks for *.zip files in each directory
# 3. Extracts zip contents into the directory
# 4. Removes the zip file after extraction
```

### Example Workflow

```bash
# Step 1: Create directory for issue #3022
scripts/in-container.sh "cd tests/regression/ba-issues && ./helper.sh 3022"

# Step 2: Directory now exists at issues/issue-3022/
# You can now add test files to this directory

# Alternative: If you have a zip file from the issue
# 1. Place the zip file in the directory: issues/issue-3022/test.zip
# 2. Run with -x flag to extract:
scripts/in-container.sh "cd tests/regression/ba-issues && ./helper.sh -x 3022"
```

---

## 2. Add Test Files

After creating the test directory, add your test artifacts (WASM files, input files, etc.) to the issue directory.

### Directory Structure

```
issues/issue-3022/
├── test.wasm           (the WASM file that reproduces the bug)
├── input.txt           (optional: input data if needed)
└── other-files...      (any other required files)
```

### How to Add Files

**Option 1: Copy from host to container**:
```bash
# Copy WASM file to the issue directory
docker cp test.wasm <container-name>:/workspace/tests/regression/ba-issues/issues/issue-3022/
```

**Option 2: Download directly in container**:
```bash
# Download from GitHub issue
scripts/in-container.sh "cd tests/regression/ba-issues/issues/issue-3022 && curl -L -O https://github.com/.../test.wasm"
```

**Option 3: Extract from zip** (recommended):
```bash
# Place zip file in directory first, then:
scripts/in-container.sh "cd tests/regression/ba-issues && ./helper.sh -x 3022"
```

---

## 3. Build WAMR (wamrc and iwasm build script)

The `build_wamr.sh` script builds multiple variants of the iwasm runtime with different feature combinations, plus the wamrc AOT compiler. This step can take 5-15 minutes depending on your system.

### Build All Variants

```bash
# Build wamrc and all iwasm variants
scripts/in-container.sh "cd tests/regression/ba-issues && ./build_wamr.sh"
```

**This builds**:
- `build/build-wamrc/wamrc` - AOT compiler
- `build/build-iwasm-default/iwasm` - Fast interpreter + AOT support
- `build/build-iwasm-default-gc-enabled/iwasm` - With GC support
- `build/build-iwasm-llvm-jit/iwasm` - LLVM JIT mode
- `build/build-iwasm-fast-jit/iwasm` - Fast JIT mode
- `build/build-iwasm-default-wasi-disabled/iwasm` - No WASI support
- `build/build-iwasm-llvm-jit-wasi-disabled/iwasm` - JIT without WASI
- `build/build-iwasm-fast-jit-wasi-disabled/iwasm` - Fast JIT without WASI
- `build/build-iwasm-default-branch-hints-enabled/iwasm` - With branch hints

**Note**: All builds include Address Sanitizer (ASan) for better error detection.

### Understanding Runtime Names

The "runtime name" is used in `running_config.json` to specify which iwasm variant to use:

| Runtime Name | Description | When to Use |
|--------------|-------------|-------------|
| `iwasm-default` | Fast interpreter + AOT | Most common tests |
| `iwasm-default-wasi-disabled` | No WASI support | Tests that don't need WASI |
| `iwasm-llvm-jit` | LLVM JIT compiler | JIT-specific bugs |
| `iwasm-fast-jit` | Fast JIT compiler | Fast JIT-specific bugs |
| `iwasm-default-gc-enabled` | Garbage collection support | GC proposal tests |
| `iwasm-default-branch-hints-enabled` | Branch hint support | Branch hint proposal tests |

### Adding a New Build Configuration

If your test case needs a different CMake configuration (e.g., different feature flags), you must add a new build command to `build_wamr.sh`.

**Example**: Add a multi-tier JIT build with WASI disabled:

1. Open `build_wamr.sh` in an editor
2. Add a new `build_iwasm` call at the end:

```bash
# Format: build_iwasm "CMake flags" "runtime-name"
build_iwasm "-DWAMR_BUILD_LIBC_WASI=0 -DWAMR_BUILD_LIBC_BUILTIN=1 -DWAMR_BUILD_REF_TYPES=1 -DWAMR_BUILD_BULK_MEMORY=1 -DWAMR_BUILD_JIT=1 -DWAMR_BUILD_FAST_JIT=1" "multi-tier-wasi-disabled"
```

3. This creates: `build/build-iwasm-multi-tier-wasi-disabled/iwasm`
4. Use it in JSON config with: `"runtime": "iwasm-multi-tier-wasi-disabled"`

**Common CMake flags**:
- `-DWAMR_BUILD_LIBC_WASI=1/0` - Enable/disable WASI support
- `-DWAMR_BUILD_AOT=1` - Enable AOT support
- `-DWAMR_BUILD_FAST_INTERP=1` - Enable fast interpreter
- `-DWAMR_BUILD_JIT=1` - Enable LLVM JIT
- `-DWAMR_BUILD_FAST_JIT=1` - Enable fast JIT
- `-DWAMR_BUILD_REF_TYPES=1` - Enable reference types
- `-DWAMR_BUILD_GC=1` - Enable garbage collection
- `-DWAMR_BUILD_SIMD=1` - Enable SIMD support
- `-DWAMR_BUILD_BULK_MEMORY=1` - Enable bulk memory operations

### Rebuilding After Changes

If you modify `build_wamr.sh` or need to rebuild:

```bash
# Clean previous build
scripts/in-container.sh "cd tests/regression/ba-issues && rm -rf build"

# Rebuild all variants
scripts/in-container.sh "cd tests/regression/ba-issues && ./build_wamr.sh"
```

---

## 4. Add Test Configuration (running_config.json)

The `running_config.json` file contains an array of test configurations under the `"test cases"` key. Each configuration specifies how to run a test case and what result to expect.

### Configuration Structure

Every test configuration is a JSON object with these fields:

```json
{
    "deprecated": false,              // true = skip this test
    "ids": [3022],                    // issue number(s) this config applies to
    "runtime": "iwasm-default",       // which iwasm build to use
    "file": "test.wasm",              // WASM file to run (supports wildcards)
    "mode": "fast-interp",            // execution mode
    "options": "--heap-size=0 -f main", // iwasm command-line options
    "argument": "",                   // arguments passed to WASM function
    "expected return": {
        "ret code": 0,                // expected exit code
        "stdout content": "Success",  // expected stdout (substring match)
        "description": "Issue #3022: should not crash" // human-readable description
    }
}
```

### Configuration Type 1: iwasm Only (Most Common)

This is the most common configuration - run a WASM file with iwasm and check the output.

**Example - Testing a crash fix**:
```json
{
    "deprecated": false,
    "ids": [2955],
    "runtime": "iwasm-default-wasi-disabled",
    "file": "iwasm_fast_interp_unexpected_value.wasm",
    "mode": "fast-interp",
    "options": "--heap-size=0 -f to_test",
    "argument": "",
    "expected return": {
        "ret code": 0,
        "stdout content": "0x44e5d17eb93a0ce:i64",
        "description": "Issue #2955: fast-interp should return correct value, not crash"
    }
}
```

**Field details**:
- `runtime`: Choose from `iwasm-default`, `iwasm-default-wasi-disabled`, `iwasm-llvm-jit`, `iwasm-fast-jit`, etc. (see [Build section](#3-build-wamr-wamrc-and-iwasm-build-script))
- `file`: Filename in `issues/issue-XXXX/` directory. Can use `*.wasm` wildcard to match all WASM files.
- `mode`: One of:
  - `"fast-interp"` - Fast interpreter
  - `"classic-interp"` - Classic interpreter (slower but simpler)
  - `"jit"` - LLVM JIT (requires `iwasm-llvm-jit` runtime)
  - `"fast-jit"` - Fast JIT (requires `iwasm-fast-jit` runtime)
  - `"aot"` - AOT compiled (requires AOT file from wamrc)
- `options`: Command-line flags for iwasm:
  - `--heap-size=0` - Use default heap size
  - `-f function_name` - Call specific exported function
  - `--env="VAR=value"` - Set environment variables
  - `--dir=.` - Map directory for WASI
- `argument`: Arguments passed to the WASM function (space-separated)
- `ret code`: Expected exit code (0 = success, 1 = WASM trap, 255 = load error)
- `stdout content`: Expected output (test passes if this string appears anywhere in stdout)

### Configuration Type 2: Multiple Issues Sharing One Config

If multiple issues have the same test setup (common for fuzzer-found bugs), you can combine them:

```json
{
    "deprecated": false,
    "ids": [2966, 2964, 2963, 2962],
    "runtime": "iwasm-multi-tier-wasi-disabled",
    "file": "*.wasm",
    "mode": "fast-jit",
    "options": "--heap-size=0 -f to_test",
    "argument": "",
    "expected return": {
        "ret code": 0,
        "stdout content": "0x0:i32",
        "description": "Issues #2962-#2966: fuzzer crashes in fast-jit, all should return 0x0:i32"
    }
}
```

**Important**: The `"file": "*.wasm"` wildcard matches ALL `.wasm` files in each issue directory. This works when:
- All issues have files with different names, OR
- All issues have files with the same name and expected behavior

### Configuration Type 3: wamrc Only (AOT Compilation Test)

Test that wamrc can compile a WASM file to AOT without errors:

```json
{
    "deprecated": false,
    "ids": [3100],
    "compile_options": {
        "compiler": "wamrc",
        "only compile": true,
        "in file": "test.wasm",
        "out file": "test.aot",
        "options": "--target=x86_64 --bounds-checks=1",
        "expected return": {
            "ret code": 0,
            "stdout content": "",
            "description": "Issue #3100: wamrc should compile without errors"
        }
    }
}
```

**Field details**:
- `"only compile": true` - Only run wamrc, don't execute the result
- `"in file"`: Input WASM file (supports wildcards)
- `"out file"`: Output AOT file name
- `"options"`: wamrc flags:
  - `--target=x86_64` - Target architecture (x86_64, aarch64, etc.)
  - `--bounds-checks=1` - Enable bounds checking
  - `--size-level=3` - Optimization for size
  - `--opt-level=3` - Optimization level

### Configuration Type 4: wamrc + iwasm (Full AOT Test)

Compile WASM to AOT with wamrc, then execute it with iwasm:

```json
{
    "deprecated": false,
    "ids": [3101],
    "compile_options": {
        "compiler": "wamrc",
        "only compile": false,
        "in file": "test.wasm",
        "out file": "test.aot",
        "options": "--target=x86_64",
        "expected return": {
            "ret code": 0,
            "stdout content": "",
            "description": "AOT compilation should succeed"
        }
    },
    "runtime": "iwasm-default",
    "file": "test.aot",
    "mode": "aot",
    "options": "--heap-size=0 -f main",
    "argument": "",
    "expected return": {
        "ret code": 0,
        "stdout content": "Success",
        "description": "Issue #3101: AOT execution should return Success"
    }
}
```

**Important**: 
- `"only compile": false` - Compile AND execute
- The `runtime`, `file`, `mode`, etc. fields specify how to run the AOT file
- `"file": "test.aot"` - Must match the `"out file"` from compile_options

### Adding Your Configuration

1. Open `running_config.json` in an editor
2. Find the `"test cases": [` array
3. Add your new configuration object to the array (include trailing comma if not last)
4. Save the file
5. Validate JSON syntax: `scripts/in-container.sh "cd tests/regression/ba-issues && python3 -m json.tool running_config.json > /dev/null"`

**Example addition**:
```json
{
    "test cases": [
        {
            "deprecated": false,
            "ids": [2955],
            ...existing configs...
        },
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
                "stdout content": "expected output",
                "description": "Issue #3022: should not crash"
            }
        }
    ]
}
```

---

### Deprecating Test Cases

Test cases may become deprecated when:
- WebAssembly spec changes invalidate the test
- The WASM file is no longer valid according to current spec
- The issue was closed as "not a bug" or "wontfix"

**Process to deprecate a test**:

1. **Verify the test should be deprecated**:
   ```bash
   # Use wasm-validate to check if WASM is still valid
   scripts/in-container.sh "cd tests/regression/ba-issues/issues/issue-XXXX && wasm-validate test.wasm"
   ```

2. **Move test files to deprecated directory**:
   ```bash
   scripts/in-container.sh "cd tests/regression/ba-issues && mv issues/issue-47 issues-deprecated/"
   ```

3. **Mark configuration as deprecated**:
   ```json
   {
       "deprecated": true,
       "ids": [47],
       "runtime": "iwasm-default",
       "mode": "classic-interp",
       "file": "PoC.wasm",
       "argument": "",
       "expected return": {
           "ret code": 0,
           "stdout content": "",
           "description": "Deprecated: no longer valid per WebAssembly spec 2.0"
       }
   }
   ```

4. **Update description**: Explain WHY the test is deprecated in the `"description"` field.

**Example - Multiple deprecated fuzzer tests**:
```json
{
    "deprecated": true,
    "ids": [47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 
            61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 
            75, 76, 77, 78, 79, 80, 81, 82, 83, 84],
    "runtime": "iwasm-default",
    "mode": "classic-interp",
    "file": "PoC.wasm",
    "argument": "",
    "expected return": {
        "ret code": 0,
        "stdout content": "",
        "description": "Deprecated: WASM files no longer pass wasm-validate (invalid per spec)"
    }
}
```

**Note**: Deprecated tests are skipped during test execution but their configurations remain in `running_config.json` for historical reference.

---

## 5. Run Tests and Verify Results

The `run.py` script executes regression tests based on the configurations in `running_config.json`. It compares actual results against expected results and reports pass/fail status.

### Run All Tests

Execute all non-deprecated regression tests:

```bash
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

**Output explanation**:
- `Total`: Number of test configurations executed (excludes deprecated)
- `Passed`: Number of tests where actual result matched expected result
- `Failed`: Number of tests where results didn't match
- `Left issues in folder`: Issue directories that exist but have no JSON config
- `Cases in JSON but not found in folder`: JSON configs that reference missing issue directories

### Run Specific Tests

Test specific issues by issue number:

```bash
# Test a single issue
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py --issues 2833"

# Test multiple specific issues (comma-separated, no spaces)
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py -i 2833,2834,2835"

# Alternative: use spaces and quotes
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py --issues 2833 2834 2835"
```

### Understanding Test Results

#### Scenario 1: Missing JSON Configuration

**Output**:
```
==== Test results ====
   Total: 21
  Passed: 21
  Failed: 0
  Left issues in folder: #3022
  Cases in JSON but not found in folder: no more
```

**Meaning**: You created `issues/issue-3022/` but forgot to add a configuration entry in `running_config.json`.

**Fix**: Add a JSON configuration for issue #3022 (see [Configuration section](#4-add-test-configuration-running_configjson)).

---

#### Scenario 2: Missing Issue Directory

**Output**:
```
==== Test results ====
   Total: 21
  Passed: 21
  Failed: 0
  Left issues in folder: #2855
  Cases in JSON but not found in folder: #12345
```

**Meaning**: Your `running_config.json` references issue #12345 but `issues/issue-12345/` doesn't exist.

**Fix**: 
- Create the directory: `scripts/in-container.sh "cd tests/regression/ba-issues && ./helper.sh 12345"`
- OR remove the JSON config if the test is no longer needed

---

#### Scenario 3: Test Failure

**Output**:
```
==== Test results ====
   Total: 22
  Passed: 21
  Failed: 1
  Left issues in folder: no more
  Cases in JSON but not found in folder: no more
```

**Meaning**: One test failed - actual result didn't match expected result.

**Next step**: Check the detailed failure log:

```bash
scripts/in-container.sh "cd tests/regression/ba-issues && cat issues_tests.log"
```

**Example failure log**:
```
=======================================================
Failing issue id: 2945.
run with command_lists: ['./build/build-iwasm-default-wasi-disabled/iwasm', '--heap-size=0', '-f', 'to_test', '/workspace/tests/regression/ba-issues/issues/issue-2945/iwasm_fast_interp_moob_unhandled.wasm']
exit code (actual, expected) : (1, 0)
stdout (actual, expected) : ('Exception: out of bounds memory access', 'Exception: out of bounds memory access')
=======================================================
```

**Log interpretation**:
- `Failing issue id`: Which test failed
- `run with command_lists`: Exact command that was executed (you can copy/paste to reproduce)
- `exit code (actual, expected)`: Exit code comparison (in this example: got 1, expected 0)
- `stdout (actual, expected)`: Output comparison (both match but exit code differs)

**Debugging a failure**:

1. **Reproduce manually**:
   ```bash
   # Copy the command from the log and run it manually
   scripts/in-container.sh "./build/build-iwasm-default-wasi-disabled/iwasm --heap-size=0 -f to_test /workspace/tests/regression/ba-issues/issues/issue-2945/test.wasm"
   ```

2. **Check if the failure is legitimate**:
   - Did recent code changes reintroduce the bug?
   - Was the expected result incorrect?
   - Did the test configuration change?

3. **Fix the issue**:
   - If bug reintroduced: Fix the code
   - If expected result wrong: Update `running_config.json`
   - If test is outdated: Deprecate it (see [Deprecating section](#deprecating-test-cases))

### Checking Exit Codes

The `run.py` script returns exit code 0 if all tests pass, non-zero if any test fails. This enables automated testing:

```bash
# Example CI/CD usage
if scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py"; then
    echo "✅ All regression tests passed"
else
    echo "❌ Regression tests failed - check issues_tests.log"
    scripts/in-container.sh "cd tests/regression/ba-issues && cat issues_tests.log"
    exit 1
fi
```

---

## Complete Workflow Example

Here's a complete example of adding a new regression test for issue #3022:

```bash
# Step 1: Create test directory
scripts/in-container.sh "cd tests/regression/ba-issues && ./helper.sh 3022"

# Step 2: Add test file (assuming you have test.wasm locally)
docker cp test.wasm wamr-dev:/workspace/tests/regression/ba-issues/issues/issue-3022/

# Step 3: Build iwasm variants (if not already built)
scripts/in-container.sh "cd tests/regression/ba-issues && ./build_wamr.sh"

# Step 4: Edit running_config.json to add configuration
# (Use your editor to add the JSON entry)

# Step 5: Validate JSON syntax
scripts/in-container.sh "cd tests/regression/ba-issues && python3 -m json.tool running_config.json > /dev/null && echo 'JSON is valid'"

# Step 6: Run the new test
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py -i 3022"

# Step 7: If test passes, run all tests to ensure no regressions
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py"

# Step 8: Commit the new test
git add tests/regression/ba-issues/issues/issue-3022/
git add tests/regression/ba-issues/running_config.json
git commit -m "test: add regression test for issue #3022"
```

---

## Troubleshooting

### Build Errors

**Problem**: `build_wamr.sh` fails with CMake or compilation errors.

**Solution**:
```bash
# Ensure you're in the container
scripts/in-container.sh --status

# Clean and rebuild
scripts/in-container.sh "cd tests/regression/ba-issues && rm -rf build && ./build_wamr.sh"

# Check for missing dependencies (should be in container)
scripts/in-container.sh "which cmake && which gcc && which python3"
```

### File Not Found Errors

**Problem**: `run.py` reports "file not found" errors.

**Solution**:
```bash
# Check if issue directory exists
scripts/in-container.sh "ls -la tests/regression/ba-issues/issues/issue-3022/"

# Check if WASM file is in the directory
scripts/in-container.sh "ls tests/regression/ba-issues/issues/issue-3022/*.wasm"

# Verify the file path in running_config.json matches the actual filename
```

### JSON Syntax Errors

**Problem**: `run.py` crashes with JSON parsing errors.

**Solution**:
```bash
# Validate JSON syntax
scripts/in-container.sh "cd tests/regression/ba-issues && python3 -m json.tool running_config.json"

# Common issues:
# - Missing commas between array elements
# - Trailing comma after last element
# - Unescaped quotes in strings
# - Missing closing braces/brackets
```

### Permission Denied Errors

**Problem**: Cannot execute scripts or access files.

**Solution**:
```bash
# Make scripts executable
scripts/in-container.sh "cd tests/regression/ba-issues && chmod +x helper.sh build_wamr.sh run.py"

# Check file ownership (should be accessible in container)
scripts/in-container.sh "ls -la tests/regression/ba-issues/"
```

### Wildcard Not Matching Files

**Problem**: Using `"file": "*.wasm"` but test doesn't run.

**Solution**:
```bash
# Check what files exist
scripts/in-container.sh "ls tests/regression/ba-issues/issues/issue-3022/"

# Verify filename extensions are lowercase (.wasm not .WASM)

# If no match, use explicit filename:
# Change "file": "*.wasm" to "file": "exact-name.wasm"
```

### Tests Pass Locally But Fail in CI

**Problem**: Tests pass when run manually but fail in automated CI.

**Common causes**:
- Not using container wrapper in CI scripts
- Different build configurations
- Missing build step in CI

**Solution**:
```bash
# Ensure CI uses container wrapper
scripts/in-container.sh "cd tests/regression/ba-issues && ./build_wamr.sh && ./run.py"

# Add to CI configuration file (e.g., .github/workflows/test.yml):
- name: Run regression tests
  run: |
    scripts/in-container.sh "cd tests/regression/ba-issues && ./build_wamr.sh"
    scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py"
```

---

## For AI Agents

When working with BA issue regression tests, follow these guidelines:

### Always Use Container Wrapper

```bash
# ✅ Correct - runs in devcontainer
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py"

# ❌ Wrong - runs on host
cd tests/regression/ba-issues && ./run.py
```

### Check Test Results Programmatically

```bash
# Capture exit code to verify success
if scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py -i 3022"; then
    echo "Test passed"
else
    echo "Test failed"
    scripts/in-container.sh "cd tests/regression/ba-issues && cat issues_tests.log"
    exit 1
fi
```

### Validate JSON Before Committing

```bash
# Always validate JSON syntax after editing
scripts/in-container.sh "cd tests/regression/ba-issues && python3 -m json.tool running_config.json > /dev/null"
if [ $? -eq 0 ]; then
    echo "JSON is valid"
else
    echo "JSON syntax error - fix running_config.json"
    exit 1
fi
```

### Complete Test Addition Pattern

```bash
# 1. Create directory
scripts/in-container.sh "cd tests/regression/ba-issues && ./helper.sh 3022"

# 2. Add test files (use Write tool or docker cp)

# 3. Build if not already built
scripts/in-container.sh "cd tests/regression/ba-issues && [ -d build ] || ./build_wamr.sh"

# 4. Edit running_config.json (use Edit tool)

# 5. Validate JSON
scripts/in-container.sh "cd tests/regression/ba-issues && python3 -m json.tool running_config.json > /dev/null"

# 6. Run new test
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py -i 3022"

# 7. Run all tests
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py"

# 8. Commit if all pass
git add tests/regression/ba-issues/issues/issue-3022/
git add tests/regression/ba-issues/running_config.json
git commit -m "test: add regression test for issue #3022"
```

---

## Related Documentation

- **[tests/regression/README.md](../README.md)** - Overview of all regression test suites
- **[doc/testing.md](../../../doc/testing.md)** - Complete testing guide for WAMR
- **[doc/building.md](../../../doc/building.md)** - Building iwasm and wamrc
- **[doc/dev-in-container.md](../../../doc/dev-in-container.md)** - Devcontainer setup
- **[AGENTS.md](../../../AGENTS.md)** - Guide for AI agents working with WAMR

---

## Summary

To add a regression test for BA issue #XXXX:

1. **Create directory**: `./helper.sh XXXX`
2. **Add test files**: Place WASM files in `issues/issue-XXXX/`
3. **Build iwasm**: `./build_wamr.sh` (if not already built)
4. **Configure test**: Add JSON entry to `running_config.json`
5. **Run test**: `./run.py -i XXXX`
6. **Verify**: Check output and `issues_tests.log` if failed
7. **Commit**: Add test files and config to git

**Remember**: Always use `scripts/in-container.sh "<command>"` to ensure tests run in the correct environment.
