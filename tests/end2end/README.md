# End-to-End Tests

## Overview

End-to-end (E2E) tests validate complete workflows in WAMR, from WebAssembly Text (WAT) source code through compilation and execution. These tests exercise the entire toolchain:

1. **WAT → WASM**: Compile WebAssembly Text format to bytecode using `wat2wasm` (from WABT)
2. **WASM → AOT**: Compile bytecode to native code using `wamrc` with various flags
3. **Execution**: Run the compiled module using `iwasm` and verify output

E2E tests complement existing test suites:
- **Unit tests** (`tests/unit/`) - Test individual functions and components in isolation
- **Spec tests** (`tests/wamr-test-suites/`) - Validate conformance to WebAssembly specification
- **E2E tests** (this directory) - Validate real-world usage scenarios and configuration flags

## When to Use E2E Tests

Create an E2E test when you need to verify:

1. **Configuration flag behavior**: Testing that flags like `--enable-gc` correctly control runtime behavior
2. **Multi-stage compilation**: Testing that WAT compiles correctly through the entire pipeline
3. **Real-world scenarios**: Testing actual use cases that span multiple components
4. **Regression prevention**: Ensuring past bugs stay fixed with realistic test cases

E2E tests are more expensive than unit tests, so use them judiciously for scenarios that cannot be adequately covered by unit tests.

## Directory Structure

```
tests/end2end/
├── CMakeLists.txt              # CTest integration and configuration
├── README.md                   # This file
├── common/
│   └── test-helpers.sh        # Shared utilities for test scripts
└── gc-ref-null/               # Example: GC ref.null test suite
    ├── CMakeLists.txt         # Test discovery and registration
    ├── README.md              # Test suite documentation
    ├── run-test.sh            # Test execution script
    └── wat/                   # WAT source files
        ├── test-case-1.wat
        └── test-case-2.wat
```

Each test suite is a subdirectory with:
- `CMakeLists.txt`: Registers tests with CTest
- `README.md`: Documents the test suite and test cases
- `run-test.sh`: Main test execution script
- `wat/`: Directory containing WAT source files

## Building and Running

### Prerequisites

1. **Build WAMR tools**: You need both `wamrc` and `iwasm` built
2. **Install WABT**: The `wat2wasm` tool is required for compiling WAT to WASM

```bash
# Inside the devcontainer:
./scripts/in-container.sh "apt-get update && apt-get install -y wabt"
```

Or build WABT from source: https://github.com/WebAssembly/wabt

### Run E2E tests

```bash
# Configure with E2E tests enabled
./scripts/in-container.sh "cmake -B build -DWAMR_BUILD_E2E_TEST=ON"

# Build WAMR tools
./scripts/in-container.sh "cmake --build build --target wamrc iwasm"

# Run all E2E tests
./scripts/in-container.sh "ctest --test-dir build --output-on-failure -R e2e"
```

### Run specific tests

```bash
# Run tests from a specific suite
./scripts/in-container.sh "ctest --test-dir build --output-on-failure -R e2e_gc_ref_null"

# Run a single test case
./scripts/in-container.sh "ctest --test-dir build --output-on-failure -R e2e_gc_ref_null_enabled"
```

## Adding a New Test Suite

To add a new E2E test suite:

1. Create a subdirectory under `tests/end2end/`
2. Create `CMakeLists.txt`, `README.md`, `run-test.sh`, and `wat/` directory
3. See `gc-ref-null/` for a complete working example

The `gc-ref-null` test suite demonstrates:
- How to structure test cases
- How to use the shared test helpers
- How to register tests with CTest
- How to handle different compilation modes (AOT with/without GC)
- How to validate expected outcomes
