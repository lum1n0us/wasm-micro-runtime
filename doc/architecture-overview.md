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

#### 1. VMcore (core/iwasm/, core/shared)

The runtime library set that loads and runs Wasm modules.

- core/iwasm/aot/ - AOT runtime implementation
- core/iwasm/common/ - Common code can be shared between AOT runtime and Wasm runtime
- core/iwasm/compilation/ - Compilation engine using LLVM, shared between AOT compiler(wamr-compiler) and JIT
- core/iwasm/fast-jit/ - Fast JIT implementation
- core/iwasm/interpreter/ - Interpreter implementation and Wasm loader and Wasm runtime
- core/iwasm/include/ - Public API headers for embedding
  - core/iwasm/include/wasm_c_api.h - C API definitions for embedding. Another Set of public APIs for embedding.
  - core/iwasm/include/wasm_export.h - Main public API for loading and executing Wasm modules.with C API style and more features than wasm_c_api.h
- core/iwasm/libraries/ - Native libraries(call by Wasm) for both runtimes to support WASI and builtin libc, socket, threading, etc.

And platform abstraction for portability across different operating systems and environments.

- **Memory management** - core/shared/mem-alloc/
- **Platform abstraction** - core/shared/platform/
- **Utilities** - core/shared/utils/. Based on platform abstraction layer for cross-platform support.

#### 2. iwasm (product-mini/platforms)

The standalone executable binary built with WAMR VMcore. Provides:

- Command-line interface for running Wasm modules

#### 3. wamrc (wamr-compiler/)

The Ahead-of-Time (AOT) compiler that converts Wasm bytecode to native machine code:

- Compiles `.wasm` files to `.aot` files
- Uses LLVM backend for optimization and code generation. Share code with JIT compilation in VMcore.

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
  └─> Provides CLI
```

**About Dependency**

- Always use platform abstraction layer for OS interactions in VMcore
- Avoid direct dependencies between iwasm and wamrc - they should only interact through VMcore
- VMcore should be designed to be reusable in other host applications beyond iwasm

**Design Principles:**

- Maintain a minimal and efficient core runtime (VMcore) that can be embedded in various applications
- Always use Marco-based design for modularity and configurability
- Ensure clear separation of concerns between loading, instantiation, execution, and platform abstraction layers
- Always handle errors gracefully and provide informative error messages to aid debugging and development
- Release resources properly to avoid memory leaks and ensure stability in long-running applications

**Runtime Interactions:**

For every .wasm and .aot file, the runtime goes through these stages:

1. Loading

2. Instantiation

3. Execution

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
│Wasm linear memory       │
│  ├─ Data segments       │
│  ├─ Heap (grows up →)   │
│  └─ Stack (grows ← down)│
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
- Location: `core/shared/mem-alloc/ems/ems_alloc.c`

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
    stack_size,       // Wasm stack size (e.g., 8192)
    heap_size,        // Wasm heap size (e.g., 8192)
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

Location: `core/shared/mem-alloc/ems/ems_alloc.c`

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
