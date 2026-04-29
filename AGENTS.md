# AI Agent Guide for WAMR Development

**WebAssembly Micro Runtime (WAMR)** is a lightweight standalone WebAssembly runtime with small footprint, high performance, and highly configurable features. It includes:

- **VMcore** (core/iwasm/, core/shared) - Runtime libraries for loading and running Wasm modules and platform abstraction
- **iwasm** (product-mini/platforms) - Executable binary with WASI support
- **wamrc** (wamr-compiler/) - AOT compiler for compiling Wasm to native code

For full project details, see [README.md](./README.md).

## ⚠️ Development Environment Requirement

**CRITICAL (Linux Only): On Linux systems, all build, test, debug, and code quality checks MUST be performed inside the devcontainer.**

### Command Execution Pattern

All commands in WAMR documentation show the raw command syntax (e.g., `cmake -B build`, `ctest --test-dir build`). On Linux, you must prefix these with the devcontainer wrapper:

```bash
devcontainer exec --workspace-folder . -- <raw-command>
```

**Prerequisites:**
```bash
# Install devcontainer CLI if needed
npm install -g @devcontainers/cli
```

**Common Examples:**

| Documentation Shows | You Execute on Linux |
|---------------------|---------------------|
| `cmake -B build` | `devcontainer exec --workspace-folder . -- cmake -B build` |
| `ctest --test-dir build` | `devcontainer exec --workspace-folder . -- ctest --test-dir build` |
| `clang-format-14 -i file.c` | `devcontainer exec --workspace-folder . -- clang-format-14 -i file.c` |

**For commands with shell features (pipes, variables, cd):**
```bash
devcontainer exec --workspace-folder . -- bash -c "<command-with-shell-features>"
```

**Examples:**
```bash
# Multiple commands chained
devcontainer exec --workspace-folder . -- bash -c "cd tests/unit && cmake -S . -B build"

# Commands with pipes
devcontainer exec --workspace-folder . -- bash -c "find . -name '*.c' | xargs clang-format-14 -i"

# Commands with shell variables
devcontainer exec --workspace-folder . -- bash -c "cmake --build build -j\$(nproc)"
```

**Why this pattern?**
- Documentation focuses on the actual commands (portable, reusable)
- Platform-specific execution details centralized here
- Reduces repetition in other docs (smaller context window usage)
- macOS/Windows developers can use commands directly in VS Code

**Detailed guides:**
- **Container details:** [doc/dev-in-container.md](./doc/dev-in-container.md)
- **Building:** [doc/building.md](./doc/building.md)
- **Testing:** [doc/testing.md](./doc/testing.md)
- **Debugging:** [doc/debugging.md](./doc/debugging.md)
- **Code quality:** [doc/code-quality.md](./doc/code-quality.md)
- **Pre-commit checks:** [doc/linting.md](./doc/linting.md) ⚠️ **RUN BEFORE COMMITTING**

## What AI Agents Can Help With

AI agents can effectively assist with these WAMR development activities:

- **Bug fixes and debugging** - Understanding code paths, identifying root causes, implementing fixes
- **PR reviews and code quality** - Convention compliance, architectural consistency, performance implications
- **Writing and maintaining tests** - Unit tests, integration tests, following project test patterns
- **Code refactoring and optimization** - Code cleanup while maintaining architectural integrity
- **Documentation improvements** - Following progressive loading and "Think once, document once" principles

## Documentation Organization

**IMPORTANT**: WAMR documentation follows progressive loading principles to optimize context window usage.

- **Strategy Layer** (doc/*.md) - Concepts, decisions, when/why to use features
- **Operations Layer** (component READMEs) - Detailed commands, all options, troubleshooting

**Read [doc/documentation-principles.md](./doc/documentation-principles.md) if you need to**:
- Write or improve documentation
- Understand the documentation hierarchy
- Learn the "Think once, document once" principle

This ensures efficient context loading: read strategy docs for concepts, drill down to operational details only when needed.

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
5. **[doc/linting.md](./doc/linting.md)** - ⚠️ Run all pre-commit checks

### For Adding Features

When implementing new functionality:

0. **[doc/dev-in-container.md](./doc/dev-in-container.md)** - ⚠️ Container environment setup
1. **[doc/architecture-overview.md](./doc/architecture-overview.md)** - Understand where feature fits
2. **[doc/building.md](./doc/building.md)** - Configure build for your feature
3. **[doc/embed_wamr.md](./doc/embed_wamr.md)** - If adding API, understand embedding patterns
4. **[doc/export_native_api.md](./doc/export_native_api.md)** - If exposing native functions
5. **[doc/testing.md](./doc/testing.md)** - Write comprehensive tests
6. **[doc/code-quality.md](./doc/code-quality.md)** - Check code formatting
7. **[doc/linting.md](./doc/linting.md)** - ⚠️ Run all pre-commit checks

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

- [doc/dev-in-container.md](./doc/dev-in-container.md) - Development in devcontainer
- [doc/building.md](./doc/building.md) - Building WAMR
- [doc/testing.md](./doc/testing.md) - Testing strategy and practices
- [doc/debugging.md](./doc/debugging.md) - Debugging guide
- [doc/code-quality.md](./doc/code-quality.md) - Code formatting and quality
- [doc/linting.md](./doc/linting.md) - Pre-commit checklist
- [doc/documentation-principles.md](./doc/documentation-principles.md) - Documentation organization and best practices
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
