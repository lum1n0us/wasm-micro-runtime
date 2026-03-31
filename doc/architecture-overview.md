# WAMR Architecture Overview

This document provides AI agents and developers with a mental model of WAMR's structure, component relationships, key abstractions, and design patterns. Understanding this architecture is essential for effective bug fixing, feature development, and code review.

**Related documentation:**
- [README.md](../README.md) - Project overview and features
- [doc/build_wamr.md](./build_wamr.md) - Build system details
- [doc/embed_wamr.md](./embed_wamr.md) - Embedding API guide

---

## Component Hierarchy & Relationships

WAMR consists of three main components that work together to load, compile, and execute WebAssembly modules.

### Core Components

#### 1. VMcore (core/iwasm/)

The runtime library set that loads and runs Wasm modules. VMcore provides multiple execution modes:

- **Interpreter** - Direct bytecode execution
  - Classic interpreter - Straightforward bytecode interpretation
  - Fast interpreter - Optimized interpreter with register-based execution
- **AOT Runtime** - Executes pre-compiled native code
- **JIT Engines** - Runtime compilation
  - Fast JIT - Quick compilation with basic optimizations
  - LLVM JIT - Advanced optimizations, slower compilation
  - Tier-up - Automatic transition from Fast JIT to LLVM JIT for hot code

#### 2. iwasm (product-mini/)

The standalone executable binary built with WAMR VMcore. Provides:

- Command-line interface for running Wasm modules
- WASI (WebAssembly System Interface) support
- Environment for testing and running Wasm applications

#### 3. wamrc (wamr-compiler/)

The Ahead-of-Time (AOT) compiler that converts Wasm bytecode to native machine code:

- Compiles `.wasm` files to `.aot` files
- Uses LLVM backend for optimization
- Platform-specific code generation
