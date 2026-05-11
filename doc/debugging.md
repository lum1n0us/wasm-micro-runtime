# Debugging WAMR

This guide explains debugging strategies for WAMR development, when to use different debugging approaches, and how to troubleshoot issues effectively.

**Prerequisites**:
2. [building.md](./building.md) - Build types for debugging
3. [testing.md](./testing.md) - Test-driven debugging

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

GDB is the GNU Debugger for debugging WAMR's native C code.

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

Use Debug build type for debugging WAMR. See [building.md § Build Types](./building.md#build-types) for how to create Debug builds and when to use different build types.

**WAMR-specific debugging options:**

| Build Option | Effect | Needed For |
|--------------|--------|------------|
| `WAMR_BUILD_DEBUG_INTERP=1` | Debug interpreter internals | Interpreter debugging |
| `WAMR_BUILD_DEBUG_AOT=1` | Debug AOT compilation | AOT debugging |
| `WAMR_BUILD_DUMP_CALL_STACK=1` | Print stack on errors | Crash investigation |
| `WAMR_BUILD_MEMORY_PROFILING=1` | Track memory allocations | Memory leak hunting |

### Essential GDB Commands

**1. Start debugging:**
```bash
gdb --args product-mini/platforms/linux/build-debug/iwasm test.wasm
```

**2. Set breakpoint and inspect:**
```gdb
(gdb) break wasm_runtime_instantiate
(gdb) run
(gdb) backtrace
(gdb) print *module
```

**3. Analyze crash (core dump):**
```bash
ulimit -c unlimited && product-mini/platforms/linux/build-debug/iwasm test.wasm
gdb product-mini/platforms/linux/build-debug/iwasm core
```
```gdb
(gdb) backtrace full
```

### WAMR-Specific Breakpoints

**Common debugging entry points:**
- `wasm_load` - Module loading
- `wasm_runtime_instantiate` - Module instantiation
- `wasm_runtime_call_wasm` - Function execution
- `wasm_set_exception` - Error handling
- `__assert_fail` - Assertion failures

**→ For complete GDB reference:** [GDB Documentation](https://sourceware.org/gdb/documentation/)

---

## Memory Debugging with Valgrind

Valgrind is a dynamic analysis framework for detecting memory errors, leaks, and profiling performance.

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

**Leak types**: Definitely lost (must fix), Indirectly lost, Possibly lost, Still reachable (usually OK)

**Common errors**: Invalid read/write, Use of uninitialized value, Invalid free, Mismatched free

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

AddressSanitizer (ASan) is a compiler-based memory error detector. Faster than Valgrind (2x vs 10-50x slowdown).

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

Debug WebAssembly applications at the source code level (C, Rust, etc.) using lldb.

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

**→ For complete setup, lldb commands, and troubleshooting:**
- **[source_debugging_interpreter.md](source_debugging_interpreter.md)** - Interpreter mode
- **[source_debugging_aot.md](source_debugging_aot.md)** - AOT mode
- **[source_debugging.md](source_debugging.md)** - Overview

---

## Debug Logging

WAMR provides runtime logging for diagnostics without a debugger.

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

Use Debug build type for debugging WAMR. See [building.md § Build Types](./building.md#build-types) for:
- How to create Debug builds
- When to use Debug vs RelWithDebInfo
- Build characteristics and trade-offs

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
