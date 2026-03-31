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
