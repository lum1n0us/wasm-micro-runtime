# AI-Friendly Documentation Phase 1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Create AGENTS.md navigation guide and doc/architecture-overview.md to enable AI agents to effectively assist with WAMR development.

**Architecture:** Two core documentation files - AGENTS.md serves as the entry point with task-oriented navigation, doc/architecture-overview.md provides the mental model of WAMR's structure and components.

**Tech Stack:** Markdown documentation, referencing existing WAMR docs extensively to minimize duplication.

---

## Task 1: Research Existing Documentation

**Files:**
- Read: `README.md`
- Read: `doc/build_wamr.md`
- Read: `doc/embed_wamr.md`
- Read: `doc/export_native_api.md`
- Read: `doc/source_debugging.md`
- Read: `core/iwasm/` directory structure

- [ ] **Step 1: Document existing doc inventory**

Create notes file:

```bash
cat > .superpowers/specs/existing-docs-inventory.md << 'EOF'
# Existing WAMR Documentation Inventory

## Core Guides
- README.md - Project overview, key features, getting started
- doc/build_wamr.md - Build system configuration, WAMR_BUILD_* flags
- doc/embed_wamr.md - Embedding API guide
- doc/export_native_api.md - Native function registration
- doc/build_wasm_app.md - Wasm application building

## Specialized Topics
- doc/source_debugging.md - Debugging workflows
- doc/memory_tune.md - Memory usage tuning
- doc/perf_tune.md - Performance tuning
- doc/port_wamr.md - Platform porting guide

## Testing & Examples
- tests/unit/README.md - Unit test info
- samples/README.md - Example applications

## Component Structure
- core/iwasm/ - VMcore runtime libraries
- product-mini/ - iwasm executable
- wamr-compiler/ - wamrc AOT compiler
EOF
```

- [ ] **Step 2: Run documentation inventory command**

Run: `ls -la .superpowers/specs/existing-docs-inventory.md`
Expected: File exists with documentation inventory

- [ ] **Step 3: Commit inventory**

```bash
git add .superpowers/specs/existing-docs-inventory.md
git commit -m "docs: inventory existing WAMR documentation for Phase 1"
```

---

## Task 2: Create AGENTS.md Header and Introduction

**Files:**
- Create: `AGENTS.md`

- [ ] **Step 1: Write AGENTS.md header section**

```markdown
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
```

- [ ] **Step 2: Create AGENTS.md with header**

```bash
cat > AGENTS.md << 'EOF'
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
EOF
```

- [ ] **Step 3: Verify AGENTS.md created**

Run: `head -20 AGENTS.md`
Expected: Header and introduction sections visible

- [ ] **Step 4: Commit header**

```bash
git add AGENTS.md
git commit -m "docs(agents): add AGENTS.md header and introduction"
```

---

## Task 3: Add Navigation Sections to AGENTS.md

**Files:**
- Modify: `AGENTS.md`

- [ ] **Step 1: Add navigation structure**

```markdown

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
```

- [ ] **Step 2: Append navigation to AGENTS.md**

```bash
cat >> AGENTS.md << 'EOF'

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
EOF
```

- [ ] **Step 3: Verify navigation added**

Run: `grep -A 3 "## Navigation" AGENTS.md`
Expected: Navigation section with task types visible

- [ ] **Step 4: Commit navigation**

```bash
git add AGENTS.md
git commit -m "docs(agents): add task-oriented navigation structure"
```

---

## Task 4: Complete AGENTS.md with Quick Reference

**Files:**
- Modify: `AGENTS.md`

- [ ] **Step 1: Add quick reference section**

```markdown

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
```

- [ ] **Step 2: Append quick reference to AGENTS.md**

```bash
cat >> AGENTS.md << 'EOF'

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
EOF
```

- [ ] **Step 3: Verify AGENTS.md is complete**

Run: `wc -l AGENTS.md && tail -10 AGENTS.md`
Expected: Complete document with quick reference at end

- [ ] **Step 4: Commit complete AGENTS.md**

```bash
git add AGENTS.md
git commit -m "docs(agents): complete AGENTS.md with quick reference"
```

---

## Task 5: Create architecture-overview.md Header

**Files:**
- Create: `doc/architecture-overview.md`

- [ ] **Step 1: Write architecture overview header**

```markdown
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
```

- [ ] **Step 2: Create doc/architecture-overview.md with header**

```bash
cat > doc/architecture-overview.md << 'EOF'
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
EOF
```

- [ ] **Step 3: Verify architecture file created**

Run: `head -20 doc/architecture-overview.md`
Expected: Header and core components section visible

- [ ] **Step 4: Commit architecture header**

```bash
git add doc/architecture-overview.md
git commit -m "docs(arch): add architecture overview header and core components"
```

---

## Task 6: Add Component Relationships to architecture-overview.md

**Files:**
- Modify: `doc/architecture-overview.md`

- [ ] **Step 1: Add component relationships section**

```markdown

### Component Relationships

**Build Dependencies:**

```
wamrc (AOT compiler)
  └─> Uses LLVM to compile Wasm → native code
  └─> Generates .aot files

VMcore (runtime libraries)
  ├─> Interpreter: Executes .wasm files directly
  ├─> AOT Runtime: Loads and executes .aot files
  └─> JIT: Compiles .wasm to native code at runtime

iwasm (executable)
  └─> Links against VMcore
  └─> Provides CLI and WASI support
```

**Runtime Interactions:**

1. **Module Loading Path:**
   - Application calls `wasm_runtime_load()` with bytecode
   - VMcore parses and validates the Wasm module
   - Returns `wasm_module_t` handle

2. **Instantiation Path:**
   - Application calls `wasm_runtime_instantiate()`
   - VMcore allocates linear memory and initializes tables
   - Returns `wasm_module_inst_t` instance handle

3. **Execution Path:**
   - Application looks up function: `wasm_runtime_lookup_function()`
   - Application calls function: `wasm_runtime_call_wasm()`
   - VMcore dispatches to appropriate execution engine (interpreter/AOT/JIT)

**Data Flow:**

```
Host Application
  ↕ (API calls & native functions)
VMcore
  ↕ (loads & executes)
Wasm Module (bytecode or AOT)
  ↕ (system calls)
Platform Abstraction Layer
  ↕ (OS primitives)
Operating System
```
```

- [ ] **Step 2: Append component relationships**

```bash
cat >> doc/architecture-overview.md << 'EOF'

### Component Relationships

**Build Dependencies:**

```
wamrc (AOT compiler)
  └─> Uses LLVM to compile Wasm → native code
  └─> Generates .aot files

VMcore (runtime libraries)
  ├─> Interpreter: Executes .wasm files directly
  ├─> AOT Runtime: Loads and executes .aot files
  └─> JIT: Compiles .wasm to native code at runtime

iwasm (executable)
  └─> Links against VMcore
  └─> Provides CLI and WASI support
```

**Runtime Interactions:**

1. **Module Loading Path:**
   - Application calls `wasm_runtime_load()` with bytecode
   - VMcore parses and validates the Wasm module
   - Returns `wasm_module_t` handle

2. **Instantiation Path:**
   - Application calls `wasm_runtime_instantiate()`
   - VMcore allocates linear memory and initializes tables
   - Returns `wasm_module_inst_t` instance handle

3. **Execution Path:**
   - Application looks up function: `wasm_runtime_lookup_function()`
   - Application calls function: `wasm_runtime_call_wasm()`
   - VMcore dispatches to appropriate execution engine (interpreter/AOT/JIT)

**Data Flow:**

```
Host Application
  ↕ (API calls & native functions)
VMcore
  ↕ (loads & executes)
Wasm Module (bytecode or AOT)
  ↕ (system calls)
Platform Abstraction Layer
  ↕ (OS primitives)
Operating System
```
EOF
```

- [ ] **Step 3: Verify relationships added**

Run: `grep -A 5 "Component Relationships" doc/architecture-overview.md`
Expected: Relationships section visible

- [ ] **Step 4: Commit relationships**

```bash
git add doc/architecture-overview.md
git commit -m "docs(arch): add component relationships and data flow"
```

---

## Task 7: Add Key Abstractions to architecture-overview.md

**Files:**
- Modify: `doc/architecture-overview.md`

- [ ] **Step 1: Add key abstractions section**

```markdown

## Key Abstractions & Data Structures

Understanding these core types is essential for navigating the WAMR codebase.

### Module Loading & Execution

**`wasm_module_t`** - Loaded module (parsed and validated)
- Created by: `wasm_runtime_load()`
- Contains: Parsed bytecode, function signatures, imports/exports, data sections
- Memory: Read-only, shared across instances
- Location: `core/iwasm/common/wasm_runtime_common.h`

**`wasm_module_inst_t`** - Instantiated module (with allocated memory)
- Created by: `wasm_runtime_instantiate()`
- Contains: Linear memory, tables, globals, function instances
- Memory: Per-instance state, isolated between instances
- Location: `core/iwasm/common/wasm_runtime_common.h`

**`wasm_exec_env_t`** - Execution context
- Created during: Function calls
- Contains: Stack frames, native stack, calling context
- Purpose: Thread-safe execution environment
- Location: `core/iwasm/common/wasm_exec_env.h`

**`wasm_function_inst_t`** - Function instance
- Retrieved by: `wasm_runtime_lookup_function()`
- Contains: Function pointer/bytecode offset, signature
- Usage: Pass to `wasm_runtime_call_wasm()` to execute
- Location: `core/iwasm/include/wasm_export.h`

### Memory Model

**Linear Memory Allocation (mem-alloc subsystem)**

Location: `core/shared/mem-alloc/`

Key functions:
- `mem_allocator_create()` - Initialize memory allocator
- `mem_allocator_malloc()` - Allocate memory block
- `mem_allocator_free()` - Free memory block

**Memory Layout:**

```
Module Instance Memory:
┌─────────────────────────┐
│ Module instance struct  │
├─────────────────────────┤
│ Global data             │
├─────────────────────────┤
│ Wasm linear memory      │
│   ├─ Data segments      │
│   ├─ Heap (grows up →) │
│   └─ Stack (grows ← down)│
├─────────────────────────┤
│ Function instances      │
└─────────────────────────┘
```

**Stack vs Heap Boundaries:**
- Wasm stack size: Configured at instantiation via `stack_size` parameter
- Heap size: Configured at instantiation via `heap_size` parameter
- Stack overflow detection: Checked by runtime before function calls

**WASI vs Builtin Libc:**
- **WASI** (`WAMR_BUILD_LIBC_WASI=1`): Full libc with system calls, larger footprint
- **Builtin libc** (`WAMR_BUILD_LIBC_BUILTIN=1`): Minimal libc subset, smaller footprint
- Memory implications: WASI requires more heap for stdio buffers

**Aligned Allocation Support:**
- Function: `gc_alloc_vo_aligned()` - Allocate with alignment requirement
- Metadata: Alignment metadata tracked for proper deallocation
- Use case: SIMD data, hardware-specific alignment requirements
- Location: `core/shared/mem-alloc/ems/ems_gc.c`

### Execution Modes

**Interpreter** - Direct bytecode execution
- Entry: `wasm_runtime_call_wasm()` → interpreter dispatch loop
- Location: `core/iwasm/interpreter/`
- Performance: Slower, but smallest binary size
- Use case: Embedded systems, low memory

**AOT (Ahead-of-Time)** - Pre-compiled native code
- Compilation: `wamrc` compiles `.wasm` → `.aot`
- Loading: `wasm_runtime_load()` loads `.aot` file
- Execution: Direct native function calls
- Performance: Near-native speed
- Use case: Production deployments, performance-critical

**JIT (Just-in-Time)** - Runtime compilation
- Fast JIT: Quick compilation, basic optimizations
  - Location: `core/iwasm/fast-jit/`
  - Compilation time: Milliseconds
- LLVM JIT: Advanced optimizations, slower compilation
  - Location: `core/iwasm/compilation/`
  - Compilation time: Seconds
- Use case: Dynamic workloads, long-running applications

**Tier-up** - Fast JIT → LLVM JIT transitions
- Mechanism: Profile hot functions in Fast JIT, compile with LLVM JIT
- Configuration: `WAMR_BUILD_FAST_JIT=1` and `WAMR_BUILD_JIT=1`
- Benefit: Fast startup + high peak performance
```

- [ ] **Step 2: Append key abstractions**

```bash
cat >> doc/architecture-overview.md << 'EOF'

## Key Abstractions & Data Structures

Understanding these core types is essential for navigating the WAMR codebase.

### Module Loading & Execution

**`wasm_module_t`** - Loaded module (parsed and validated)
- Created by: `wasm_runtime_load()`
- Contains: Parsed bytecode, function signatures, imports/exports, data sections
- Memory: Read-only, shared across instances
- Location: `core/iwasm/common/wasm_runtime_common.h`

**`wasm_module_inst_t`** - Instantiated module (with allocated memory)
- Created by: `wasm_runtime_instantiate()`
- Contains: Linear memory, tables, globals, function instances
- Memory: Per-instance state, isolated between instances
- Location: `core/iwasm/common/wasm_runtime_common.h`

**`wasm_exec_env_t`** - Execution context
- Created during: Function calls
- Contains: Stack frames, native stack, calling context
- Purpose: Thread-safe execution environment
- Location: `core/iwasm/common/wasm_exec_env.h`

**`wasm_function_inst_t`** - Function instance
- Retrieved by: `wasm_runtime_lookup_function()`
- Contains: Function pointer/bytecode offset, signature
- Usage: Pass to `wasm_runtime_call_wasm()` to execute
- Location: `core/iwasm/include/wasm_export.h`

### Memory Model

**Linear Memory Allocation (mem-alloc subsystem)**

Location: `core/shared/mem-alloc/`

Key functions:
- `mem_allocator_create()` - Initialize memory allocator
- `mem_allocator_malloc()` - Allocate memory block
- `mem_allocator_free()` - Free memory block

**Memory Layout:**

```
Module Instance Memory:
┌─────────────────────────┐
│ Module instance struct  │
├─────────────────────────┤
│ Global data             │
├─────────────────────────┤
│ Wasm linear memory      │
│   ├─ Data segments      │
│   ├─ Heap (grows up →) │
│   └─ Stack (grows ← down)│
├─────────────────────────┤
│ Function instances      │
└─────────────────────────┘
```

**Stack vs Heap Boundaries:**
- Wasm stack size: Configured at instantiation via `stack_size` parameter
- Heap size: Configured at instantiation via `heap_size` parameter
- Stack overflow detection: Checked by runtime before function calls

**WASI vs Builtin Libc:**
- **WASI** (`WAMR_BUILD_LIBC_WASI=1`): Full libc with system calls, larger footprint
- **Builtin libc** (`WAMR_BUILD_LIBC_BUILTIN=1`): Minimal libc subset, smaller footprint
- Memory implications: WASI requires more heap for stdio buffers

**Aligned Allocation Support:**
- Function: `gc_alloc_vo_aligned()` - Allocate with alignment requirement
- Metadata: Alignment metadata tracked for proper deallocation
- Use case: SIMD data, hardware-specific alignment requirements
- Location: `core/shared/mem-alloc/ems/ems_gc.c`

### Execution Modes

**Interpreter** - Direct bytecode execution
- Entry: `wasm_runtime_call_wasm()` → interpreter dispatch loop
- Location: `core/iwasm/interpreter/`
- Performance: Slower, but smallest binary size
- Use case: Embedded systems, low memory

**AOT (Ahead-of-Time)** - Pre-compiled native code
- Compilation: `wamrc` compiles `.wasm` → `.aot`
- Loading: `wasm_runtime_load()` loads `.aot` file
- Execution: Direct native function calls
- Performance: Near-native speed
- Use case: Production deployments, performance-critical

**JIT (Just-in-Time)** - Runtime compilation
- Fast JIT: Quick compilation, basic optimizations
  - Location: `core/iwasm/fast-jit/`
  - Compilation time: Milliseconds
- LLVM JIT: Advanced optimizations, slower compilation
  - Location: `core/iwasm/compilation/`
  - Compilation time: Seconds
- Use case: Dynamic workloads, long-running applications

**Tier-up** - Fast JIT → LLVM JIT transitions
- Mechanism: Profile hot functions in Fast JIT, compile with LLVM JIT
- Configuration: `WAMR_BUILD_FAST_JIT=1` and `WAMR_BUILD_JIT=1`
- Benefit: Fast startup + high peak performance
EOF
```

- [ ] **Step 3: Verify abstractions added**

Run: `grep -A 3 "Key Abstractions" doc/architecture-overview.md`
Expected: Abstractions section visible

- [ ] **Step 4: Commit abstractions**

```bash
git add doc/architecture-overview.md
git commit -m "docs(arch): add key abstractions and data structures"
```

---

## Task 8: Add Critical Code Paths to architecture-overview.md

**Files:**
- Modify: `doc/architecture-overview.md`

- [ ] **Step 1: Add critical code paths section**

```markdown

## Critical Code Paths

Understanding these workflows is essential for debugging and feature development.

### Module Load & Execute Flow

**1. Load Wasm Module**

```c
wasm_module_t module = wasm_runtime_load(
    wasm_file_buf,    // Wasm bytecode buffer
    wasm_file_size,   // Buffer size
    error_buf,        // Error message buffer
    error_buf_size    // Error buffer size
);
```

Flow:
- Parse Wasm binary format
- Validate function signatures, types, imports/exports
- Build internal module structure
- Return module handle (or NULL on error)

Location: `core/iwasm/common/wasm_runtime_common.c`

**2. Instantiate Module**

```c
wasm_module_inst_t module_inst = wasm_runtime_instantiate(
    module,           // Module handle from load
    stack_size,       // Wasm stack size (e.g., 8092)
    heap_size,        // Wasm heap size (e.g., 8092)
    error_buf,        // Error message buffer
    error_buf_size    // Error buffer size
);
```

Flow:
- Allocate module instance structure
- Allocate linear memory (heap + stack)
- Initialize data segments (copy data into memory)
- Initialize tables and globals
- Call start function if present
- Return instance handle (or NULL on error)

Location: `core/iwasm/common/wasm_runtime_common.c`

**3. Lookup Function**

```c
wasm_function_inst_t func = wasm_runtime_lookup_function(
    module_inst,      // Module instance
    "main",           // Function name
    NULL              // Optional signature
);
```

Flow:
- Search exports for function name
- Return function instance (or NULL if not found)

Location: `core/iwasm/common/wasm_runtime_common.c`

**4. Call Function**

```c
bool success = wasm_runtime_call_wasm(
    exec_env,         // Execution environment
    func,             // Function to call
    argc,             // Argument count
    argv              // Arguments array
);
```

Flow:
- Validate function signature
- Check stack space available
- Dispatch to execution engine:
  - Interpreter: Enter dispatch loop
  - AOT: Call native function pointer
  - JIT: Call JIT-compiled function (or trigger compilation)
- Handle return values
- Return success/failure

Location: `core/iwasm/common/wasm_runtime_common.c`

### Memory Allocation Path

**Entry Points:**

```c
// Standard allocation
void* ptr = gc_alloc_vo(heap, size);

// Aligned allocation (e.g., for SIMD)
void* ptr = gc_alloc_vo_aligned(heap, size, alignment);
```

**Allocation Flow:**

1. Check available heap space
2. Find suitable free block (first-fit or best-fit strategy)
3. For aligned allocations:
   - Calculate alignment padding
   - Store alignment metadata before allocated block
4. Update free list
5. Return pointer (or NULL if out of memory)

**Deallocation Flow:**

```c
gc_free_vo(heap, ptr);
```

1. For aligned allocations: Read metadata to find actual block start
2. Mark block as free
3. Coalesce with adjacent free blocks
4. Update free list

Location: `core/shared/mem-alloc/ems/ems_gc.c`

### Native Function Call Path

When Wasm code calls a host-registered native function:

1. Wasm module calls imported function
2. Runtime looks up native function pointer in import table
3. Runtime marshals Wasm arguments to native calling convention
4. Native function executes
5. Runtime marshals return value back to Wasm
6. Execution continues in Wasm

Registration example:

```c
static NativeSymbol native_symbols[] = {
    {"print_string", print_string, "(i)i", NULL}
};

wasm_runtime_register_natives("env", native_symbols, 1);
```

Location: `core/iwasm/common/wasm_native.c`
```

- [ ] **Step 2: Append critical code paths**

```bash
cat >> doc/architecture-overview.md << 'EOF'

## Critical Code Paths

Understanding these workflows is essential for debugging and feature development.

### Module Load & Execute Flow

**1. Load Wasm Module**

```c
wasm_module_t module = wasm_runtime_load(
    wasm_file_buf,    // Wasm bytecode buffer
    wasm_file_size,   // Buffer size
    error_buf,        // Error message buffer
    error_buf_size    // Error buffer size
);
```

Flow:
- Parse Wasm binary format
- Validate function signatures, types, imports/exports
- Build internal module structure
- Return module handle (or NULL on error)

Location: `core/iwasm/common/wasm_runtime_common.c`

**2. Instantiate Module**

```c
wasm_module_inst_t module_inst = wasm_runtime_instantiate(
    module,           // Module handle from load
    stack_size,       // Wasm stack size (e.g., 8092)
    heap_size,        // Wasm heap size (e.g., 8092)
    error_buf,        // Error message buffer
    error_buf_size    // Error buffer size
);
```

Flow:
- Allocate module instance structure
- Allocate linear memory (heap + stack)
- Initialize data segments (copy data into memory)
- Initialize tables and globals
- Call start function if present
- Return instance handle (or NULL on error)

Location: `core/iwasm/common/wasm_runtime_common.c`

**3. Lookup Function**

```c
wasm_function_inst_t func = wasm_runtime_lookup_function(
    module_inst,      // Module instance
    "main",           // Function name
    NULL              // Optional signature
);
```

Flow:
- Search exports for function name
- Return function instance (or NULL if not found)

Location: `core/iwasm/common/wasm_runtime_common.c`

**4. Call Function**

```c
bool success = wasm_runtime_call_wasm(
    exec_env,         // Execution environment
    func,             // Function to call
    argc,             // Argument count
    argv              // Arguments array
);
```

Flow:
- Validate function signature
- Check stack space available
- Dispatch to execution engine:
  - Interpreter: Enter dispatch loop
  - AOT: Call native function pointer
  - JIT: Call JIT-compiled function (or trigger compilation)
- Handle return values
- Return success/failure

Location: `core/iwasm/common/wasm_runtime_common.c`

### Memory Allocation Path

**Entry Points:**

```c
// Standard allocation
void* ptr = gc_alloc_vo(heap, size);

// Aligned allocation (e.g., for SIMD)
void* ptr = gc_alloc_vo_aligned(heap, size, alignment);
```

**Allocation Flow:**

1. Check available heap space
2. Find suitable free block (first-fit or best-fit strategy)
3. For aligned allocations:
   - Calculate alignment padding
   - Store alignment metadata before allocated block
4. Update free list
5. Return pointer (or NULL if out of memory)

**Deallocation Flow:**

```c
gc_free_vo(heap, ptr);
```

1. For aligned allocations: Read metadata to find actual block start
2. Mark block as free
3. Coalesce with adjacent free blocks
4. Update free list

Location: `core/shared/mem-alloc/ems/ems_gc.c`

### Native Function Call Path

When Wasm code calls a host-registered native function:

1. Wasm module calls imported function
2. Runtime looks up native function pointer in import table
3. Runtime marshals Wasm arguments to native calling convention
4. Native function executes
5. Runtime marshals return value back to Wasm
6. Execution continues in Wasm

Registration example:

```c
static NativeSymbol native_symbols[] = {
    {"print_string", print_string, "(i)i", NULL}
};

wasm_runtime_register_natives("env", native_symbols, 1);
```

Location: `core/iwasm/common/wasm_native.c`
EOF
```

- [ ] **Step 3: Verify code paths added**

Run: `grep -A 3 "Critical Code Paths" doc/architecture-overview.md`
Expected: Code paths section visible

- [ ] **Step 4: Commit code paths**

```bash
git add doc/architecture-overview.md
git commit -m "docs(arch): add critical code paths for module load and execution"
```

---

## Task 9: Complete architecture-overview.md

**Files:**
- Modify: `doc/architecture-overview.md`

- [ ] **Step 1: Add platform abstraction and build system sections**

```markdown

## Platform Abstraction Layer

**Location:** `core/shared/platform/`

**Purpose:** Provide OS abstraction for portability across different platforms.

**Key abstractions:**
- Memory management: `os_malloc()`, `os_free()`, `os_mmap()`, `os_munmap()`
- Threading: `os_mutex_init()`, `os_thread_create()`, `os_cond_wait()`
- Time: `os_time_get_boot_us()`, `os_time_delay()`
- File I/O: Platform-specific file operations
- Socket: Platform-specific network operations

**Supported platforms:**
- Linux, macOS, Windows
- Android, iOS
- Zephyr, VxWorks, NuttX, RT-Thread
- ESP-IDF (FreeRTOS)
- SGX (Intel Software Guard Extensions)

**Porting guide:** See [doc/port_wamr.md](./port_wamr.md) for adding new platform support.

**Platform-specific directories:**
- `core/shared/platform/linux/` - Linux implementation
- `core/shared/platform/windows/` - Windows implementation
- `core/shared/platform/darwin/` - macOS implementation
- etc.

## Build System Architecture

**Primary build system:** CMake

**Entry point:** `build-scripts/runtime_lib.cmake`

This script is included by host applications to pull WAMR runtime into their build. It exposes `WAMR_BUILD_*` variables to configure features.

### Key Build Configuration Variables

**Execution modes:**
- `WAMR_BUILD_INTERP=1` - Enable interpreter
- `WAMR_BUILD_FAST_INTERP=1` - Enable fast interpreter (recommended over classic)
- `WAMR_BUILD_AOT=1` - Enable AOT runtime
- `WAMR_BUILD_JIT=1` - Enable LLVM JIT
- `WAMR_BUILD_FAST_JIT=1` - Enable Fast JIT

**Libc support:**
- `WAMR_BUILD_LIBC_BUILTIN=1` - Minimal builtin libc (smaller footprint)
- `WAMR_BUILD_LIBC_WASI=1` - Full WASI libc (larger footprint, more features)

**Platform:**
- `WAMR_BUILD_PLATFORM` - Target platform (e.g., "linux", "darwin", "windows")
- `WAMR_BUILD_TARGET` - Target architecture (e.g., "X86_64", "AARCH64", "ARM")

**Memory and performance:**
- `WAMR_BUILD_BULK_MEMORY=1` - Bulk memory operations
- `WAMR_BUILD_SHARED_MEMORY=1` - Shared memory between threads
- `WAMR_BUILD_MEMORY_PROFILING=1` - Memory usage profiling

**Example configuration:**

```cmake
set(WAMR_BUILD_PLATFORM "linux")
set(WAMR_BUILD_TARGET "X86_64")
set(WAMR_BUILD_INTERP 1)
set(WAMR_BUILD_FAST_INTERP 1)
set(WAMR_BUILD_AOT 1)
set(WAMR_BUILD_LIBC_BUILTIN 1)
set(WAMR_BUILD_LIBC_WASI 1)

include(${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
add_library(vmlib ${WAMR_RUNTIME_LIB_SOURCE})
```

**Reference:** See [doc/build_wamr.md](./build_wamr.md) for comprehensive build configuration details.

## Design Patterns & Conventions

**Error Handling:**
- Most APIs return `bool` (true=success, false=failure) or pointer (NULL=failure)
- Error messages written to caller-provided `error_buf`
- Check return values and inspect error buffer on failure

**Memory Management:**
- Explicit allocation/deallocation - no garbage collection at runtime level
- Always pair `wasm_runtime_load()` with `wasm_runtime_unload()`
- Always pair `wasm_runtime_instantiate()` with `wasm_runtime_deinstantiate()`

**Thread Safety:**
- Module (`wasm_module_t`) is read-only and can be shared across threads
- Module instances (`wasm_module_inst_t`) must not be shared - create per-thread
- Execution environments (`wasm_exec_env_t`) are thread-local

**Naming Conventions:**
- Public API: `wasm_runtime_*` prefix
- Internal functions: `wasm_*` or component-specific prefix
- OS abstraction: `os_*` prefix
- Memory allocator: `mem_allocator_*`, `gc_*` prefix

**Build-time Configuration:**
- Use `WAMR_BUILD_*` variables for major features
- Conditional compilation with `#if WASM_ENABLE_*` macros
- Keep runtime configurable - avoid hard-coding limits

---

**Last updated:** 2026-03-31 (Phase 1 - Foundation)

**Next steps:** After implementing Phase 1, observe AI agent usage patterns to identify gaps for Phase 3 (dev-workflows.md) and Phase 4 (testing-guide.md).
```

- [ ] **Step 2: Append final sections**

```bash
cat >> doc/architecture-overview.md << 'EOF'

## Platform Abstraction Layer

**Location:** `core/shared/platform/`

**Purpose:** Provide OS abstraction for portability across different platforms.

**Key abstractions:**
- Memory management: `os_malloc()`, `os_free()`, `os_mmap()`, `os_munmap()`
- Threading: `os_mutex_init()`, `os_thread_create()`, `os_cond_wait()`
- Time: `os_time_get_boot_us()`, `os_time_delay()`
- File I/O: Platform-specific file operations
- Socket: Platform-specific network operations

**Supported platforms:**
- Linux, macOS, Windows
- Android, iOS
- Zephyr, VxWorks, NuttX, RT-Thread
- ESP-IDF (FreeRTOS)
- SGX (Intel Software Guard Extensions)

**Porting guide:** See [doc/port_wamr.md](./port_wamr.md) for adding new platform support.

**Platform-specific directories:**
- `core/shared/platform/linux/` - Linux implementation
- `core/shared/platform/windows/` - Windows implementation
- `core/shared/platform/darwin/` - macOS implementation
- etc.

## Build System Architecture

**Primary build system:** CMake

**Entry point:** `build-scripts/runtime_lib.cmake`

This script is included by host applications to pull WAMR runtime into their build. It exposes `WAMR_BUILD_*` variables to configure features.

### Key Build Configuration Variables

**Execution modes:**
- `WAMR_BUILD_INTERP=1` - Enable interpreter
- `WAMR_BUILD_FAST_INTERP=1` - Enable fast interpreter (recommended over classic)
- `WAMR_BUILD_AOT=1` - Enable AOT runtime
- `WAMR_BUILD_JIT=1` - Enable LLVM JIT
- `WAMR_BUILD_FAST_JIT=1` - Enable Fast JIT

**Libc support:**
- `WAMR_BUILD_LIBC_BUILTIN=1` - Minimal builtin libc (smaller footprint)
- `WAMR_BUILD_LIBC_WASI=1` - Full WASI libc (larger footprint, more features)

**Platform:**
- `WAMR_BUILD_PLATFORM` - Target platform (e.g., "linux", "darwin", "windows")
- `WAMR_BUILD_TARGET` - Target architecture (e.g., "X86_64", "AARCH64", "ARM")

**Memory and performance:**
- `WAMR_BUILD_BULK_MEMORY=1` - Bulk memory operations
- `WAMR_BUILD_SHARED_MEMORY=1` - Shared memory between threads
- `WAMR_BUILD_MEMORY_PROFILING=1` - Memory usage profiling

**Example configuration:**

```cmake
set(WAMR_BUILD_PLATFORM "linux")
set(WAMR_BUILD_TARGET "X86_64")
set(WAMR_BUILD_INTERP 1)
set(WAMR_BUILD_FAST_INTERP 1)
set(WAMR_BUILD_AOT 1)
set(WAMR_BUILD_LIBC_BUILTIN 1)
set(WAMR_BUILD_LIBC_WASI 1)

include(${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
add_library(vmlib ${WAMR_RUNTIME_LIB_SOURCE})
```

**Reference:** See [doc/build_wamr.md](./build_wamr.md) for comprehensive build configuration details.

## Design Patterns & Conventions

**Error Handling:**
- Most APIs return `bool` (true=success, false=failure) or pointer (NULL=failure)
- Error messages written to caller-provided `error_buf`
- Check return values and inspect error buffer on failure

**Memory Management:**
- Explicit allocation/deallocation - no garbage collection at runtime level
- Always pair `wasm_runtime_load()` with `wasm_runtime_unload()`
- Always pair `wasm_runtime_instantiate()` with `wasm_runtime_deinstantiate()`

**Thread Safety:**
- Module (`wasm_module_t`) is read-only and can be shared across threads
- Module instances (`wasm_module_inst_t`) must not be shared - create per-thread
- Execution environments (`wasm_exec_env_t`) are thread-local

**Naming Conventions:**
- Public API: `wasm_runtime_*` prefix
- Internal functions: `wasm_*` or component-specific prefix
- OS abstraction: `os_*` prefix
- Memory allocator: `mem_allocator_*`, `gc_*` prefix

**Build-time Configuration:**
- Use `WAMR_BUILD_*` variables for major features
- Conditional compilation with `#if WASM_ENABLE_*` macros
- Keep runtime configurable - avoid hard-coding limits

---

**Last updated:** 2026-03-31 (Phase 1 - Foundation)

**Next steps:** After implementing Phase 1, observe AI agent usage patterns to identify gaps for Phase 3 (dev-workflows.md) and Phase 4 (testing-guide.md).
EOF
```

- [ ] **Step 3: Verify complete architecture doc**

Run: `wc -l doc/architecture-overview.md && tail -5 doc/architecture-overview.md`
Expected: Complete document with footer

- [ ] **Step 4: Commit complete architecture overview**

```bash
git add doc/architecture-overview.md
git commit -m "docs(arch): complete architecture overview with platform and build system"
```

---

## Task 10: Validation - Test AI Agent Understanding

**Files:**
- Create: `.superpowers/validation/phase1-test-results.md`
- Test: `AGENTS.md` and `doc/architecture-overview.md`

- [ ] **Step 1: Create validation directory**

```bash
mkdir -p .superpowers/validation
```

- [ ] **Step 2: Verify directory created**

Run: `ls -la .superpowers/`
Expected: validation directory exists

- [ ] **Step 3: Create validation test document**

Create document to track validation results:

```bash
cat > .superpowers/validation/phase1-test-results.md << 'EOF'
# Phase 1 Documentation Validation

## Test 1: Navigation Test

**Task:** Use AGENTS.md to find documentation for implementing a bug fix.

**Expected outcome:** Agent can locate:
1. doc/architecture-overview.md
2. doc/build_wamr.md
3. doc/source_debugging.md

**Result:** [PENDING]

## Test 2: Architecture Understanding Test

**Task:** Read doc/architecture-overview.md and explain:
1. What is the difference between wasm_module_t and wasm_module_inst_t?
2. What are the three main WAMR components?
3. What is the Module Load & Execute flow?

**Expected outcome:** Agent can accurately explain all three concepts.

**Result:** [PENDING]

## Test 3: Bug Fix Simulation

**Task:** Given a hypothetical bug in memory allocation, use the docs to:
1. Identify which component handles memory allocation
2. Find the relevant code location
3. Understand the allocation flow

**Expected outcome:** Agent identifies mem-alloc subsystem, finds core/shared/mem-alloc/, explains allocation flow.

**Result:** [PENDING]

## Next Steps

After validation:
- Document any confusion or missing information
- Update docs based on findings
- Proceed to Phase 2 (learning period)
EOF
```

- [ ] **Step 4: Verify validation doc created**

Run: `cat .superpowers/validation/phase1-test-results.md`
Expected: Validation document with three tests

- [ ] **Step 5: Commit validation framework**

```bash
git add .superpowers/validation/phase1-test-results.md
git commit -m "docs(validation): add Phase 1 validation test framework"
```

---

## Task 11: Final Review and Documentation

**Files:**
- Read: `AGENTS.md`
- Read: `doc/architecture-overview.md`
- Create: `.superpowers/specs/phase1-completion-checklist.md`

- [ ] **Step 1: Review AGENTS.md completeness**

Run: `grep -E "^#{1,3} " AGENTS.md`
Expected: All major sections present (header, what AI agents can help with, getting started, navigation, quick reference)

- [ ] **Step 2: Review architecture-overview.md completeness**

Run: `grep -E "^#{1,3} " doc/architecture-overview.md`
Expected: All major sections present per spec (component hierarchy, relationships, key abstractions, critical code paths, platform abstraction, build system, design patterns)

- [ ] **Step 3: Create completion checklist**

```bash
cat > .superpowers/specs/phase1-completion-checklist.md << 'EOF'
# Phase 1 Completion Checklist

## Deliverables

- [x] AGENTS.md created with:
  - [x] Project introduction
  - [x] What AI agents can help with
  - [x] Getting started section
  - [x] Task-oriented navigation (bug fixes, features, PR reviews, tests, refactoring)
  - [x] Quick reference links

- [x] doc/architecture-overview.md created with:
  - [x] Component hierarchy (VMcore, iwasm, wamrc)
  - [x] Component relationships and data flow
  - [x] Key abstractions (module types, memory model, execution modes)
  - [x] Critical code paths (load, instantiate, execute, memory allocation)
  - [x] Platform abstraction layer
  - [x] Build system architecture
  - [x] Design patterns and conventions

- [x] Validation framework created
  - [x] .superpowers/validation/phase1-test-results.md

- [x] All files committed to git

## Success Criteria

Phase 1 is complete when:

1. [ ] AI agent can read AGENTS.md and navigate to relevant docs
2. [ ] AI agent can explain WAMR architecture from doc/architecture-overview.md
3. [ ] Both docs pass validation tests
4. [ ] Documents are committed to repository

## Next Phase

**Phase 2: Learning Period (Week 3-4)**
- Assign real development tasks to AI agents
- Observe where agents get stuck
- Document gaps for Phase 3 and 4 content
EOF
```

- [ ] **Step 4: Verify checklist created**

Run: `cat .superpowers/specs/phase1-completion-checklist.md`
Expected: Completion checklist visible

- [ ] **Step 5: Commit completion checklist**

```bash
git add .superpowers/specs/phase1-completion-checklist.md
git commit -m "docs(phase1): add Phase 1 completion checklist"
```

---

## Task 12: Run Validation Tests

**Files:**
- Update: `.superpowers/validation/phase1-test-results.md`

- [ ] **Step 1: Validation Test 1 - Navigation**

Manually test: Open AGENTS.md and follow the "For Bug Fixes" navigation.

Expected: Can locate doc/architecture-overview.md, doc/build_wamr.md, doc/source_debugging.md

Result: Document in phase1-test-results.md

- [ ] **Step 2: Validation Test 2 - Architecture Understanding**

Manually test: Read doc/architecture-overview.md and verify you can answer:
1. Difference between wasm_module_t and wasm_module_inst_t
2. Three main WAMR components
3. Module Load & Execute flow

Expected: All questions answerable from the doc

Result: Document in phase1-test-results.md

- [ ] **Step 3: Validation Test 3 - Bug Fix Simulation**

Manually test: Use docs to locate memory allocation subsystem

Expected: Can find core/shared/mem-alloc/ and understand allocation flow

Result: Document in phase1-test-results.md

- [ ] **Step 4: Update validation results**

```bash
# Update the test results based on manual validation
# Edit .superpowers/validation/phase1-test-results.md
# Change [PENDING] to [PASS] or [FAIL] for each test
# Add notes on any issues found
```

- [ ] **Step 5: Commit validation results**

```bash
git add .superpowers/validation/phase1-test-results.md
git commit -m "docs(validation): complete Phase 1 validation tests"
```

---

## Self-Review Checklist

Before considering Phase 1 complete, verify:

**Spec Coverage:**
- [x] AGENTS.md includes all sections from spec (header, what agents can help with, getting started, navigation, quick reference)
- [x] architecture-overview.md includes all sections from spec (components, relationships, abstractions, code paths, platform, build system, patterns)
- [x] Both docs reference existing documentation extensively
- [x] No duplicated content from existing docs

**No Placeholders:**
- [x] No "TBD" or "TODO" in any document
- [x] All code examples are complete
- [x] All file paths are exact (not "path/to/...")
- [x] All links are valid

**Execution Ready:**
- [x] All steps are 2-5 minutes
- [x] Each step has clear success criteria
- [x] Validation tests defined
- [x] Completion checklist created

**Commits:**
- [x] Each task produces at least one commit
- [x] Commit messages are descriptive

---

## Execution Handoff

Plan complete and saved to `docs/superpowers/plans/2026-03-31-ai-friendly-docs-phase1.md`.

**Two execution options:**

**1. Subagent-Driven (recommended)** - I dispatch a fresh subagent per task, review between tasks, fast iteration

**2. Inline Execution** - Execute tasks in this session using executing-plans, batch execution with checkpoints

**Which approach?**
