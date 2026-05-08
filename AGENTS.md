# AI Agent Guide for WAMR Development

## Project Overview

WebAssembly Micro Runtime (WAMR) is a lightweight standalone WebAssembly runtime with small footprint, high performance, and highly configurable features.

**Core Components**:
- **VMcore** (core/iwasm/, core/shared) - Runtime engine
- **iwasm** (product-mini/platforms) - CLI executable  
- **wamrc** (wamr-compiler/) - AOT compiler

For full details: [README.md](./README.md)

---

## Project Structure

```
wasm-micro-runtime/
├── core/              # Runtime engine
├── product-mini/      # iwasm builds
├── wamr-compiler/     # wamrc AOT compiler
├── tests/             # Test suites
├── samples/           # Integration examples
└── doc/               # Strategy documentation
```

---

## Command Execution Pattern

**CRITICAL (Linux Only)**: All commands must run inside the devcontainer on Linux systems.

**Pattern**: All documentation shows pure command syntax. On Linux, prefix with:

```bash
devcontainer exec --workspace-folder . -- <command>
```

**Examples**:

| Documentation Shows | Execute on Linux |
|---------------------|------------------|
| `cmake -B build` | `devcontainer exec --workspace-folder . -- cmake -B build` |
| `ctest --test-dir build` | `devcontainer exec --workspace-folder . -- ctest --test-dir build` |

**For shell features** (pipes, variables, cd):
```bash
devcontainer exec --workspace-folder . -- bash -c "<command>"
```

**Why**: WAMR requires WASI-SDK, WABT, LLVM - only available in devcontainer.

**Details**: [doc/dev-in-container.md](./doc/dev-in-container.md)

---

## Documentation Navigation

**Documentation follows lazy loading**: Read high-level docs first, drill down only when needed.

**Architecture**: [doc/documentation-principles.md](./doc/documentation-principles.md)

### By Task Type

#### Bug Fixes
1. [doc/architecture-overview.md](./doc/architecture-overview.md) - Component relationships
2. [doc/building.md](./doc/building.md) - Build with debug flags
3. [doc/debugging.md](./doc/debugging.md) - Debug workflow
4. [doc/testing.md](./doc/testing.md) - Verify the fix
5. [doc/linting.md](./doc/linting.md) - Pre-commit checks ⚠️

#### Adding Features
1. [doc/architecture-overview.md](./doc/architecture-overview.md) - Where feature fits
2. [doc/building.md](./doc/building.md) - Configure build
3. [doc/embed_wamr.md](./doc/embed_wamr.md) - API patterns (if adding API)
4. [doc/export_native_api.md](./doc/export_native_api.md) - Native functions (if exposing)
5. [doc/testing.md](./doc/testing.md) - Write tests
6. [doc/linting.md](./doc/linting.md) - Pre-commit checks ⚠️

#### PR Reviews
1. [doc/architecture-overview.md](./doc/architecture-overview.md) - Verify architecture fit
2. [doc/testing.md](./doc/testing.md) - Check test coverage
3. [doc/code-quality.md](./doc/code-quality.md) - Verify code quality

#### Test Writing
1. [doc/testing.md](./doc/testing.md) - Test strategy
2. [tests/unit/README.md](./tests/unit/README.md) - Unit test details
3. [tests/wamr-test-suites/README.md](./tests/wamr-test-suites/README.md) - Spec test details

#### Refactoring
1. [doc/architecture-overview.md](./doc/architecture-overview.md) - Maintain principles
2. [doc/perf_tune.md](./doc/perf_tune.md) - Performance implications
3. [doc/memory_tune.md](./doc/memory_tune.md) - Memory implications

---

## Quick Reference

**Most Frequently Used**:
- [README.md](./README.md) - Project overview
- [doc/building.md](./doc/building.md) - Build guide
- [doc/testing.md](./doc/testing.md) - Testing guide
- [doc/debugging.md](./doc/debugging.md) - Debug guide
- [doc/linting.md](./doc/linting.md) - Pre-commit checklist ⚠️

**All Documentation**: Browse [doc/](./doc/) directory.
