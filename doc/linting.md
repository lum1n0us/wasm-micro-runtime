# Pre-Commit Linting Checklist

This is the execution checklist for pre-commit quality checks. For standards and principles, see [code_quality.md](./code_quality.md).

**Purpose**: Quick checklist of all checks to run before committing code.

**Boundary with code_quality.md**:
- **linting.md** (this doc): Execution checklist (WHAT to run)
- **code_quality.md**: Standards and principles (WHY)


---

## Prerequisites


- Read [dev_in_container.md](dev_in_container.md) for container setup
- All commands must run inside devcontainer on Linux

---

## Quick Checklist

**All Changes:**
- [ ] Code format check passes
- [ ] iwasm build succeeds with no warnings (host platform)
- [ ] wamrc build succeeds with no warnings (if AOT changes)

**Code Changes:**
- [ ] Unit tests pass

**VMcore Changes:**
- [ ] Spec tests pass (affected modes)

**Bug Fixes:**
- [ ] Regression test added and passes

---

## 1. Code Format Check

**Run**: `python3 ci/coding_guidelines_check.py`

**Expected**: No output means all changes meet coding guidelines.

**What it checks**:
- Code formatting (clang-format-14 with `.clang-format` config)
- File naming: must use underscores, not hyphens (e.g., `my_file.c` not `my-file.c`)
- Directory naming: must use hyphens, not underscores (e.g., `my-dir/` not `my_dir/`)

**Auto-fix formatting only**: `git diff --cached --name-only --diff-filter=ACM | grep '\.\(c\|h\|cpp\|hpp\)$' | xargs -r clang-format-14 -i`

---

## 2. Build Check

Build iwasm and wamrc separately on the host platform (Linux/macOS/Windows).

**iwasm build**:
```bash
cd product-mini/platforms/linux  # Use darwin for macOS, windows for Windows
rm -rf build
cmake -S . -B build -DCMAKE_C_FLAGS='-Wall -Werror'
cmake --build build -j$(nproc)
```

**wamrc build** (if AOT changes):
```bash
cd wamr-compiler
rm -rf build
cmake -S . -B build -DCMAKE_C_FLAGS='-Wall -Werror'
cmake --build build -j$(nproc)
```

**Expected**: Both builds succeed with exit code 0, no warnings or errors.

**Details**: See [code_quality.md § Compiler Warnings](./code_quality.md#compiler-warnings) for standards.

---

## 3. Unit Tests

**Run**: `cd build && ctest --output-on-failure`

**Expected**: "100% tests passed, 0 tests failed" with exit code 0.

**Details**: See [testing.md § Unit Tests](./testing.md#unit-tests) for test organization.

---

## 4. Spec Tests (VMcore Changes Only)

**Run**: `cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp`

**Expected**: 99%+ pass rate. Check report at `tests/wamr-test-suites/workspace/report/spec_test_report.txt`.

**When**: Required for VMcore changes (interpreter, JIT, AOT, validation, memory management).

**Details**: See [testing.md § Spec Tests](./testing.md#spec-tests) for execution modes and proposals.

---

## 5. Regression Tests (Bug Fixes Only)

**Build**: `cd tests/regression/ba-issues && ./build_wamr.sh`

**Run**: `cd tests/regression/ba-issues && ./run.py`

**Expected**: "Total: N, Passed: N, Failed: 0"

**When**: Required for bug fixes. Always add new regression test when fixing bugs.

**Details**: See [tests/regression/README.md](../tests/regression/README.md) for adding tests.

---

## 6. Static Analysis (Optional)

**Run**: `cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && clang-tidy -p build --checks='*' <file.c>`

**Expected**: No warnings or errors reported.

**When**: Recommended for memory management changes, complex logic, or security-sensitive code.

**Details**: See [code_quality.md § Static Analysis](./code_quality.md#static-analysis) for interpreting results.

---

## 7. Memory Leak Check (Optional)

**Run**: `cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug && cmake --build build-debug && valgrind --leak-check=full ./build-debug/product-mini/platforms/linux/build/iwasm test.wasm`

**Expected**: "All heap blocks were freed -- no leaks are possible"

**When**: Recommended for memory management changes or when debugging crashes.

**Details**: See [debugging.md § Memory Leak Detection](./debugging.md#memory-leak-detection) for analysis.

---

## Complete Workflow Examples

### Bug Fix in VMcore

1. Format check + auto-fix
2. Build with `-Wall -Werror`
3. Unit tests
4. Spec tests (affected mode)
5. Add regression test
6. Run all regression tests
7. Commit

### New Feature

1. Format check + auto-fix
2. Build with `-Wall -Werror`
3. Unit tests
4. Feature-specific spec tests (if applicable)
5. Update documentation (if API/config changes)
6. Commit

### Simple Code Change

1. Format check + auto-fix
2. Build with `-Wall -Werror`
3. Unit tests
4. Commit

---

## CI/CD Integration

All checks documented here run in CI. To reproduce CI failures locally:

1. Find failing command in CI logs
2. Run same command in devcontainer
3. Fix issue
4. Verify locally before pushing

**Common CI checks:**
- Format check (all changed files)
- Build (multiple configurations)
- Unit tests
- Spec tests
- Regression tests
- Platform-specific builds


---

## Troubleshooting

**Format check fails but no visible diff:**
- Use `clang-format-14 file.c | diff -u file.c -` to see actual diff
- Check for tabs vs spaces with `cat -A file.c`

**Naming check fails:**
- File names must use underscores: `my_file.c` not `my-file.c`
- Directory names must use hyphens: `my-dir/` not `my_dir/`

**Tests pass locally but fail in CI:**
- Verify using devcontainer (not host)
- Check `docker ps | grep devcontainer`

**Tests timeout:**
- Use `ctest --timeout 600` to increase timeout
- Run specific test with `ctest -R test_name --verbose`

**Regression test infrastructure missing:**
- Run `cd tests/regression/ba-issues && ./build_wamr.sh` first

**Details**: See [dev_in_container.md § Troubleshooting](./dev_in_container.md#troubleshooting) for container issues.

---

## Best Practices

- Run checks frequently during development (not just before commit)
- Fix issues immediately (don't accumulate failures)
- Review changes before committing with `git diff --cached`
- Add regression tests for every bug fix
- Verify test output (don't assume success)

---

## For AI Agents

**Never report work complete without:**
1. Format check passes
2. Build succeeds (no warnings)
3. Unit tests pass
4. Spec tests pass (VMcore changes)
5. Regression tests pass (bug fixes)

**Always:**
- Parse test output to verify success
- Use devcontainer (never host)
- Fix failures before committing
- Add regression test with bug fixes
- Include check results in completion summary

---

## Quick Command Reference

**Format:**
```bash
# Check all guidelines (format + naming)
python3 ci/coding_guidelines_check.py

# Auto-fix formatting only
git diff --cached --name-only --diff-filter=ACM | grep '\.\(c\|h\|cpp\|hpp\)$' | xargs -r clang-format-14 -i
```

**Build:**
```bash
# iwasm (Linux, use darwin for macOS, windows for Windows)
cd product-mini/platforms/linux && rm -rf build && cmake -S . -B build -DCMAKE_C_FLAGS='-Wall -Werror' && cmake --build build -j$(nproc)

# wamrc (if AOT changes)
cd wamr-compiler && rm -rf build && cmake -S . -B build -DCMAKE_C_FLAGS='-Wall -Werror' && cmake --build build -j$(nproc)
```

**Tests:**
```bash
cd build && ctest --output-on-failure
cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp
cd tests/regression/ba-issues && ./build_wamr.sh && ./run.py
```

---

## Related Documentation

- [code_quality.md](./code_quality.md) - Standards and principles
- [testing.md](./testing.md) - Complete testing guide
- [dev_in_container.md](./dev_in_container.md) - Container environment
- [debugging.md](./debugging.md) - GDB and Valgrind
- [tests/regression/README.md](../tests/regression/README.md) - Regression tests
