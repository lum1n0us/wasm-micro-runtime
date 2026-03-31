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
