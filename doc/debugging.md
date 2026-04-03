# Debugging WAMR

This guide covers debugging techniques for WAMR itself and WebAssembly applications running on WAMR. It includes GDB debugging workflows, memory debugging with Valgrind, and troubleshooting common issues.

---

## Prerequisites

Before debugging WAMR, you need a debug build with symbols enabled:

1. **Read [building.md](building.md)** for complete build instructions
2. **Build with debug symbols**: Use `CMAKE_BUILD_TYPE=Debug`
3. **Enable debugging features**: Set appropriate `WAMR_BUILD_DEBUG_*` options
4. **Verify devcontainer**: Debugging tools (GDB, Valgrind) are pre-installed

**Quick debug build:**
```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build-debug && cd build-debug && cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DWAMR_BUILD_INTERP=1 \
  -DWAMR_BUILD_DUMP_CALL_STACK=1 \
  -DWAMR_BUILD_DEBUG_INTERP=1 && \
  cmake --build . -j\$(nproc)"
```

---

## For AI Agents

**CRITICAL**: All debugging commands MUST run inside the devcontainer using the `scripts/in-container.sh` wrapper script.

**Pattern for all debug commands:**
```bash
scripts/in-container.sh "<debug-command>"
```

The devcontainer provides:
- GDB with Python support
- Valgrind with all tools (memcheck, callgrind, cachegrind)
- Debug builds of system libraries
- Proper capabilities (SYS_PTRACE, seccomp=unconfined)

**Never run debugging tools directly on the host.** The container environment is configured specifically for debugging.

---

## Building for Debugging

### Debug Build Options

A proper debug build requires specific CMake options:

**Minimal debug build:**
```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build-debug && cd build-debug && cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DWAMR_BUILD_INTERP=1 && \
  cmake --build . -j\$(nproc)"
```

**Full-featured debug build:**
```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build-debug && cd build-debug && cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DWAMR_BUILD_INTERP=1 \
  -DWAMR_BUILD_AOT=1 \
  -DWAMR_BUILD_DEBUG_INTERP=1 \
  -DWAMR_BUILD_DEBUG_AOT=1 \
  -DWAMR_BUILD_DUMP_CALL_STACK=1 \
  -DWAMR_BUILD_MEMORY_PROFILING=1 \
  -DWAMR_BUILD_CUSTOM_NAME_SECTION=1 && \
  cmake --build . -j\$(nproc)"
```

**Debug options explained:**

| Option | Effect | Use Case |
|--------|--------|----------|
| `CMAKE_BUILD_TYPE=Debug` | Enables `-g`, disables `-O2` | Source-level debugging |
| `WAMR_BUILD_DEBUG_INTERP=1` | Debug interpreter internals | Debugging interpreter mode |
| `WAMR_BUILD_DEBUG_AOT=1` | Debug AOT compilation/execution | Debugging AOT mode |
| `WAMR_BUILD_DUMP_CALL_STACK=1` | Print stack on errors | Crash investigation |
| `WAMR_BUILD_MEMORY_PROFILING=1` | Track memory allocations | Memory leak hunting |
| `WAMR_BUILD_CUSTOM_NAME_SECTION=1` | Load WASM function names | Better stack traces |

**Binary location:** `product-mini/platforms/linux/build-debug/iwasm`

### RelWithDebInfo Build

For debugging with some optimizations (useful for performance issues):

```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build-relwithdebinfo && cd build-relwithdebinfo && cmake .. \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DWAMR_BUILD_INTERP=1 \
  -DWAMR_BUILD_DUMP_CALL_STACK=1 && \
  cmake --build . -j\$(nproc)"
```

**RelWithDebInfo** provides:
- Debug symbols (`-g`)
- Optimizations enabled (`-O2`)
- Better performance than Debug builds
- Harder to step through (optimized code)

**Use case:** Debugging performance issues where Debug build is too slow to reproduce the problem.

---

## Debugging with GDB

GDB is the primary debugging tool for WAMR native code.

### Basic GDB Session

Start GDB with a WASM module:

```bash
scripts/in-container.sh "gdb --args product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

**Common GDB commands:**

```gdb
# Run the program
(gdb) run

# Run with arguments
(gdb) run arg1 arg2

# Set breakpoint
(gdb) break main
(gdb) break wasm_runtime_instantiate
(gdb) break core/iwasm/interpreter/wasm_interp_classic.c:1234

# View backtrace
(gdb) backtrace
(gdb) bt full

# Inspect variables
(gdb) print module
(gdb) print *exec_env
(gdb) print frame->ip

# Step through code
(gdb) next        # Next line (step over)
(gdb) step        # Step into function
(gdb) finish      # Step out of function
(gdb) continue    # Continue execution

# Examine memory
(gdb) x/16xb 0x12345678    # Examine 16 bytes in hex
(gdb) x/s 0x12345678       # Examine string

# Watch variable
(gdb) watch module->name
(gdb) watch *0x12345678

# List source code
(gdb) list
(gdb) list function_name

# Quit
(gdb) quit
```

### Debugging Module Loading

Debug issues during WASM module loading and validation:

```bash
scripts/in-container.sh "gdb --args product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

**GDB session:**
```gdb
# Set breakpoints at key loading stages
(gdb) break wasm_load
(gdb) break wasm_loader_load
(gdb) break load_from_sections

# Run and stop at module loading
(gdb) run

# Inspect module structure
(gdb) print *module
(gdb) print module->function_count
(gdb) print module->import_count
(gdb) print module->malloc_function

# Check validation errors
(gdb) break wasm_set_exception
(gdb) commands
> print (char*)exception_buf
> continue
> end

# Continue execution
(gdb) continue
```

**Common module loading breakpoints:**
- `wasm_load` - Entry point for loading
- `load_from_sections` - Parse WASM sections
- `wasm_loader_prepare_bytecode` - Bytecode preparation
- `wasm_loader_link_wasi` - WASI imports linking

### Debugging Execution

Debug WASM code execution and runtime behavior:

```bash
scripts/in-container.sh "gdb --args product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

**Interpreter execution breakpoints:**
```gdb
# Break at interpreter entry
(gdb) break wasm_runtime_call_wasm
(gdb) break wasm_interp_call_wasm

# Break at specific opcodes (classic interpreter)
(gdb) break wasm_interp_classic.c:2000
(gdb) condition 1 opcode == 0x20  # local.get

# Inspect execution environment
(gdb) print *exec_env
(gdb) print exec_env->cur_frame
(gdb) print exec_env->wasm_stack

# Watch program counter
(gdb) watch frame->ip
(gdb) commands
> x/i frame->ip
> continue
> end

# Run
(gdb) run
```

**AOT execution breakpoints:**
```gdb
# Break at AOT entry
(gdb) break aot_call_function
(gdb) break aot_invoke_native

# Inspect AOT module
(gdb) print *aot_module
(gdb) print aot_module->func_ptrs[0]
(gdb) print aot_module->import_func_count

# Run
(gdb) run
```

**Fast JIT execution:**
```gdb
# Break at JIT compilation
(gdb) break jit_compiler_compile_op_block
(gdb) break jit_compile_op_call

# Break at JIT execution
(gdb) break jit_interp_switch_to_jitted

# Inspect JIT context
(gdb) print *jit_compiler_ctx
(gdb) print cc->jitted_addr_begin

# Run
(gdb) run
```

### Debugging Memory Issues

Debug memory corruption, out-of-bounds access, and allocation issues:

```bash
scripts/in-container.sh "gdb --args product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

**Memory debugging session:**
```gdb
# Break on memory allocation/free
(gdb) break wasm_runtime_malloc
(gdb) break wasm_runtime_free

# Watch for out-of-bounds access
(gdb) break wasm_runtime_validate_app_addr
(gdb) break wasm_runtime_addr_app_to_native

# Break on memory operations
(gdb) break memmove
(gdb) break memcpy
(gdb) break memset

# Catch segmentation faults
(gdb) catch signal SIGSEGV
(gdb) commands
> backtrace
> print $_siginfo
> end

# Run and examine crash
(gdb) run

# When stopped, examine memory
(gdb) bt full
(gdb) print module->memory_data
(gdb) x/64xb module->memory_data
```

**Memory watchpoints:**
```gdb
# Watch specific memory location
(gdb) watch *0x12345678
(gdb) rwatch *0x12345678  # Read watchpoint
(gdb) awatch *0x12345678  # Access watchpoint (read or write)

# Watch structure field
(gdb) watch module->memory_count
(gdb) watch exec_env->cur_frame->sp
```

### Debugging with Core Dumps

Analyze crashes after they occur using core dumps:

**Enable core dumps:**
```bash
# Inside container (for interactive debugging)
scripts/in-container.sh "ulimit -c unlimited"

# Or set for specific command
scripts/in-container.sh "ulimit -c unlimited && product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

**Analyze core dump:**
```bash
# Find core dump
scripts/in-container.sh "ls -lh core*"

# Load in GDB
scripts/in-container.sh "gdb product-mini/platforms/linux/build-debug/iwasm core"
```

**GDB core dump analysis:**
```gdb
# View backtrace
(gdb) backtrace full
(gdb) bt

# Examine registers at crash
(gdb) info registers

# Examine crash location
(gdb) frame 0
(gdb) list

# Print all variables in all frames
(gdb) bt full

# Examine memory at crash point
(gdb) x/32xb $rip
(gdb) x/32xb $rsp

# Print specific variables
(gdb) print module
(gdb) print *exec_env

# Switch between frames
(gdb) frame 3
(gdb) print local_var
```

**Automated crash analysis:**
```bash
# Generate backtrace from core dump
scripts/in-container.sh "gdb -batch -ex 'bt full' product-mini/platforms/linux/build-debug/iwasm core > crash-report.txt"
```

### GDB Scripting

Automate debugging tasks with GDB scripts:

**Create GDB script (`debug.gdb`):**
```gdb
# debug.gdb - WAMR debugging script

# Pretty printing
set print pretty on
set print array on
set print array-indexes on

# Auto-load symbols
set auto-solib-add on

# Breakpoints
break wasm_runtime_instantiate
break wasm_runtime_call_wasm

# Break on exceptions
break wasm_set_exception
commands
  silent
  printf "Exception: %s\n", (char*)exception_buf
  backtrace 5
  continue
end

# Useful functions
define print-module
  print module->name
  print module->function_count
  print module->import_count
  print module->export_count
end

define print-frame
  print *frame
  print frame->function->func_name
  print frame->ip
  print frame->sp
end

# Run with arguments
run
```

**Use GDB script:**
```bash
scripts/in-container.sh "gdb -x debug.gdb --args product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

**Batch mode (no interaction):**
```bash
scripts/in-container.sh "gdb -batch -x debug.gdb --args product-mini/platforms/linux/build-debug/iwasm test.wasm 2>&1 | tee debug.log"
```

---

## Memory Debugging with Valgrind

Valgrind detects memory errors, leaks, and performance issues that are hard to find with GDB.

### Memory Leak Detection

Find memory leaks in WAMR:

**Basic leak check:**
```bash
scripts/in-container.sh "valgrind --leak-check=full --show-leak-kinds=all product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

**Detailed leak report:**
```bash
scripts/in-container.sh "valgrind \
  --leak-check=full \
  --show-leak-kinds=all \
  --track-origins=yes \
  --verbose \
  --log-file=valgrind-leak.log \
  product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

**Valgrind leak check options:**

| Option | Description |
|--------|-------------|
| `--leak-check=full` | Detailed leak information with backtraces |
| `--show-leak-kinds=all` | Show all leak types (definite, indirect, possible, reachable) |
| `--track-origins=yes` | Track origin of uninitialized values (slower but helpful) |
| `--verbose` | Detailed diagnostic messages |
| `--log-file=FILE` | Save output to file |

**Understanding Valgrind leak reports:**

```text
==12345== HEAP SUMMARY:
==12345==     in use at exit: 1,024 bytes in 1 blocks
==12345==   total heap usage: 100 allocs, 99 frees, 10,240 bytes allocated
==12345==
==12345== 1,024 bytes in 1 blocks are definitely lost in loss record 1 of 1
==12345==    at 0x4C2FB0F: malloc (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
==12345==    by 0x401234: wasm_runtime_malloc (wasm_runtime_common.c:123)
==12345==    by 0x402345: wasm_runtime_instantiate (wasm_runtime.c:456)
==12345==    by 0x403456: main (main.c:789)
==12345==
==12345== LEAK SUMMARY:
==12345==    definitely lost: 1,024 bytes in 1 blocks
==12345==    indirectly lost: 0 bytes in 0 blocks
==12345==      possibly lost: 0 bytes in 0 blocks
==12345==    still reachable: 0 bytes in 0 blocks
==12345==         suppressed: 0 bytes in 0 blocks
```

**Leak types:**
- **Definitely lost**: Real memory leak, must fix
- **Indirectly lost**: Leaked memory referenced by definitely lost blocks
- **Possibly lost**: Pointer to block interior, may or may not be leak
- **Still reachable**: Memory still pointed to at exit (usually OK)

### Memory Error Detection

Detect invalid memory access, use of uninitialized memory, and other errors:

**Full memory checking:**
```bash
scripts/in-container.sh "valgrind \
  --tool=memcheck \
  --leak-check=full \
  --track-origins=yes \
  --read-var-info=yes \
  --error-exitcode=1 \
  product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

**Common memory errors detected:**

1. **Invalid read/write:**
```text
==12345== Invalid write of size 4
==12345==    at 0x401234: wasm_interp_call_func (wasm_interp.c:123)
==12345==  Address 0x5000004 is 4 bytes after a block of size 1,000 alloc'd
```

2. **Use of uninitialized value:**
```text
==12345== Conditional jump or move depends on uninitialised value(s)
==12345==    at 0x402345: wasm_runtime_call_wasm (wasm_runtime.c:456)
```

3. **Invalid free:**
```text
==12345== Invalid free() / delete / delete[]
==12345==    at 0x4C30D3B: free (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
==12345==    by 0x403456: wasm_runtime_free (wasm_runtime_common.c:789)
```

**Memcheck options:**

| Option | Description |
|--------|-------------|
| `--track-origins=yes` | Track origin of uninitialized values |
| `--read-var-info=yes` | Read DWARF3 variable info for better error messages |
| `--error-exitcode=N` | Exit with code N if errors found (for CI) |
| `--gen-suppressions=all` | Generate suppression entries for false positives |

### Valgrind with Suppressions

Suppress known false positives (e.g., from third-party libraries):

**Create suppression file (`valgrind.supp`):**
```text
{
   known_glibc_leak
   Memcheck:Leak
   match-leak-kinds: reachable
   fun:malloc
   fun:_dl_init
   obj:/lib/x86_64-linux-gnu/ld-2.*.so
}

{
   llvm_jit_reachable
   Memcheck:Leak
   match-leak-kinds: reachable
   fun:malloc
   obj:/usr/lib/llvm-*/lib/libLLVM-*.so
}
```

**Use suppression file:**
```bash
scripts/in-container.sh "valgrind --leak-check=full --suppressions=valgrind.supp product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

**Generate suppressions automatically:**
```bash
scripts/in-container.sh "valgrind --leak-check=full --gen-suppressions=all product-mini/platforms/linux/build-debug/iwasm test.wasm 2>&1 | grep -A 10 '^{'"
```

### Performance Profiling with Callgrind

Profile execution to find performance bottlenecks:

**Run Callgrind:**
```bash
scripts/in-container.sh "valgrind --tool=callgrind --callgrind-out-file=callgrind.out product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

**Analyze results:**
```bash
# View in terminal
scripts/in-container.sh "callgrind_annotate callgrind.out"

# View top functions
scripts/in-container.sh "callgrind_annotate --auto=yes --threshold=1 callgrind.out | head -50"

# View specific function
scripts/in-container.sh "callgrind_annotate --include=wasm_interp callgrind.out"
```

**Callgrind output:**
```text
--------------------------------------------------------------------------------
Profile data file 'callgrind.out' (creator: callgrind-3.15.0)
--------------------------------------------------------------------------------
I1 cache: 
D1 cache: 
LL cache: 
Timerange: Basic block 0 - 1234567
Trigger: Program termination
Profiled target:  product-mini/platforms/linux/build-debug/iwasm test.wasm
Events recorded:  Ir
Events shown:     Ir
Event sort order: Ir
Thresholds:       99
Include dirs:     
User annotated:   
Auto-annotation:  on

--------------------------------------------------------------------------------
Ir          file:function
--------------------------------------------------------------------------------
12,345,678  core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 3,456,789  core/iwasm/common/wasm_runtime_common.c:wasm_runtime_lookup_function
 2,345,678  core/iwasm/interpreter/wasm_runtime.c:wasm_runtime_call_wasm
```

**Callgrind options:**

| Option | Description |
|--------|-------------|
| `--cache-sim=yes` | Simulate cache behavior (slower, more detail) |
| `--branch-sim=yes` | Simulate branch prediction |
| `--collect-jumps=yes` | Collect jump information |
| `--separate-threads=yes` | Profile threads separately |

---

## Common Debug Workflows

Step-by-step guides for common debugging scenarios.

### Workflow 1: Crash Investigation

**Scenario:** iwasm crashes with segmentation fault.

**Steps:**

1. **Build with debug symbols:**
```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build-debug && cd build-debug && cmake .. -DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_DUMP_CALL_STACK=1 && cmake --build . -j\$(nproc)"
```

2. **Enable core dumps and reproduce:**
```bash
scripts/in-container.sh "ulimit -c unlimited && product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

3. **Analyze crash with GDB:**
```bash
scripts/in-container.sh "gdb product-mini/platforms/linux/build-debug/iwasm core"
```

4. **In GDB, examine crash:**
```gdb
(gdb) backtrace full
(gdb) frame 0
(gdb) list
(gdb) info registers
(gdb) x/32xb $rip
```

5. **Identify root cause:**
   - Check for NULL pointer dereference
   - Check for buffer overflow
   - Check for use-after-free
   - Check for uninitialized memory

6. **Verify fix:**
```bash
scripts/in-container.sh "product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

### Workflow 2: Memory Leak Hunting

**Scenario:** Memory usage grows over time.

**Steps:**

1. **Build with memory profiling:**
```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build-debug && cd build-debug && cmake .. -DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_MEMORY_PROFILING=1 && cmake --build . -j\$(nproc)"
```

2. **Run with Valgrind:**
```bash
scripts/in-container.sh "valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=leak.log product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

3. **Analyze leak report:**
```bash
scripts/in-container.sh "cat leak.log | grep 'definitely lost' -A 10"
```

4. **Find leak location:**
   - Look for "definitely lost" leaks first
   - Note the allocation backtrace
   - Find where the memory should be freed

5. **Reproduce with GDB:**
```bash
scripts/in-container.sh "gdb --args product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

```gdb
# Set breakpoint at allocation site from Valgrind report
(gdb) break wasm_runtime_malloc
(gdb) commands
> backtrace 5
> continue
> end
(gdb) run
```

6. **Fix and verify:**
```bash
# After fix, run Valgrind again
scripts/in-container.sh "valgrind --leak-check=full --error-exitcode=1 product-mini/platforms/linux/build-debug/iwasm test.wasm"
echo $?  # Should be 0 if no leaks
```

### Workflow 3: Assertion Failure Debugging

**Scenario:** Assertion fails with unclear reason.

**Steps:**

1. **Run with full stack dump:**
```bash
scripts/in-container.sh "product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

2. **Start GDB and break at assertion:**
```bash
scripts/in-container.sh "gdb --args product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

```gdb
# Break on assertion functions
(gdb) break __assert_fail
(gdb) break bh_assert
(gdb) run
```

3. **When stopped, examine state:**
```gdb
(gdb) backtrace full
(gdb) frame 2  # Go to frame before assertion
(gdb) print *module
(gdb) print exec_env->cur_frame
```

4. **Understand invariant:**
   - Read assertion message
   - Read source code around assertion
   - Check what condition was violated

5. **Trace back to root cause:**
```gdb
# Set breakpoint earlier in call chain
(gdb) break wasm_load
(gdb) run
# Step through to see where invariant is violated
(gdb) step
(gdb) next
```

6. **Fix and verify:**
```bash
scripts/in-container.sh "product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

### Workflow 4: Performance Issue Profiling

**Scenario:** WASM execution is slower than expected.

**Steps:**

1. **Profile with Callgrind:**
```bash
scripts/in-container.sh "valgrind --tool=callgrind --callgrind-out-file=callgrind.out product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

2. **Analyze hot functions:**
```bash
scripts/in-container.sh "callgrind_annotate --auto=yes callgrind.out | head -100 > profile.txt"
scripts/in-container.sh "cat profile.txt"
```

3. **Identify bottleneck:**
   - Look for functions with highest instruction counts
   - Check if bottleneck is in WAMR runtime or WASM module

4. **Profile with GDB sampling:**
```bash
scripts/in-container.sh "gdb --args product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

```gdb
# Set breakpoint at hot function
(gdb) break wasm_interp_call_func_bytecode
(gdb) commands
> silent
> backtrace 1
> continue
> end
(gdb) run
# Ctrl-C after a few seconds to see samples
```

5. **Compare execution modes:**
```bash
# Interpreter
scripts/in-container.sh "time product-mini/platforms/linux/build/iwasm test.wasm"

# Fast JIT (if available)
scripts/in-container.sh "time product-mini/platforms/linux/build-fastjit/iwasm test.wasm"

# AOT (if available)
scripts/in-container.sh "wamr-compiler/build/wamrc -o test.aot test.wasm"
scripts/in-container.sh "time product-mini/platforms/linux/build/iwasm test.aot"
```

6. **Optimize based on findings:**
   - If in interpreter, try Fast JIT or AOT
   - If in runtime, optimize hot code path
   - If in WASM, optimize WASM code

### Workflow 5: AOT/JIT Compilation Debugging

**Scenario:** AOT or JIT compilation fails or generates incorrect code.

**Steps:**

1. **Enable debug logging:**
```bash
scripts/in-container.sh "WAMR_LOG_LEVEL=4 product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

2. **Build with debug symbols:**
```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build-debug && cd build-debug && cmake .. -DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_DEBUG_AOT=1 -DWAMR_BUILD_AOT=1 && cmake --build . -j\$(nproc)"
```

3. **Debug AOT compilation:**
```bash
scripts/in-container.sh "gdb --args wamr-compiler/build/wamrc -o test.aot test.wasm"
```

```gdb
# Break at compilation stages
(gdb) break aot_compile_wasm
(gdb) break aot_compile_op_call
(gdb) break aot_emit_llvm_file
(gdb) run
```

4. **Examine generated code:**
```bash
# Dump LLVM IR
scripts/in-container.sh "wamr-compiler/build/wamrc --enable-dump-ir -o test.aot test.wasm"
scripts/in-container.sh "cat test.aot.ll"

# Disassemble AOT file
scripts/in-container.sh "objdump -d test.aot | less"
```

5. **Debug JIT execution:**
```bash
scripts/in-container.sh "gdb --args product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

```gdb
# Break at JIT compilation
(gdb) break jit_compiler_compile
(gdb) break fast_jit_compile_op_call

# Break at tier-up (multi-tier JIT)
(gdb) break jit_check_suspend_flags
(gdb) break llvm_jit_compile_func

(gdb) run
```

6. **Compare with interpreter:**
```bash
# Run in interpreter mode (for comparison)
scripts/in-container.sh "product-mini/platforms/linux/build-interp/iwasm test.wasm"

# Run with AOT
scripts/in-container.sh "product-mini/platforms/linux/build-aot/iwasm test.aot"
```

---

## Debug Logging

WAMR provides runtime logging for diagnostics without a debugger.

### Log Levels

Control log verbosity with environment variable:

```bash
# No logging (default in release builds)
scripts/in-container.sh "WAMR_LOG_LEVEL=0 product-mini/platforms/linux/build/iwasm test.wasm"

# Fatal errors only
scripts/in-container.sh "WAMR_LOG_LEVEL=1 product-mini/platforms/linux/build/iwasm test.wasm"

# Errors and warnings
scripts/in-container.sh "WAMR_LOG_LEVEL=2 product-mini/platforms/linux/build/iwasm test.wasm"

# Info messages (default in debug builds)
scripts/in-container.sh "WAMR_LOG_LEVEL=3 product-mini/platforms/linux/build/iwasm test.wasm"

# Verbose/debug messages
scripts/in-container.sh "WAMR_LOG_LEVEL=4 product-mini/platforms/linux/build/iwasm test.wasm"

# Very verbose (trace level)
scripts/in-container.sh "WAMR_LOG_LEVEL=5 product-mini/platforms/linux/build/iwasm test.wasm"
```

**Log level meanings:**

| Level | Name | Description |
|-------|------|-------------|
| 0 | NONE | No logging |
| 1 | FATAL | Fatal errors (crash imminent) |
| 2 | ERROR | Errors and warnings |
| 3 | INFO | Informational messages |
| 4 | DEBUG | Debug messages |
| 5 | VERBOSE | Trace-level verbosity |

### Log Output

**Example verbose output:**
```bash
scripts/in-container.sh "WAMR_LOG_LEVEL=4 product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

```text
[INFO] wasm_runtime_init
[DEBUG] wasm_runtime_init: allocate global data
[DEBUG] wasm_runtime_init: initialize global data
[INFO] wasm_runtime_load: loading module
[DEBUG] load_from_sections: parsing sections
[DEBUG] load_import_section: 5 imports
[DEBUG] load_function_section: 20 functions
[DEBUG] load_table_section: 1 tables
[DEBUG] load_memory_section: 1 memories
[DEBUG] load_export_section: 10 exports
[INFO] wasm_runtime_instantiate: instantiating module
[DEBUG] wasm_runtime_instantiate: allocating memory
[DEBUG] wasm_runtime_instantiate: initializing memory
[INFO] wasm_runtime_call_wasm: calling function "main"
[DEBUG] wasm_interp_call_func_bytecode: entering function
Hello from WASM!
[INFO] wasm_runtime_deinstantiate
[INFO] wasm_runtime_unload
[INFO] wasm_runtime_destroy
```

### Debug Assertions

Enable runtime assertions (debug builds only):

```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build-debug && cd build-debug && cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS='-UNDEBUG' && cmake --build . -j\$(nproc)"
```

Assertions check internal invariants:
- Module structure validity
- Execution environment consistency
- Memory bounds
- Stack pointer validity

**When assertion fails:**
```text
Assertion failed: module != NULL (wasm_runtime.c: wasm_runtime_instantiate: 123)
```

The message shows:
- Failed condition: `module != NULL`
- Source file: `wasm_runtime.c`
- Function: `wasm_runtime_instantiate`
- Line: `123`

---

## Troubleshooting

### GDB Shows No Debug Symbols

**Problem:** GDB shows "No debugging symbols found".

**Solution:** Rebuild with debug symbols:

```bash
scripts/in-container.sh "cd product-mini/platforms/linux/build-debug && cmake .. -DCMAKE_BUILD_TYPE=Debug && cmake --build . --clean-first -j\$(nproc)"
```

**Verify debug info:**
```bash
scripts/in-container.sh "file product-mini/platforms/linux/build-debug/iwasm"
# Should show "with debug_info, not stripped"

scripts/in-container.sh "objdump -h product-mini/platforms/linux/build-debug/iwasm | grep debug"
# Should show .debug_* sections
```

### Valgrind Reports False Positives

**Problem:** Valgrind reports leaks or errors in third-party libraries.

**Solution:** Use suppression file:

1. **Generate suppressions:**
```bash
scripts/in-container.sh "valgrind --leak-check=full --gen-suppressions=all product-mini/platforms/linux/build-debug/iwasm test.wasm 2>&1 > valgrind-full.log"
```

2. **Extract suppression entries:**
```bash
scripts/in-container.sh "grep -A 10 '^{' valgrind-full.log > valgrind.supp"
```

3. **Use suppression file:**
```bash
scripts/in-container.sh "valgrind --suppressions=valgrind.supp --leak-check=full product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

### Debugging Optimized Code

**Problem:** Optimized code is hard to debug (variables optimized away, execution jumps).

**Solution:** Use RelWithDebInfo or disable specific optimizations:

```bash
# RelWithDebInfo (moderate optimization)
scripts/in-container.sh "cd product-mini/platforms/linux/build && cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo && cmake --build . -j\$(nproc)"

# Or disable optimization for specific file
scripts/in-container.sh "cd product-mini/platforms/linux/build && cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS_DEBUG='-g -O0' && cmake --build . -j\$(nproc)"
```

**In GDB, try:**
```gdb
# Print optimized-out variable
(gdb) print variable
# If "optimized out", try:
(gdb) print $rax  # Check register
(gdb) x/xg $rbp-0x18  # Check stack location
```

### Container Doesn't Have Debug Capabilities

**Problem:** Valgrind fails with permission errors.

**Solution:** Verify container has proper capabilities:

```bash
# Check container capabilities
scripts/in-container.sh "cat /proc/self/status | grep Cap"

# Devcontainer should have:
# - SYS_PTRACE capability (for GDB)
# - seccomp=unconfined (for Valgrind)
```

If missing, rebuild devcontainer:
1. Open project in VS Code
2. Press `F1` → "Dev Containers: Rebuild Container"

### Debugging Hangs or Loops Forever

**Problem:** Program hangs and GDB doesn't show where.

**Solution:** Interrupt and examine state:

```bash
scripts/in-container.sh "gdb --args product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

```gdb
(gdb) run
# Wait for hang, then Ctrl-C
(gdb) backtrace
(gdb) info threads
(gdb) thread apply all backtrace
```

**For infinite loops, sample periodically:**
```gdb
(gdb) run &
(gdb) shell sleep 1 && kill -INT $!
(gdb) backtrace
(gdb) continue &
(gdb) shell sleep 1 && kill -INT $!
(gdb) backtrace
# Repeat to see if same location
```

### Core Dump Not Generated

**Problem:** Program crashes but no core file appears.

**Solution:** Check core dump configuration:

```bash
# Check core dump limit
scripts/in-container.sh "ulimit -c"
# Should show "unlimited"

# Enable core dumps
scripts/in-container.sh "ulimit -c unlimited"

# Check core pattern
scripts/in-container.sh "cat /proc/sys/kernel/core_pattern"
# Should show "core" or "core.%p"

# If set to pipe (e.g., "| /usr/share/apport/apport..."), core dumps may be elsewhere
# Check /var/crash/ or /var/lib/systemd/coredump/
```

---

## Reference

### GDB Cheat Sheet

```gdb
# Breakpoints
break function_name
break file.c:123
break *0x401234
delete 1
disable 1
enable 1

# Execution
run [args]
continue
next
step
finish
until
kill

# Inspection
backtrace [full]
frame N
info registers
info locals
info args
print variable
print *pointer
print array[0]@10
x/16xb address

# Watchpoints
watch variable
rwatch variable
awatch variable
watch *0x401234

# Threads
info threads
thread N
thread apply all backtrace

# Core dumps
gdb program core
```

### Valgrind Tools

| Tool | Purpose | Command |
|------|---------|---------|
| Memcheck | Memory errors and leaks | `valgrind --tool=memcheck` (default) |
| Callgrind | Call graph profiling | `valgrind --tool=callgrind` |
| Cachegrind | Cache simulation | `valgrind --tool=cachegrind` |
| Helgrind | Thread error detection | `valgrind --tool=helgrind` |
| Massif | Heap profiler | `valgrind --tool=massif` |

### Common WAMR Breakpoints

**Module loading:**
- `wasm_load` - Entry point
- `load_from_sections` - Section parsing
- `wasm_loader_prepare_bytecode` - Bytecode prep

**Instantiation:**
- `wasm_runtime_instantiate` - Create instance
- `wasm_instantiate` - Internal instantiation

**Execution:**
- `wasm_runtime_call_wasm` - Call entry point
- `wasm_interp_call_wasm` - Interpreter dispatch
- `wasm_interp_call_func_bytecode` - Bytecode execution
- `aot_call_function` - AOT execution

**Memory:**
- `wasm_runtime_malloc` - Allocate
- `wasm_runtime_free` - Free
- `wasm_enlarge_memory` - Grow memory

**Errors:**
- `wasm_set_exception` - Set exception
- `wasm_runtime_set_exception` - Set runtime exception

### External Documentation

- [GDB Documentation](https://sourceware.org/gdb/documentation/)
- [Valgrind User Manual](https://valgrind.org/docs/manual/manual.html)
- [WAMR Source-Level Debugging](source_debugging.md)
- [WAMR Architecture Overview](architecture-overview.md)

---

**Documentation Version**: 1.0.0  
**Last Updated**: 2026-04-03  
**Maintained By**: WAMR Development Team
