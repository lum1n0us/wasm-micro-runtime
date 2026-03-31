# AI Agent Guide for WAMR Development

**WebAssembly Micro Runtime (WAMR)** is a lightweight standalone WebAssembly runtime with small footprint, high performance, and highly configurable features. It includes:
- **VMcore** (core/iwasm/) - Runtime libraries for loading and running Wasm modules
- **iwasm** (product-mini/) - Executable binary with WASI support
- **wamrc** (wamr-compiler/) - AOT compiler for compiling Wasm to native code

For full project details, see [README.md](./README.md).

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

1. **[doc/architecture-overview.md](./doc/architecture-overview.md)** - Understand component relationships and critical code paths
2. **[doc/build_wamr.md](./doc/build_wamr.md)** - Build the runtime with necessary features enabled
3. **[doc/source_debugging.md](./doc/source_debugging.md)** - Set up debugging tools
4. **[doc/testing-guide.md](./doc/testing-guide.md)** *(Phase 4)* - Write tests to verify the fix

### For Adding Features

When implementing new functionality:

1. **[doc/architecture-overview.md](./doc/architecture-overview.md)** - Understand where your feature fits in the architecture
2. **[doc/build_wamr.md](./doc/build_wamr.md)** - Configure build flags for your feature
3. **[doc/embed_wamr.md](./doc/embed_wamr.md)** - If adding API, understand embedding patterns
4. **[doc/export_native_api.md](./doc/export_native_api.md)** - If exposing native functions to Wasm
5. **[doc/testing-guide.md](./doc/testing-guide.md)** *(Phase 4)* - Write comprehensive tests

### For PR Reviews

When reviewing pull requests:

1. **[doc/architecture-overview.md](./doc/architecture-overview.md)** - Verify architectural consistency
2. **[doc/dev-workflows.md](./doc/dev-workflows.md)** *(Phase 3)* - Check convention compliance
3. **[doc/testing-guide.md](./doc/testing-guide.md)** *(Phase 4)* - Verify adequate test coverage

### For Test Writing

When writing tests:

1. **[doc/testing-guide.md](./doc/testing-guide.md)** *(Phase 4)* - Comprehensive testing strategy
2. **[tests/unit/README.md](./tests/unit/README.md)** - Unit test patterns and examples
3. **[samples/README.md](./samples/README.md)** - Integration test examples

### For Refactoring

When refactoring code:

1. **[doc/architecture-overview.md](./doc/architecture-overview.md)** - Maintain architectural principles
2. **[doc/dev-workflows.md](./doc/dev-workflows.md)** *(Phase 3)* - Follow coding conventions
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
- [tests/benchmarks/](./tests/benchmarks/) - Performance benchmarks
- [samples/](./samples/) - Example applications and use cases

**All Documentation:**
Browse the [doc/](./doc/) directory for comprehensive documentation.
