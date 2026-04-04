# Testing WAMR

This guide provides a strategic overview of testing approaches for WAMR. It explains what each test type is for, when to use it, and where to find detailed operational instructions.

**Philosophy**: WAMR uses multiple test layers to ensure correctness, spec compliance, and stability. Understanding which test to run or write for your use case is key to effective development.

---

## Prerequisites

Before running tests:

1. **Read [dev-in-container.md](dev-in-container.md)** - All tests run in the devcontainer
2. **Read [building.md](building.md)** - Tests require iwasm and sometimes wamrc
3. **Verify container**: Run `scripts/in-container.sh --status`

---

## For AI Agents

**CRITICAL**: ALL test commands MUST run inside the devcontainer using the `scripts/in-container.sh` wrapper.

**Pattern for all test commands:**
```bash
scripts/in-container.sh "<command>"
```

The script automatically detects or starts the devcontainer. Never run test commands directly on the host system.

**Error handling**: The wrapper properly propagates exit codes:
```bash
if scripts/in-container.sh "cd tests/unit/build && ctest"; then
    echo "Tests passed"
else
    echo "Tests failed"
    exit 1
fi
```

---

## Test Types Overview

WAMR uses four complementary test approaches, each serving a distinct purpose:

| Test Type | Purpose | When to Use | Scope | Documentation |
|-----------|---------|-------------|-------|---------------|
| **Unit Tests** | Verify individual components work correctly | Testing specific functions, modules, or APIs | Focused, fast, isolated | [tests/unit/README.md](../tests/unit/README.md) |
| **Spec Tests** | Validate WebAssembly specification conformance | Verifying standards compliance, proposal implementation | Comprehensive, all WASM features | [tests/wamr-test-suites/README.md](../tests/wamr-test-suites/README.md) |
| **Regression Tests** | Prevent fixed bugs from returning | After fixing any bug or CVE | Specific bug scenarios | [tests/regression/README.md](../tests/regression/README.md) |
| **Integration Tests** | Verify real-world usage patterns | Testing complete workflows, API usage | End-to-end scenarios | Samples in `samples/` |

---

## Unit Tests

### What Are Unit Tests?

Unit tests verify WAMR's core components using Google Test (C++) and custom test harnesses (C). They test individual functions, modules, and APIs in isolation from the rest of the system.

Located in: `tests/unit/`

### Why Unit Tests?

- **Fast feedback**: Run in seconds, enabling rapid development iteration
- **Precise isolation**: Pinpoint exactly which component has a bug
- **Code coverage**: Measure how much code is exercised by tests
- **Refactoring safety**: Ensure changes don't break existing functionality

### When to Use Unit Tests

**Write unit tests when:**
- Adding a new feature to WAMR (test the feature components)
- Modifying existing runtime logic (ensure no regressions)
- Fixing a bug in a specific component (prevent recurrence)
- Optimizing code (verify behavior remains correct)

**Run unit tests when:**
- During development (continuous verification)
- Before committing code (local smoke test)
- In CI/CD pipelines (automated quality gate)

### Quick Example

```bash
# Build and run all unit tests
scripts/in-container.sh "cd tests/unit && cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure"

# Run specific test category
scripts/in-container.sh "cd tests/unit && ctest --test-dir build -R interpreter --output-on-failure"
```

### Test Categories

Unit tests cover these WAMR components:
- AOT compiler and code generation
- Interpreter (classic and fast modes)
- Memory management (linear memory, Memory64, shared heap)
- Garbage collection
- Exception handling
- Custom sections
- SIMD operations
- Platform utilities

**→ See [tests/unit/README.md](../tests/unit/README.md) for complete guide** including:
- Creating new unit tests
- CMake configuration
- Generating WASM test fixtures
- Running specific tests
- Code coverage collection
- Troubleshooting

---

## WASM Spec Tests

### What Are Spec Tests?

Spec tests validate that WAMR correctly implements the official WebAssembly specification and proposals. They use the test suite from the WebAssembly Working Group to ensure standards compliance.

Located in: `tests/wamr-test-suites/`

### Why Spec Tests?

- **Standards compliance**: Prove WAMR correctly implements the WebAssembly specification
- **Proposal validation**: Test new WebAssembly features as proposals advance
- **Cross-runtime compatibility**: Ensure WASM modules work across different runtimes
- **Regression prevention**: Detect when spec conformance breaks

### When to Use Spec Tests

**Write/add spec tests when:**
- Implementing a new WebAssembly proposal (GC, Memory64, SIMD, etc.)
- The official spec test suite adds new tests
- You discover a spec compliance gap

**Run spec tests when:**
- Implementing WebAssembly proposals
- Making changes to instruction execution
- Before releasing new WAMR versions
- Testing different execution modes (interpreter, AOT, JIT)

### Quick Example

```bash
# Run core spec tests with fast interpreter
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -b"

# Run spec tests with AOT compilation
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -b"

# Test with SIMD enabled
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -S -b"
```

### Test Coverage

Spec tests validate:
- Core WebAssembly instructions (arithmetic, memory, control flow)
- Memory operations (load, store, grow, Memory64)
- Table operations and reference types
- Function calls, imports, and exports
- SIMD instructions (when enabled)
- Threading and atomics (when enabled)
- Garbage collection (when enabled)
- Exception handling (when enabled)

### Execution Modes

Test across multiple execution modes:
- `fast-interp` - Fast interpreter (recommended for development)
- `classic-interp` - Classic interpreter
- `aot` - Ahead-of-time compilation
- `jit` - LLVM JIT
- `fast-jit` - Fast JIT

**→ See [tests/wamr-test-suites/README.md](../tests/wamr-test-suites/README.md) for complete guide** including:
- All execution modes and feature flags
- Adding new proposal tests
- Understanding test results
- Architecture-specific testing
- CI integration
- Troubleshooting

---

## Regression Tests

### What Are Regression Tests?

Regression tests ensure that previously fixed bugs do not reappear when code changes. Each test represents a specific bug that was discovered, fixed, and must remain fixed in future versions.

Located in: `tests/regression/`

### Why Regression Tests?

- **Bug prevention**: Stop fixed bugs from silently returning
- **Historical record**: Document what bugs existed and how they were fixed
- **Confidence**: Make changes knowing existing fixes are validated
- **Security**: Ensure CVE fixes remain effective

### When to Use Regression Tests

**Write regression tests when:**
- Fixing any bug from the issue tracker
- Resolving a security vulnerability (CVE)
- Fixing a crash, hang, or incorrect behavior
- Addressing bugs found by fuzzing

**Run regression tests when:**
- Before merging any PR
- After making runtime changes
- Validating releases
- Testing on new platforms

**Best Practice**: Every bug fix should include a regression test.

### Quick Example

```bash
# Build WAMR variants for regression testing
scripts/in-container.sh "cd tests/regression/ba-issues && ./build_wamr.sh"

# Run all regression tests
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py"

# Test specific issue
scripts/in-container.sh "cd tests/regression/ba-issues && ./run.py --issues 2833"
```

### Test Organization

- **ba-issues/** - Tests for Bytecode Alliance issue tracker bugs
- **Future suites** - CVEs, performance regressions, platform-specific bugs

### Test Workflow

When you fix a bug:
1. Create test directory: `./helper.sh <issue-number>`
2. Add WASM file that reproduces the bug
3. Configure expected behavior in `running_config.json`
4. Run test to verify it passes with your fix
5. Commit test with the bug fix

**→ See [tests/regression/README.md](../tests/regression/README.md) and [tests/regression/ba-issues/README.md](../tests/regression/ba-issues/README.md) for complete guides** including:
- Adding new regression tests
- Test configuration reference
- Multiple execution mode testing
- Troubleshooting failed tests
- Deprecating outdated tests

---

## Integration Tests (Samples)

### What Are Integration Tests?

Integration tests verify that WAMR works correctly in real-world usage scenarios. They test complete workflows, API usage patterns, and feature interactions rather than isolated components.

Located in: `samples/`

### Why Integration Tests?

- **Real-world validation**: Test how users actually use WAMR
- **API correctness**: Verify public APIs work as documented
- **Feature interaction**: Ensure features work together correctly
- **Example code**: Provide working examples for users

### When to Use Integration Tests

**Write integration tests when:**
- Adding a new public API
- Implementing a feature users will directly interact with
- Creating examples for documentation
- Testing cross-feature scenarios

**Run integration tests when:**
- Making API changes
- Testing release candidates
- Validating platform ports

### Quick Example

```bash
# Test basic WAMR usage
scripts/in-container.sh "cd samples/basic && mkdir -p build && cd build && cmake .. && cmake --build . && ./basic"

# Test multi-threading
scripts/in-container.sh "cd samples/multi-thread && mkdir -p build && cd build && cmake .. && cmake --build . && ctest"

# Test WASM C API
scripts/in-container.sh "cd samples/wasm-c-api && mkdir -p build && cd build && cmake .. && cmake --build . && ctest"
```

### Available Sample Tests

Key integration tests:
- **basic/** - Basic iwasm usage patterns
- **file/** - File I/O with WASI
- **multi-thread/** - Multi-threading operations
- **wasm-c-api/** - WASM C API examples
- **native-lib/** - Native function registration and calls
- **socket-api/** - Socket operations with WASI
- **ref-types/** - Reference types usage
- **spawn-thread/** - Thread spawning patterns

**Note**: Integration tests are less formal than unit tests but equally important for validating real-world usage.

---

## Test Selection Guide

**Decision tree for choosing which test to write:**

```
Is this a bug fix?
├─ Yes → Write a regression test (tests/regression/)
└─ No → What are you testing?
    ├─ Spec compliance / WebAssembly proposal
    │   └─ → Check/add spec tests (tests/wamr-test-suites/)
    ├─ Specific WAMR component (function, module, API)
    │   └─ → Write unit test (tests/unit/)
    └─ Real-world usage / API workflow
        └─ → Create integration test (samples/)
```

**Common scenarios:**

| Scenario | Test Type | Example |
|----------|-----------|---------|
| Implementing GC proposal | Spec tests | Add GC spec tests to wamr-test-suites |
| Optimizing interpreter | Unit tests | Test interpreter functions in tests/unit/interpreter/ |
| Fixing crash from issue #3022 | Regression test | Add to tests/regression/ba-issues/ |
| Adding new WAMR API | Integration test | Create sample in samples/ showing API usage |
| Fixing Memory64 instruction | Spec + unit tests | Both spec conformance and unit-level verification |

**Best practice**: Most significant changes benefit from multiple test types. For example, implementing a new proposal should include:
1. Spec tests (validate conformance)
2. Unit tests (test component implementation)
3. Integration test (show real-world usage)

---

## Pre-Commit Testing

Before committing or creating a PR, run these tests locally:

### Minimum (Fast - ~2 minutes)

```bash
# Build iwasm
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. && cmake --build . -j\$(nproc)"

# Unit tests only
scripts/in-container.sh "cd tests/unit && cmake -S . -B build && cmake --build build -j\$(nproc) && ctest --test-dir build --output-on-failure"
```

### Recommended (Moderate - ~10 minutes)

```bash
# Unit tests + core spec tests
scripts/in-container.sh "cd tests/unit && cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure"
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -b"

# Regression tests
scripts/in-container.sh "cd tests/regression/ba-issues && ./build_wamr.sh && ./run.py"
```

### Thorough (Complete - ~30 minutes)

```bash
# All of the above plus multiple execution modes
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -b"
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -b"

# Feature-specific tests if applicable
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -S"  # SIMD
```

**Also see**: [code-quality.md](code-quality.md) for linting and formatting checks.

---

## Continuous Integration

WAMR uses GitHub Actions to automatically test every pull request.

### What CI Tests

**For every PR:**
- Unit tests on Linux, macOS, Windows
- Core spec tests (fast-interp and AOT modes)
- Build verification on multiple platforms
- Code formatting (clang-format)

**For feature PRs:**
- Relevant proposal spec tests (e.g., SIMD tests for SIMD changes)
- Tests with feature enabled and disabled
- Cross-architecture tests (ARM, RISC-V via QEMU)

### Viewing CI Results

1. Go to your PR on GitHub
2. Click the "Checks" tab
3. Click on failed jobs to see logs
4. Download test artifacts if available

### Reproducing CI Failures Locally

CI uses the same test scripts and devcontainer as local development:

```bash
# Example: Reproduce "spec-test-aot" CI job
scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t aot -b"

# Example: Reproduce "unit-tests" CI job
scripts/in-container.sh "cd tests/unit && cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure"
```

---

## Benchmarks

Performance benchmarks measure WAMR execution speed but are separate from correctness testing.

Located in: `tests/benchmarks/`

### Available Benchmarks

- **CoreMark** - Industry-standard CPU benchmark
- **Sightglass** - WebAssembly-focused benchmarks
- **PolyBench** - Scientific computing kernels
- **Dhrystone** - Integer performance benchmark
- **JetStream** - JavaScript/WASM benchmark suite
- **libsodium** - Cryptography performance

### Quick Example

```bash
# Run CoreMark in AOT mode
scripts/in-container.sh "cd tests/benchmarks/coremark && ./build.sh && ./run_aot.sh"
```

**Note**: Benchmarks are for performance measurement, not correctness verification. They complement but don't replace the test types above.

---

## Memory Testing

Valgrind detects memory leaks, use-after-free, buffer overflows, and other memory bugs.

### Quick Example

```bash
# Build debug version of iwasm
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build-debug && cd build-debug && cmake .. -DCMAKE_BUILD_TYPE=Debug && cmake --build ."

# Run with memory leak detection
scripts/in-container.sh "valgrind --leak-check=full product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

**When to use Valgrind:**
- Investigating crashes
- Debugging memory corruption
- Verifying leak fixes
- Testing resource cleanup

**Note**: Valgrind significantly slows execution. Use for targeted debugging, not routine testing.

---

## Troubleshooting

### Tests Fail to Run

**Problem**: Container or environment issues.

**Solution**:
```bash
# Check container status
scripts/in-container.sh --status

# Verify you're using the wrapper
# ✅ Correct: scripts/in-container.sh "cd tests/unit && ctest --test-dir build"
# ❌ Wrong: cd tests/unit && ctest --test-dir build
```

See [dev-in-container.md](dev-in-container.md#troubleshooting) for container troubleshooting.

### Test-Specific Issues

Each test type has specific troubleshooting:

- **Unit test issues** → See [tests/unit/README.md](../tests/unit/README.md#troubleshooting)
- **Spec test issues** → See [tests/wamr-test-suites/README.md](../tests/wamr-test-suites/README.md#troubleshooting)
- **Regression test issues** → See [tests/regression/ba-issues/README.md](../tests/regression/ba-issues/README.md#troubleshooting)

### Common Issues Across All Tests

**Tests pass locally but fail in CI:**
- Ensure you're using the container wrapper locally
- Check if tests depend on specific environment state
- Verify file paths are relative, not absolute

**Tests are flaky:**
- Check for race conditions (run with `--repeat until-fail:10`)
- Look for uninitialized variables (run under Valgrind)
- Verify tests are independent (don't share state)

**Build failures:**
- Clean build directory: `rm -rf build`
- Rebuild iwasm: See [building.md](building.md)
- Check compiler and dependency versions (should match container)

---

## Related Documentation

### Core Documentation
- **[building.md](building.md)** - Building iwasm and wamrc
- **[dev-in-container.md](dev-in-container.md)** - Devcontainer setup and usage
- **[code-quality.md](code-quality.md)** - Linting, formatting, and code standards
- **[debugging.md](debugging.md)** - Debugging WAMR with GDB and other tools

### Test Documentation
- **[tests/unit/README.md](../tests/unit/README.md)** - Complete unit testing guide
- **[tests/wamr-test-suites/README.md](../tests/wamr-test-suites/README.md)** - Spec test suite guide
- **[tests/regression/README.md](../tests/regression/README.md)** - Regression testing overview
- **[tests/regression/ba-issues/README.md](../tests/regression/ba-issues/README.md)** - BA issues test details

### External Resources
- [WebAssembly Specification](https://webassembly.github.io/spec/)
- [WebAssembly Test Suite](https://github.com/WebAssembly/testsuite)
- [Google Test Documentation](https://google.github.io/googletest/)
- [CMake CTest Documentation](https://cmake.org/cmake/help/latest/manual/ctest.1.html)

---

**Documentation Version**: 2.0.0  
**Last Updated**: 2026-04-04  
**Maintained By**: WAMR Development Team
