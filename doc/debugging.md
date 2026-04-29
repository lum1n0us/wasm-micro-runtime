# Debugging WAMR

This guide explains debugging strategies for WAMR and provides guidance on selecting the right debugging tools. It covers both WAMR runtime debugging (native code) and WebAssembly application debugging (source-level).

**Philosophy**: Different debugging scenarios require different tools. Understanding which tool to use and when is key to efficient debugging.

---

## Prerequisites

Before debugging WAMR:

1. **Read [AGENTS.md](../AGENTS.md)** - Platform-specific execution requirements
2. **Read [dev-in-container.md](dev-in-container.md)** - Container technical details
3. **Read [building.md](building.md)** - Debugging requires debug builds

> **Note**: All commands in this guide show raw syntax. See [AGENTS.md](../AGENTS.md) for platform-specific execution.

**Quick debug build:**
```bash
cd product-mini/platforms/linux && mkdir -p build-debug && cd build-debug && cmake .. -DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_DUMP_CALL_STACK=1 && cmake --build . -j$(nproc)
```

---

## Debugging Tools Overview

The devcontainer provides all necessary debugging tools:
- GDB with Python support
- Valgrind with all tools (memcheck, callgrind, cachegrind)
- lldb for WebAssembly source debugging
- Debug builds of system libraries
- Proper capabilities (SYS_PTRACE, seccomp=unconfined)

---

## Debugging Tools Overview

WAMR supports multiple debugging approaches depending on what you're debugging:

| Tool | What It Debugs | Best For | When to Use | Documentation |
|------|----------------|----------|-------------|---------------|
| **GDB** | WAMR runtime (native C code) | Crashes, assertions, runtime behavior | Debugging WAMR itself, investigating segfaults | See below |
| **Valgrind** | Memory errors, leaks, performance | Memory corruption, leaks, profiling | Finding memory bugs, performance analysis | See below |
| **lldb (interpreter)** | WASM source code (C/Rust/etc.) | Debugging WASM applications | Stepping through WASM source code | [source_debugging_interpreter.md](source_debugging_interpreter.md) |
| **lldb (AOT)** | AOT-compiled WASM code | Debugging AOT modules | Source-level debugging of AOT | [source_debugging_aot.md](source_debugging_aot.md) |
| **AddressSanitizer** | Memory safety violations | Use-after-free, buffer overflows | Faster than Valgrind, integrated | See below |
| **Log levels** | Runtime behavior, diagnostics | Understanding execution flow | Quick diagnostics without debugger | See below |

---

## Debugging Decision Guide

**Choose your tool based on the problem:**

```
┌─ Crash or segfault? ──────────────────────────→ GDB
│
├─ Memory leak or corruption? ──────────────────→ Valgrind memcheck or AddressSanitizer
│
├─ Performance bottleneck? ─────────────────────→ Valgrind callgrind
│
├─ Need to debug WASM application source? ─────→ lldb (source debugging)
│  ├─ Interpreter mode? ──────────────────────→ lldb with patch (see source_debugging_interpreter.md)
│  └─ AOT mode? ──────────────────────────────→ lldb with GDB JIT loader (see source_debugging_aot.md)
│
├─ Assertion failure? ─────────────────────────→ GDB (break on __assert_fail)
│
├─ Need quick diagnostics? ────────────────────→ WAMR_LOG_LEVEL environment variable
│
└─ Understanding execution flow? ──────────────→ Log levels + GDB tracing
```

---

## GDB Debugging

### What Is GDB?

GDB is the GNU Debugger for native code. It allows you to debug WAMR's C code: set breakpoints, inspect variables, examine stack traces, and step through execution.

### When to Use GDB

**Use GDB when:**
- WAMR crashes (segmentation fault, assertion failure)
- Investigating WAMR runtime behavior
- Debugging module loading/validation
- Debugging execution engine (interpreter, AOT, JIT)
- Analyzing core dumps
- Need to inspect WAMR internal state

**Don't use GDB for:**
- Debugging WASM application source code (use lldb with source debugging instead)
- Memory leak detection (use Valgrind for comprehensive analysis)
- Performance profiling (use Valgrind callgrind)

### Debug Build Requirements

| Build Option | Effect | Needed For |
|--------------|--------|------------|
| `CMAKE_BUILD_TYPE=Debug` | Enables `-g`, disables `-O2` | All debugging |
| `WAMR_BUILD_DEBUG_INTERP=1` | Debug interpreter internals | Interpreter debugging |
| `WAMR_BUILD_DEBUG_AOT=1` | Debug AOT compilation | AOT debugging |
| `WAMR_BUILD_DUMP_CALL_STACK=1` | Print stack on errors | Crash investigation |
| `WAMR_BUILD_MEMORY_PROFILING=1` | Track memory allocations | Memory leak hunting |

### Quick Examples

**Basic debugging session:**
```bash
gdb --args product-mini/platforms/linux/build-debug/iwasm test.wasm
```

In GDB:
```gdb
(gdb) break wasm_runtime_instantiate
(gdb) run
(gdb) backtrace
(gdb) print *module
(gdb) continue
```

**Crash investigation:**
```bash
# Enable core dumps and run
ulimit -c unlimited && product-mini/platforms/linux/build-debug/iwasm test.wasm

# Analyze core dump
gdb product-mini/platforms/linux/build-debug/iwasm core
```

In GDB:
```gdb
(gdb) backtrace full
(gdb) frame 0
(gdb) info registers
(gdb) list
```

### Common Breakpoints

**Module loading:**
```gdb
break wasm_load
break load_from_sections
break wasm_loader_prepare_bytecode
```

**Execution:**
```gdb
break wasm_runtime_call_wasm
break wasm_interp_call_wasm
break aot_call_function
```

**Memory operations:**
```gdb
break wasm_runtime_malloc
break wasm_runtime_free
break wasm_runtime_validate_app_addr
```

**Errors:**
```gdb
break wasm_set_exception
break __assert_fail
```

### GDB Workflows

**1. Crash Investigation:**
```bash
# Build with debug symbols
cd product-mini/platforms/linux && cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_DUMP_CALL_STACK=1 && cmake --build build-debug

# Run under GDB
gdb --args product-mini/platforms/linux/build-debug/iwasm test.wasm
```

**2. Module Loading Issues:**
```gdb
(gdb) break wasm_load
(gdb) break wasm_set_exception
(gdb) commands 2
> print (char*)exception_buf
> backtrace
> continue
> end
(gdb) run
```

**3. Execution Tracing:**
```gdb
(gdb) break wasm_runtime_call_wasm
(gdb) commands
> silent
> printf "Calling function: %s\n", function->func_name
> continue
> end
(gdb) run
```

**→ For complete GDB reference, command examples, and troubleshooting, see:** [GDB Debugging Guide](https://sourceware.org/gdb/documentation/) (external) and WAMR-specific breakpoints above.

---

## Memory Debugging with Valgrind

### What Is Valgrind?

Valgrind is a dynamic analysis framework that detects memory errors, leaks, and can profile performance. It runs your program in a virtual environment that tracks all memory operations.

### When to Use Valgrind

**Use Valgrind when:**
- Hunting memory leaks (memory not freed)
- Detecting invalid memory access (buffer overflows, use-after-free)
- Finding uninitialized memory usage
- Performance profiling (with callgrind)
- Cache analysis (with cachegrind)

**Don't use Valgrind for:**
- Quick crash debugging (use GDB, Valgrind is slow)
- Real-time performance testing (Valgrind adds 10-50x slowdown)
- Source-level debugging (use GDB or lldb)

### Valgrind Tools

| Tool | Purpose | Use Case |
|------|---------|----------|
| **memcheck** | Memory errors and leaks | Default tool, most common |
| **callgrind** | Call graph profiling | Find performance bottlenecks |
| **cachegrind** | Cache simulation | Optimize cache usage |
| **helgrind** | Thread error detection | Debugging multithreaded code |
| **massif** | Heap profiler | Track heap usage over time |

### Quick Examples

**Memory leak detection:**
```bash
valgrind --leak-check=full --show-leak-kinds=all product-mini/platforms/linux/build-debug/iwasm test.wasm
```

**Memory error detection:**
```bash
valgrind --tool=memcheck --track-origins=yes product-mini/platforms/linux/build-debug/iwasm test.wasm
```

**Performance profiling:**
```bash
valgrind --tool=callgrind --callgrind-out-file=callgrind.out product-mini/platforms/linux/build-debug/iwasm test.wasm
callgrind_annotate callgrind.out | head -50
```

### Understanding Valgrind Output

**Leak types:**
- **Definitely lost**: Real leak, must fix
- **Indirectly lost**: Leaked memory referenced by definitely lost blocks
- **Possibly lost**: Pointer to block interior, may or may not be leak
- **Still reachable**: Memory still pointed to at exit (usually OK for cleanup)

**Common errors:**
- **Invalid read/write**: Buffer overflow or out-of-bounds access
- **Use of uninitialized value**: Variable used before initialization
- **Invalid free**: Double free or freeing invalid pointer
- **Mismatched free**: Using wrong free function (malloc/new mismatch)

### Valgrind Workflow

**Memory leak hunting:**
```bash
# 1. Build with memory profiling
cd product-mini/platforms/linux && cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_MEMORY_PROFILING=1 && cmake --build build-debug

# 2. Run with Valgrind
valgrind --leak-check=full --track-origins=yes --log-file=leak.log product-mini/platforms/linux/build-debug/iwasm test.wasm

# 3. Analyze report
cat leak.log | grep 'definitely lost' -A 10
```

**→ For complete Valgrind options, suppression files, and advanced usage, see:** [Valgrind User Manual](https://valgrind.org/docs/manual/manual.html) (external).

---

## AddressSanitizer

### What Is AddressSanitizer?

AddressSanitizer (ASan) is a compiler-based memory error detector. It's faster than Valgrind and detects similar memory bugs by instrumenting code at compile time.

### When to Use AddressSanitizer

**Use ASan when:**
- Need faster memory error detection than Valgrind (only 2x slowdown)
- Running in CI/CD (faster than Valgrind)
- Detecting use-after-free, buffer overflows, stack corruption
- Want precise error locations with stack traces

**Don't use ASan for:**
- Production builds (adds overhead and extra memory)
- When you need leak detection without recompiling (use Valgrind)

### Quick Example

**Build with ASan:**
```bash
cd product-mini/platforms/linux && cmake -B build-asan -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS='-fsanitize=address -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address' && cmake --build build-asan
```

**Run:**
```bash
product-mini/platforms/linux/build-asan/iwasm test.wasm
```

ASan will automatically print detailed error reports on memory violations.

**→ For complete ASan documentation, see:** [AddressSanitizer Documentation](https://github.com/google/sanitizers/wiki/AddressSanitizer) (external).

---

## Source-Level Debugging (WASM Applications)

### What Is Source-Level Debugging?

Source-level debugging allows you to debug WebAssembly applications at the source code level (C, Rust, etc.) using lldb. You can set breakpoints in your WASM source, step through code, and inspect variables.

### When to Use Source-Level Debugging

**Use lldb source debugging when:**
- Debugging a WASM application (not WAMR runtime)
- Need to step through WASM source code
- Investigating WASM application bugs
- Want to inspect WASM application variables

**Don't use source debugging for:**
- Debugging WAMR runtime itself (use GDB)
- Quick crash investigation (use GDB for native crashes)

### Requirements

1. **WASM module compiled with debug info** (`-g` flag):
```bash
/opt/wasi-sdk/bin/clang -g test.c -o test.wasm
```

2. **Verify debug info** (optional):
```bash
llvm-dwarfdump test.wasm
```

3. **WAMR built with debug support**:
   - Interpreter: `WAMR_BUILD_DEBUG_INTERP=1`
   - AOT: `WAMR_BUILD_DEBUG_AOT=1`

### Debugging Modes

**Interpreter Mode:**
- Uses patched lldb with WebAssembly support
- Connects to iwasm via network (GDB remote protocol)
- Full source-level debugging capabilities

**→ See [source_debugging_interpreter.md](source_debugging_interpreter.md) for complete guide**

**AOT Mode:**
- Uses lldb with GDB JIT loader
- Debugs AOT-compiled native code
- Maps back to WASM source via DWARF info

**→ See [source_debugging_aot.md](source_debugging_aot.md) for complete guide**

### Quick Example (Interpreter)

```bash
# Terminal 1: Run iwasm with debug engine
product-mini/platforms/linux/build-debug/iwasm -g=127.0.0.1:1234 test.wasm

# Terminal 2: Connect lldb
lldb
```

In lldb:
```
(lldb) process connect -p wasm connect://127.0.0.1:1234
(lldb) b main
(lldb) continue
(lldb) step
```

**→ For complete setup, lldb commands, and troubleshooting:**
- **[source_debugging_interpreter.md](source_debugging_interpreter.md)** - Interpreter mode
- **[source_debugging_aot.md](source_debugging_aot.md)** - AOT mode
- **[source_debugging.md](source_debugging.md)** - Overview

---

## Debug Logging

### What Is Debug Logging?

WAMR provides runtime logging for diagnostics without a debugger. Log levels control verbosity from fatal errors only to trace-level debugging.

### When to Use Logging

**Use logging when:**
- Need quick diagnostics without debugger setup
- Understanding execution flow
- Debugging in environments where debuggers are unavailable
- CI/CD diagnostics

### Log Levels

Control log verbosity with `WAMR_LOG_LEVEL` environment variable:

| Level | Name | Description |
|-------|------|-------------|
| 0 | NONE | No logging (default in release) |
| 1 | FATAL | Fatal errors only |
| 2 | ERROR | Errors and warnings |
| 3 | INFO | Informational messages (default in debug) |
| 4 | DEBUG | Debug messages |
| 5 | VERBOSE | Trace-level verbosity |

### Quick Examples

```bash
# Verbose logging
WAMR_LOG_LEVEL=4 product-mini/platforms/linux/build/iwasm test.wasm

# Trace level (very verbose)
WAMR_LOG_LEVEL=5 product-mini/platforms/linux/build-debug/iwasm test.wasm
```

**Example output:**
```
[INFO] wasm_runtime_init
[DEBUG] wasm_runtime_load: loading module
[DEBUG] load_from_sections: parsing sections
[DEBUG] load_import_section: 5 imports
[INFO] wasm_runtime_instantiate: instantiating module
[INFO] wasm_runtime_call_wasm: calling function "main"
```

---

## Debugging Scenarios

### Scenario 1: WAMR Crashes

**Problem**: iwasm segfaults or assertion fails

**Solution**:
1. Build with debug symbols and stack dump
2. Run under GDB or capture core dump
3. Analyze backtrace to find crash location
4. Examine variables and memory state

**→ Use**: GDB (see GDB section above)

### Scenario 2: Memory Leak

**Problem**: Memory usage grows over time

**Solution**:
1. Build with memory profiling
2. Run with Valgrind memcheck
3. Analyze "definitely lost" leaks
4. Find allocation site and add missing free

**→ Use**: Valgrind memcheck (see Valgrind section above)

### Scenario 3: WASM Application Bug

**Problem**: WASM module behaves incorrectly

**Solution**:
1. Compile WASM with debug info (`-g`)
2. Build iwasm with debug support
3. Use lldb source debugging
4. Step through WASM source code

**→ Use**: lldb source debugging (see [source_debugging_interpreter.md](source_debugging_interpreter.md))

### Scenario 4: Performance Issue

**Problem**: WASM execution is slow

**Solution**:
1. Profile with Valgrind callgrind
2. Identify hot functions
3. Compare execution modes (interpreter vs AOT vs JIT)
4. Optimize bottlenecks

**→ Use**: Valgrind callgrind (see Valgrind section above)

### Scenario 5: Module Loading Failure

**Problem**: WASM module fails to load or validate

**Solution**:
1. Enable verbose logging (WAMR_LOG_LEVEL=4)
2. Check exception message
3. If unclear, debug with GDB at module loading breakpoints

**→ Use**: Logging + GDB (see sections above)

---

## Build Types for Debugging

### Debug Build

**Purpose**: Full debugging with symbols, no optimizations

**When**: Active development, crash investigation

```bash
cd product-mini/platforms/linux && cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_DUMP_CALL_STACK=1 && cmake --build build-debug
```

**Characteristics**:
- Debug symbols (`-g`)
- No optimization (`-O0`)
- Assertions enabled
- Easy to step through
- Slower execution

### RelWithDebInfo Build

**Purpose**: Debugging with some optimizations

**When**: Performance issues where Debug build is too slow

```bash
cd product-mini/platforms/linux && cmake -B build-relwithdebinfo -DCMAKE_BUILD_TYPE=RelWithDebInfo && cmake --build build-relwithdebinfo
```

**Characteristics**:
- Debug symbols (`-g`)
- Optimizations enabled (`-O2`)
- Better performance
- Harder to step through (optimized code)

---

## Common Issues

### GDB Shows No Debug Symbols

**Problem**: GDB shows "No debugging symbols found"

**Solution**: Rebuild with `CMAKE_BUILD_TYPE=Debug`

**Verify**:
```bash
file product-mini/platforms/linux/build-debug/iwasm
# Should show "with debug_info, not stripped"
```

### Valgrind Reports False Positives

**Problem**: Valgrind reports leaks in third-party libraries

**Solution**: Use suppression file to filter known false positives

**Generate suppressions**:
```bash
valgrind --gen-suppressions=all product-mini/platforms/linux/build-debug/iwasm test.wasm 2>&1 | grep -A 10 '^{' > valgrind.supp
```

**Use suppressions**:
```bash
valgrind --suppressions=valgrind.supp --leak-check=full product-mini/platforms/linux/build-debug/iwasm test.wasm
```

### Core Dump Not Generated

**Problem**: Program crashes but no core file

**Solution**: Enable core dumps

```bash
# Check limit
ulimit -c

# Enable unlimited core dumps
ulimit -c unlimited
```

### Optimized Variables Are "Optimized Out"

**Problem**: GDB can't print variables in optimized code

**Solution**: Use Debug build or try inspecting registers/stack

```gdb
(gdb) print variable
# If shows "<optimized out>", try:
(gdb) info registers
(gdb) x/xg $rbp-0x18
```

---

## External Resources

- **[GDB Documentation](https://sourceware.org/gdb/documentation/)** - Complete GDB reference
- **[Valgrind User Manual](https://valgrind.org/docs/manual/manual.html)** - Valgrind tools and options
- **[AddressSanitizer Wiki](https://github.com/google/sanitizers/wiki/AddressSanitizer)** - ASan documentation
- **[LLDB Tutorial](https://lldb.llvm.org/use/tutorial.html)** - lldb command reference
- **[WAMR Source Debugging Blog](https://bytecodealliance.github.io/wamr.dev/blog/wamr-source-debugging-basic/)** - WAMR-specific debugging guide

---

**Documentation Version**: 2.0.0  
**Last Updated**: 2026-04-04  
**Maintained By**: WAMR Development Team
