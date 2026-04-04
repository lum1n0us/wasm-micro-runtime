# Regression Tests

This directory contains regression test suites for WAMR. Regression tests verify that previously fixed bugs do not reoccur in future versions of the software.

---

## What are Regression Tests?

**Purpose**: Regression tests prevent fixed bugs from reappearing when code changes are made. Each test represents a specific bug that was fixed and validates that the fix continues to work.

**When to Add Regression Tests**: Add a regression test whenever you:
- Fix a bug reported in an issue tracker (GitHub, Bytecode Alliance)
- Discover and fix a security vulnerability
- Resolve a crash, hang, or incorrect behavior
- Fix a bug found by fuzzing or other automated testing

**Best Practice**: Every bug fix should include a regression test to prevent the bug from returning unnoticed.

---

## Directory Structure

```
tests/regression/
├── README.md           (this file - overview of regression testing)
└── ba-issues/          (Bytecode Alliance issue regression tests)
    ├── README.md       (detailed guide for ba-issues tests)
    ├── build_wamr.sh   (builds wamrc and multiple iwasm variants)
    ├── helper.sh       (creates test directories)
    ├── run.py          (test runner - executes all regression tests)
    ├── running_config.json (test configuration)
    ├── issues/         (active test cases, organized as issue-XXXX/)
    └── issues-deprecated/ (deprecated test cases)
```

---

## Quick Start

All regression test commands should run inside the devcontainer using the `scripts/in-container.sh` wrapper for environment consistency.

### Running All Regression Tests

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

### Running Specific Regression Tests

Test specific issues by their ID numbers:

```bash
# Test a single issue
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py --issues 2833"

# Test multiple specific issues
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py -i 2833,2834,2835"
```

### Checking Test Results

When tests fail, detailed logs are written to `issues_tests.log`:

```bash
# View the failure log
scripts/in-container.sh "cd tests/regression/ba-issues && cat issues_tests.log"
```

The log contains:
- Failing issue ID
- Exact command that was run
- Expected vs actual exit code
- Expected vs actual stdout content

---

## Types of Regression Tests

### 1. BA Issues (`ba-issues/`)

Tests for bugs reported in the Bytecode Alliance issue tracker. Each test case:
- Lives in `issues/issue-XXXX/` where XXXX is the issue number
- Has a configuration entry in `running_config.json`
- Tests specific WAMR execution modes (fast-interp, jit, aot)

**See [ba-issues/README.md](ba-issues/README.md) for detailed instructions.**

### 2. Future Test Suites

Additional regression test directories may be added for:
- Security vulnerabilities (CVEs)
- Performance regressions
- Spec compliance issues
- Platform-specific bugs

---

## Adding New Regression Tests

When you fix a bug, follow these steps to add a regression test:

### Step 1: Decide Where the Test Goes

- **Bytecode Alliance issue**: Add to `ba-issues/`
- **Security vulnerability**: Consider creating a new `security/` directory
- **Other sources**: Add to `ba-issues/` or create appropriate directory

### Step 2: Create Test Case

For BA issues, use the helper script:

```bash
# Inside the devcontainer
cd tests/regression/ba-issues

# Create directory for issue #3022
./helper.sh 3022

# If you have a zip file with test artifacts
# Place the zip in issues/issue-3022/test.zip, then:
./helper.sh -x 3022
```

### Step 3: Add Test Files

Place your test artifacts in the issue directory:

```
issues/issue-3022/
├── test.wasm           (the WASM file that reproduces the bug)
└── other-files...      (any other required files)
```

### Step 4: Configure Test Execution

Add a configuration entry to `running_config.json` (see [ba-issues/README.md](ba-issues/README.md) for full details):

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
        "stdout content": "expected output",
        "description": "bug should not crash/should return correct value"
    }
}
```

### Step 5: Verify the Test

Run your new test to verify it works:

```bash
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py --issues 3022"
```

### Step 6: Commit the Test

Commit the new regression test with your bug fix:

```bash
git add tests/regression/ba-issues/issues/issue-3022/
git add tests/regression/ba-issues/running_config.json
git commit -m "test: add regression test for issue #3022"
```

---

## Best Practices

### 1. Keep Tests Minimal and Focused

- Use the smallest WASM file that reproduces the bug
- Minimize dependencies and complexity
- Each test should validate exactly one bug fix

### 2. Include Issue/PR Numbers

- Name directories with issue numbers: `issue-3022`
- Reference the issue in commit messages: `test: add regression test for issue #3022`
- Add issue URL in JSON description field

### 3. Document Expected Behavior

In the JSON configuration, clearly document:
- What the bug was (in the `description` field)
- What the expected behavior is
- Why the specific exit code and output are expected

Example:
```json
"expected return": {
    "ret code": 1,
    "stdout content": "Exception: out of bounds memory access",
    "description": "Issue #2857: should trap with bounds error, not type mismatch"
}
```

### 4. Test Multiple Execution Modes

If the bug affects multiple execution modes (fast-interp, jit, aot), create configurations for each:

```json
{
    "ids": [3022],
    "runtime": "iwasm-default",
    "mode": "fast-interp",
    ...
},
{
    "ids": [3022],
    "runtime": "iwasm-llvm-jit",
    "mode": "jit",
    ...
}
```

### 5. Use Container Wrapper for Consistency

Always run tests inside the devcontainer:

```bash
# ✅ Correct - runs in devcontainer
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py"

# ❌ Wrong - runs on host, may have different environment
cd tests/regression/ba-issues && ./run.py
```

---

## Troubleshooting

### Tests Not Found

**Symptom**:
```
Left issues in folder: #3022
```

**Cause**: Test case directory exists but no JSON configuration entry.

**Fix**: Add configuration entry to `running_config.json` with `"ids": [3022]`.

---

### Test Case in JSON But Not Found

**Symptom**:
```
Cases in JSON but not found in folder: #3022
```

**Cause**: JSON configuration references issue #3022 but `issues/issue-3022/` doesn't exist.

**Fix**: 
- Create the directory: `./helper.sh 3022`
- Or remove the JSON entry if the test is no longer needed

---

### Build Failures

**Symptom**: `build_wamr.sh` fails with CMake or compilation errors.

**Cause**: Missing dependencies or incorrect build environment.

**Fix**:
```bash
# Ensure you're in the devcontainer
scripts/in-container.sh --status

# Clean and rebuild
scripts/in-container.sh "cd tests/regression/ba-issues && rm -rf build && ./build_wamr.sh"
```

---

### Test Fails After Code Changes

**Symptom**: Previously passing test now fails.

**Analysis**:
1. Check if the failure is legitimate (bug reintroduced)
2. Check if the expected behavior changed (update test)
3. Check if the test configuration is incorrect (fix config)

**Action**:
```bash
# View detailed failure information
scripts/in-container.sh "cd tests/regression/ba-issues && cat issues_tests.log"

# Run test with verbose output
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py -i 3022"
```

---

## Integration with CI/CD

Regression tests should run automatically in continuous integration:

```bash
# Example CI script
#!/bin/bash
set -e

# Build WAMR
scripts/in-container.sh "cd tests/regression/ba-issues && ./build_wamr.sh"

# Run all regression tests
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py"

# Check exit code
if [ $? -eq 0 ]; then
    echo "✅ All regression tests passed"
    exit 0
else
    echo "❌ Regression tests failed"
    cat tests/regression/ba-issues/issues_tests.log
    exit 1
fi
```

---

## Related Documentation

- **[ba-issues/README.md](ba-issues/README.md)** - Detailed guide for BA issue regression tests
- **[doc/testing.md](../../doc/testing.md)** - Complete testing guide for WAMR
- **[doc/building.md](../../doc/building.md)** - Building iwasm and wamrc
- **[doc/dev-in-container.md](../../doc/dev-in-container.md)** - Devcontainer setup and usage
- **[AGENTS.md](../../AGENTS.md)** - Guide for AI agents working with WAMR

---

## For AI Agents

**CRITICAL**: When working with regression tests:

1. **Always use the container wrapper**: All commands must use `scripts/in-container.sh "<command>"`
2. **Read existing tests first**: Before adding a new test, check if similar tests exist
3. **Follow the existing patterns**: Match the structure and style of existing tests
4. **Verify tests pass**: Always run the test after creating it to ensure it works
5. **Commit test with fix**: Include the regression test in the same commit or PR as the bug fix

**Common workflow for AI agents**:

```bash
# 1. Create test directory
scripts/in-container.sh "cd tests/regression/ba-issues && ./helper.sh 3022"

# 2. Add test files (use Write tool to create test.wasm or copy from issue)

# 3. Add JSON configuration (edit running_config.json)

# 4. Build iwasm variants
scripts/in-container.sh "cd tests/regression/ba-issues && ./build_wamr.sh"

# 5. Run the new test
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py -i 3022"

# 6. Verify it passes, then commit
```

**Error handling in scripts**:

```bash
# Check if test passes
if scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py -i 3022"; then
    echo "Test passed"
else
    echo "Test failed - check issues_tests.log"
    scripts/in-container.sh "cd tests/regression/ba-issues && cat issues_tests.log"
    exit 1
fi
```
