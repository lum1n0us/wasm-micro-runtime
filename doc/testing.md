# Testing WAMR

This guide provides a strategic overview of testing approaches for WAMR. It explains what each test type is for, when to use it, and where to find detailed operational instructions.

**Philosophy**: WAMR uses multiple test layers to ensure correctness, spec compliance, and stability. Understanding which test to run or write for your use case is key to effective development.

**Prerequisites**:
1. [AGENTS.md](../AGENTS.md) - Navigation and execution patterns
2. [building.md](./building.md) - Tests require iwasm and sometimes wamrc

> **Execution**: Commands in pure form. See [AGENTS.md § Command Execution Pattern](../AGENTS.md#command-execution-pattern).

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
cd tests/unit && cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure
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
cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -b
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
# Run all regression tests
cd tests/regression/ba-issues && ./run.py
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
cd samples/basic && mkdir -p build && cd build && cmake .. && cmake --build . && ./basic
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

**Best practice**: Most significant changes benefit from multiple test types.

---

## Pre-Commit Testing

Before committing or creating a PR, run appropriate tests locally. Start with unit tests, add spec tests for runtime changes, and include regression tests for bug fixes.

```bash
# Example: Unit + spec + regression tests
cd tests/unit && cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure
cd tests/wamr-test-suites && ./test_wamr.sh -s spec -t fast-interp -b
cd tests/regression/ba-issues && ./run.py
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

CI uses the same test scripts as local development. Run the corresponding test command from the sections above.

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
cd tests/benchmarks/coremark && ./build.sh && ./run_aot.sh
```

**Note**: Benchmarks are for performance measurement, not correctness verification. They complement but don't replace the test types above.

---

## Memory Testing

Valgrind detects memory leaks, use-after-free, buffer overflows, and other memory bugs.

### Quick Example

```bash
# Run with memory leak detection
valgrind --leak-check=full iwasm test.wasm
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

**Solution**: See [AGENTS.md](../AGENTS.md) for execution patterns and environment setup.

### Test-Specific Issues

Each test type has specific troubleshooting:

- **Unit test issues** → See [tests/unit/README.md](../tests/unit/README.md#troubleshooting)
- **Spec test issues** → See [tests/wamr-test-suites/README.md](../tests/wamr-test-suites/README.md#troubleshooting)
- **Regression test issues** → See [tests/regression/ba-issues/README.md](../tests/regression/ba-issues/README.md#troubleshooting)

### Common Issues

- **Tests pass locally but fail in CI**: Check execution patterns in [AGENTS.md](../AGENTS.md)
- **Tests are flaky**: Check for race conditions, uninitialized variables, shared state
- **Build failures**: Clean build directory, rebuild iwasm (see [building.md](building.md))

---

**Related**: See [AGENTS.md § Navigation](../AGENTS.md#documentation-navigation) for complete workflow guides.
