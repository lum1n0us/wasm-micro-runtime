# AI Agent Guide for WAMR Development

**WebAssembly Micro Runtime (WAMR)** is a lightweight standalone WebAssembly runtime with small footprint, high performance, and highly configurable features. It includes:

- **VMcore** (core/iwasm/, core/shared) - Runtime libraries for loading and running Wasm modules and platform abstraction
- **iwasm** (product-mini/platforms) - Executable binary with WASI support
- **wamrc** (wamr-compiler/) - AOT compiler for compiling Wasm to native code

For full project details, see [README.md](./README.md).

## ⚠️ Development Environment Requirement

**CRITICAL: All build, test, debug, and code quality checks MUST be performed inside the devcontainer.**

### For AI Agents

You MUST use the provided wrapper script for all development commands:

```bash
./scripts/in-container.sh "<command>"
```

The script automatically handles container detection and startup.

**Examples:**
```bash
# Building
./scripts/in-container.sh "cmake -B build"

# Testing  
./scripts/in-container.sh "ctest --test-dir build"

# Debugging
./scripts/in-container.sh "gdb ./build/iwasm"

# Code formatting
./scripts/in-container.sh "clang-format-14 --version"
```

**Detailed guides:**
- **Container environment:** [doc/dev-in-container.md](./doc/dev-in-container.md) ⚠️ **READ THIS FIRST**
- **Building:** [doc/building.md](./doc/building.md)
- **Testing:** [doc/testing.md](./doc/testing.md)
- **Debugging:** [doc/debugging.md](./doc/debugging.md)
- **Code quality:** [doc/code-quality.md](./doc/code-quality.md)

### For Human Developers

1. Open this project in VS Code
2. Click "Reopen in Container" when prompted
3. All commands run directly inside container

See [doc/dev-in-container.md](./doc/dev-in-container.md) for details.

## What AI Agents Can Help With

AI agents can effectively assist with these WAMR development activities:

- **Bug fixes and debugging** - Understanding code paths, identifying root causes, implementing fixes
- **PR reviews and code quality** - Convention compliance, architectural consistency, performance implications
- **Writing and maintaining tests** - Unit tests, integration tests, following project test patterns
- **Code refactoring and optimization** - Code cleanup while maintaining architectural integrity

## Getting Started: Read This First

When you start working with WAMR, follow this sequence:

1. **Read this entire AGENTS.md** to understand navigation and workflows
2. **Read [doc/architecture-overview.md](./doc/architecture-overview.md)** for the mental model of WAMR's structure
3. **Read task-specific docs** based on what you're trying to accomplish (see Navigation section below)

## Navigation: When to Read What

This section tells you which documentation to read based on your task. Follow the recommended order within each task type.

### For Bug Fixes

When fixing a bug, read in this order:

0. **[doc/dev-in-container.md](./doc/dev-in-container.md)** - ⚠️ Container environment setup
1. **[doc/architecture-overview.md](./doc/architecture-overview.md)** - Understand component relationships
2. **[doc/building.md](./doc/building.md)** - Build with necessary features
3. **[doc/debugging.md](./doc/debugging.md)** - Debug the issue
4. **[doc/testing.md](./doc/testing.md)** - Write tests to verify the fix

### For Adding Features

When implementing new functionality:

0. **[doc/dev-in-container.md](./doc/dev-in-container.md)** - ⚠️ Container environment setup
1. **[doc/architecture-overview.md](./doc/architecture-overview.md)** - Understand where feature fits
2. **[doc/building.md](./doc/building.md)** - Configure build for your feature
3. **[doc/embed_wamr.md](./doc/embed_wamr.md)** - If adding API, understand embedding patterns
4. **[doc/export_native_api.md](./doc/export_native_api.md)** - If exposing native functions
5. **[doc/testing.md](./doc/testing.md)** - Write comprehensive tests
6. **[doc/code-quality.md](./doc/code-quality.md)** - Check code formatting

### For PR Reviews

When reviewing pull requests:

1. **[doc/architecture-overview.md](./doc/architecture-overview.md)** - Verify architectural consistency
2. **[doc/dev-workflows.md](./doc/dev-workflows.md)** _(Phase 3)_ - Check convention compliance
3. **[doc/testing-guide.md](./doc/testing-guide.md)** _(Phase 4)_ - Verify adequate test coverage

### For Test Writing

When writing tests:

0. **[doc/dev-in-container.md](./doc/dev-in-container.md)** - ⚠️ Container environment setup
1. **[doc/testing.md](./doc/testing.md)** - Comprehensive testing strategy
2. **[tests/unit/README.md](./tests/unit/README.md)** - Unit test patterns
3. **[tests/wamr-test-suites/](./tests/wamr-test-suites/)** - Wasm spec tests
4. **[samples/README.md](./samples/README.md)** - Integration examples

### For Refactoring

When refactoring code:

1. **[doc/architecture-overview.md](./doc/architecture-overview.md)** - Maintain architectural principles
2. **[doc/dev-workflows.md](./doc/dev-workflows.md)** _(Phase 3)_ - Follow coding conventions
3. **[doc/perf_tune.md](./doc/perf_tune.md)** - Understand performance implications
4. **[doc/memory_tune.md](./doc/memory_tune.md)** - Understand memory usage implications

## Quick Reference

Frequently accessed documentation:

**Core Documentation:**

- [README.md](./README.md) - Project overview, features, getting started
- [doc/build_wamr.md](./doc/build_wamr.md) - Build instructions and configuration flags
- [doc/embed_wamr.md](./doc/embed_wamr.md) - Embedding WAMR into applications
- [doc/export_native_api.md](./doc/export_native_api.md) - Registering native functions
- [doc/architecture-overview.md](./doc/architecture-overview.md) - Component structure and design

**Development Workflows:**

- [doc/source_debugging.md](./doc/source_debugging.md) - Debugging WAMR applications
- [doc/build_wasm_app.md](./doc/build_wasm_app.md) - Building Wasm applications
- [doc/port_wamr.md](./doc/port_wamr.md) - Porting to new platforms

**Performance & Memory:**

- [doc/perf_tune.md](./doc/perf_tune.md) - Performance tuning guide
- [doc/memory_tune.md](./doc/memory_tune.md) - Memory usage optimization

**Testing & Examples:**

- [tests/unit/](./tests/unit/) - Unit test suite
- [tests/wamr-test-suites/](./tests/wamr-test-suites/) - Wasm-spec test suites
- [samples/](./samples/) - Example applications and use cases

**All Documentation:**
Browse the [doc/](./doc/) directory for comprehensive documentation.
