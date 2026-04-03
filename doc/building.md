# Building WAMR

This guide shows how to build WAMR components inside the devcontainer environment. WAMR consists of two main components:

- **iwasm** - The WebAssembly runtime that executes WASM/AOT modules
- **wamrc** - The AOT (Ahead-of-Time) compiler that compiles WASM to native code

---

## Prerequisites

Before building WAMR, ensure you have the devcontainer environment set up:

1. **Read [dev-in-container.md](dev-in-container.md)** for complete devcontainer setup
2. **Verify container is available**: Run `scripts/in-container.sh --status`
3. **All build tools are pre-installed** in the devcontainer (CMake, GCC, Clang, LLVM)

---

## For AI Agents

**CRITICAL**: All build commands MUST run inside the devcontainer using the `scripts/in-container.sh` wrapper script.

**Pattern for all build commands:**
```bash
scripts/in-container.sh "<command>"
```

The script automatically:
- Detects or starts the devcontainer
- Executes commands in the correct environment
- Returns proper exit codes for error handling

**Never run build commands directly on the host.** The devcontainer provides consistent toolchains and dependencies.

---

## Quick Start

Get a working iwasm binary in under 60 seconds:

```bash
# Configure and build basic iwasm with interpreter
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. -DWAMR_BUILD_INTERP=1 && cmake --build . -j\$(nproc)"

# Binary location (inside container)
# /workspaces/ai-thoughts/product-mini/platforms/linux/build/iwasm

# Test it works
scripts/in-container.sh "product-mini/platforms/linux/build/iwasm --version"
```

**Output location on host:** `product-mini/platforms/linux/build/iwasm` (files sync bidirectionally)

---

## Building iwasm Runtime

The iwasm runtime is the executable that runs WebAssembly modules. It supports multiple execution modes and features.

### Basic Interpreter Build

Smallest, simplest build with just the fast interpreter:

```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. -DWAMR_BUILD_INTERP=1 -DWAMR_BUILD_AOT=0 && cmake --build . -j\$(nproc)"
```

**Features enabled:**
- Fast interpreter (default)
- WASI libc support
- Builtin libc support

**Binary location:** `product-mini/platforms/linux/build/iwasm`

**Use case:** Quick testing, minimal footprint, embedded systems

---

### Full-Featured Build

Build with all major features enabled (recommended for development):

```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build-full && cd build-full && cmake .. \
  -DWAMR_BUILD_INTERP=1 \
  -DWAMR_BUILD_AOT=1 \
  -DWAMR_BUILD_JIT=1 \
  -DWAMR_BUILD_LIBC_WASI=1 \
  -DWAMR_BUILD_LIBC_BUILTIN=1 \
  -DWAMR_BUILD_FAST_INTERP=1 \
  -DWAMR_BUILD_MULTI_MODULE=1 \
  -DWAMR_BUILD_LIB_PTHREAD=1 \
  -DWAMR_BUILD_SIMD=1 \
  -DWAMR_BUILD_TAIL_CALL=1 \
  -DWAMR_BUILD_REF_TYPES=1 \
  -DWAMR_BUILD_BULK_MEMORY=1 && \
  cmake --build . -j\$(nproc)"
```

**Features enabled:**
- Fast interpreter + AOT + LLVM JIT
- WASI + Builtin libc
- Multi-module support
- Pthread support
- SIMD (128-bit)
- Tail call optimization
- Reference types
- Bulk memory operations

**Binary location:** `product-mini/platforms/linux/build-full/iwasm`

**Use case:** Maximum compatibility, full feature set for testing

**Note:** LLVM JIT requires LLVM libraries. See [Building with LLVM JIT](#building-with-llvm-jit) section.

---

### Debug Build

Build with debug symbols and optimizations disabled for debugging:

```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build-debug && cd build-debug && cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DWAMR_BUILD_INTERP=1 \
  -DWAMR_BUILD_DUMP_CALL_STACK=1 \
  -DWAMR_BUILD_DEBUG_INTERP=1 && \
  cmake --build . -j\$(nproc)"
```

**Features enabled:**
- Debug symbols (-g)
- No optimizations (-O0)
- Call stack dumping
- Source-level debugging support

**Binary location:** `product-mini/platforms/linux/build-debug/iwasm`

**Use case:** Debugging WAMR itself or WASM modules with GDB/Valgrind

**Run under GDB:**
```bash
scripts/in-container.sh "gdb --args product-mini/platforms/linux/build-debug/iwasm test.wasm"
```

---

### Fast JIT Build

Enable Fast JIT for better performance than interpreter:

```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build-fastjit && cd build-fastjit && cmake .. \
  -DWAMR_BUILD_FAST_JIT=1 \
  -DWAMR_BUILD_INTERP=1 && \
  cmake --build . -j\$(nproc)"
```

**Features:**
- Fast JIT (lightweight JIT with quick startup)
- Fallback to interpreter for unsupported instructions
- ~50% of AOT performance with faster cold start

**Supported architectures:** x86_64, ARM, AArch64 (limited)

**Binary location:** `product-mini/platforms/linux/build-fastjit/iwasm`

**Use case:** Better performance than interpreter without AOT compilation step

---

### Multi-Tier JIT Build

Combine Fast JIT + LLVM JIT for best of both worlds:

```bash
# First build LLVM libraries (one-time setup)
scripts/in-container.sh "cd product-mini/platforms/linux && ./build_llvm.sh"

# Then build iwasm with both JIT engines
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build-multi-jit && cd build-multi-jit && cmake .. \
  -DWAMR_BUILD_FAST_JIT=1 \
  -DWAMR_BUILD_JIT=1 && \
  cmake --build . -j\$(nproc)"
```

**Features:**
- Fast JIT starts execution immediately
- LLVM JIT compiles functions in background
- Automatic tier-up from Fast JIT to LLVM JIT
- Best cold-start + best peak performance

**Binary location:** `product-mini/platforms/linux/build-multi-jit/iwasm`

**Use case:** Production deployments needing both fast startup and optimal throughput

---

### Building with LLVM JIT

LLVM JIT provides the best performance but requires building LLVM libraries first.

**Step 1: Build LLVM libraries (one-time, takes 15-30 minutes)**

```bash
# Build LLVM for x86_64
scripts/in-container.sh "cd product-mini/platforms/linux && ./build_llvm.sh"

# Or use Python script with ccache for faster rebuilds
scripts/in-container.sh "cd build-scripts && python3 build_llvm.py --arch X86 --use-ccache"
```

**LLVM is cached:** Subsequent builds of iwasm with LLVM JIT are fast. You only build LLVM once.

**Step 2: Build iwasm with LLVM JIT enabled**

```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build-jit && cd build-jit && cmake .. \
  -DWAMR_BUILD_JIT=1 \
  -DWAMR_BUILD_LAZY_JIT=1 && \
  cmake --build . -j\$(nproc)"
```

**Lazy JIT (recommended):** Functions compiled on-demand in background threads for faster startup.

**Eager JIT:** Set `-DWAMR_BUILD_LAZY_JIT=0` to compile all functions upfront (slower startup, faster first execution).

**Binary location:** `product-mini/platforms/linux/build-jit/iwasm`

---

## Building wamrc AOT Compiler

The wamrc compiler converts WASM modules to native AOT (Ahead-of-Time) code for maximum performance.

### Build wamrc

```bash
# First, ensure LLVM is built (same as for LLVM JIT)
scripts/in-container.sh "cd wamr-compiler && ./build_llvm.sh"

# Build wamrc
scripts/in-container.sh "cd wamr-compiler && mkdir -p build && cd build && cmake .. && cmake --build . -j\$(nproc)"
```

**Binary location:** `wamr-compiler/build/wamrc`

**Verify it works:**
```bash
scripts/in-container.sh "wamr-compiler/build/wamrc --version"
```

### Using wamrc

Compile a WASM module to AOT:

```bash
scripts/in-container.sh "wamr-compiler/build/wamrc -o app.aot app.wasm"
```

**Common wamrc options:**

```bash
# Target specific architecture
scripts/in-container.sh "wamr-compiler/build/wamrc --target=aarch64 -o app.aot app.wasm"

# Optimization levels
scripts/in-container.sh "wamr-compiler/build/wamrc -o app.aot --opt-level=3 app.wasm"

# Enable SIMD
scripts/in-container.sh "wamr-compiler/build/wamrc -o app.aot --enable-simd app.wasm"

# Size optimization
scripts/in-container.sh "wamr-compiler/build/wamrc -o app.aot --size-level=3 app.wasm"
```

### Run AOT Files

AOT files are executed by iwasm just like WASM files:

```bash
# Build iwasm with AOT support
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. -DWAMR_BUILD_AOT=1 && cmake --build . -j\$(nproc)"

# Run the AOT file
scripts/in-container.sh "product-mini/platforms/linux/build/iwasm app.aot"
```

**Performance:** AOT files typically run 5-10x faster than interpreter mode.

---

## Common Build Options

Complete reference of CMake options for configuring WAMR builds.

### Platform and Architecture

| Option | Values | Description |
|--------|--------|-------------|
| `WAMR_BUILD_PLATFORM` | `linux`, `darwin`, `windows`, etc. | Target platform (default: auto-detected) |
| `WAMR_BUILD_TARGET` | `X86_64`, `X86_32`, `AARCH64`, `ARM`, `RISCV64`, etc. | Target CPU architecture (default: auto-detected) |

**Example:**
```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. -DWAMR_BUILD_PLATFORM=linux -DWAMR_BUILD_TARGET=X86_64 && cmake --build . -j\$(nproc)"
```

### Execution Modes

| Option | Values | Default | Description |
|--------|--------|---------|-------------|
| `WAMR_BUILD_INTERP` | 1/0 | 1 | Enable interpreter mode |
| `WAMR_BUILD_FAST_INTERP` | 1/0 | 1 | Use fast interpreter (2x faster, 2x memory) |
| `WAMR_BUILD_AOT` | 1/0 | 1 | Enable AOT mode (run pre-compiled modules) |
| `WAMR_BUILD_JIT` | 1/0 | 0 | Enable LLVM JIT compiler |
| `WAMR_BUILD_LAZY_JIT` | 1/0 | 0 | Enable lazy compilation for LLVM JIT |
| `WAMR_BUILD_FAST_JIT` | 1/0 | 0 | Enable Fast JIT (lightweight) |

**Example - Interpreter only:**
```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. -DWAMR_BUILD_INTERP=1 -DWAMR_BUILD_AOT=0 && cmake --build . -j\$(nproc)"
```

**Example - AOT + Fast JIT:**
```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. -DWAMR_BUILD_INTERP=1 -DWAMR_BUILD_AOT=1 -DWAMR_BUILD_FAST_JIT=1 && cmake --build . -j\$(nproc)"
```

### Libc Support

| Option | Values | Default | Description |
|--------|--------|---------|-------------|
| `WAMR_BUILD_LIBC_BUILTIN` | 1/0 | 1 | Built-in minimal libc |
| `WAMR_BUILD_LIBC_WASI` | 1/0 | 1 | WASI libc (system calls, file I/O) |
| `WAMR_BUILD_LIBC_UVWASI` | 1/0 | 0 | UVWASI implementation (experimental) |
| `WAMR_BUILD_LIBC_EMCC` | 1/0 | 0 | Emscripten libc compatibility |

**Example - WASI only:**
```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. -DWAMR_BUILD_LIBC_WASI=1 -DWAMR_BUILD_LIBC_BUILTIN=0 && cmake --build . -j\$(nproc)"
```

### WebAssembly Features

| Option | Values | Default | Description |
|--------|--------|---------|-------------|
| `WAMR_BUILD_SIMD` | 1/0 | 0 | 128-bit SIMD instructions |
| `WAMR_BUILD_REF_TYPES` | 1/0 | 0 | Reference types proposal |
| `WAMR_BUILD_BULK_MEMORY` | 1/0 | 0 | Bulk memory operations |
| `WAMR_BUILD_SHARED_MEMORY` | 1/0 | 0 | Shared memory (atomics) |
| `WAMR_BUILD_TAIL_CALL` | 1/0 | 0 | Tail call optimization |
| `WAMR_BUILD_MULTI_MODULE` | 1/0 | 0 | Multi-module linking |
| `WAMR_BUILD_MEMORY64` | 1/0 | 0 | 64-bit memory addressing |
| `WAMR_BUILD_EXCE_HANDLING` | 1/0 | 0 | Exception handling proposal |
| `WAMR_BUILD_GC` | 1/0 | 0 | Garbage collection proposal |

**Example - Enable SIMD + Bulk Memory:**
```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. -DWAMR_BUILD_SIMD=1 -DWAMR_BUILD_BULK_MEMORY=1 && cmake --build . -j\$(nproc)"
```

### Threading and Concurrency

| Option | Values | Default | Description |
|--------|--------|---------|-------------|
| `WAMR_BUILD_LIB_PTHREAD` | 1/0 | 0 | POSIX threads support |
| `WAMR_BUILD_LIB_PTHREAD_SEMAPHORE` | 1/0 | 0 | Pthread semaphores |
| `WAMR_BUILD_LIB_WASI_THREADS` | 1/0 | 0 | WASI threads |
| `WAMR_BUILD_THREAD_MGR` | 1/0 | 0 | Thread manager |

**Example - Enable pthread:**
```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. -DWAMR_BUILD_LIB_PTHREAD=1 -DWAMR_BUILD_THREAD_MGR=1 && cmake --build . -j\$(nproc)"
```

### Debugging and Profiling

| Option | Values | Default | Description |
|--------|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | `Debug`, `Release`, `RelWithDebInfo` | `Release` | Build type with debug info |
| `WAMR_BUILD_DEBUG_INTERP` | 1/0 | 0 | Enable interpreter debugging |
| `WAMR_BUILD_DEBUG_AOT` | 1/0 | 0 | Enable AOT debugging |
| `WAMR_BUILD_DUMP_CALL_STACK` | 1/0 | 0 | Dump call stack on errors |
| `WAMR_BUILD_MEMORY_PROFILING` | 1/0 | 0 | Memory usage profiling |
| `WAMR_BUILD_PERF_PROFILING` | 1/0 | 0 | Performance profiling |
| `WAMR_BUILD_LINUX_PERF` | 1/0 | 0 | Linux perf integration |

**Example - Debug build with call stack:**
```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_DUMP_CALL_STACK=1 && cmake --build . -j\$(nproc)"
```

### Optimization and Performance

| Option | Values | Default | Description |
|--------|--------|---------|-------------|
| `WAMR_BUILD_MINI_LOADER` | 1/0 | 0 | Minimal WASM loader (smaller footprint) |
| `WAMR_BUILD_QUICK_AOT_ENTRY` | 1/0 | 0 | Fast AOT function calls |
| `WAMR_DISABLE_HW_BOUND_CHECK` | 1/0 | 0 | Disable hardware memory bounds checking |
| `WAMR_BUILD_WASM_CACHE` | 1/0 | 0 | Cache compiled code |

**Warning:** Some options reduce security. Review [build_wamr.md](build_wamr.md) for details.

### Advanced Features

| Option | Values | Default | Description |
|--------|--------|---------|-------------|
| `WAMR_BUILD_WASI_NN` | 1/0 | 0 | WASI Neural Network API |
| `WAMR_BUILD_CUSTOM_NAME_SECTION` | 1/0 | 0 | Load custom name sections |
| `WAMR_BUILD_LOAD_CUSTOM_SECTION` | 1/0 | 0 | Load custom sections |
| `WAMR_BUILD_SHARED_HEAP` | 1/0 | 0 | Shared heap between modules and host |
| `WAMR_BUILD_SANITIZER` | 1/0 | 0 | Enable sanitizers (ASan, UBSan) |

**Example - Enable WASI-NN:**
```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. -DWAMR_BUILD_WASI_NN=1 && cmake --build . -j\$(nproc)"
```

---

## Build Targets

### Build Specific Target

By default, `cmake --build .` builds all targets. To build only iwasm:

```bash
scripts/in-container.sh "cd product-mini/platforms/linux/build && cmake --build . --target iwasm -j\$(nproc)"
```

### Clean Build

Remove build artifacts and rebuild:

```bash
scripts/in-container.sh "cd product-mini/platforms/linux && rm -rf build && mkdir build && cd build && cmake .. && cmake --build . -j\$(nproc)"
```

Or clean within build directory:

```bash
scripts/in-container.sh "cd product-mini/platforms/linux/build && cmake --build . --target clean && cmake --build . -j\$(nproc)"
```

### Parallel Builds

Use `$(nproc)` to build with all available CPU cores (much faster):

```bash
scripts/in-container.sh "cd product-mini/platforms/linux/build && cmake --build . -j\$(nproc)"
```

Or specify core count explicitly:

```bash
scripts/in-container.sh "cd product-mini/platforms/linux/build && cmake --build . -j8"
```

---

## Verifying Build

### Check Binary Exists

```bash
# Check iwasm was built
scripts/in-container.sh "ls -lh product-mini/platforms/linux/build/iwasm"

# Check wamrc was built
scripts/in-container.sh "ls -lh wamr-compiler/build/wamrc"
```

### Test Runtime

```bash
# Check iwasm version
scripts/in-container.sh "product-mini/platforms/linux/build/iwasm --version"

# Check wamrc version
scripts/in-container.sh "wamr-compiler/build/wamrc --version"
```

### Verify Features

Check which features are enabled in your iwasm build:

```bash
scripts/in-container.sh "product-mini/platforms/linux/build/iwasm --help"
```

Output shows enabled features like:
- AOT support
- JIT support
- WASI support
- SIMD support

### Run Simple Test

Create a simple WASM test file and run it:

```bash
# This requires WASI SDK to compile C to WASM (pre-installed in devcontainer)
scripts/in-container.sh "echo 'int main() { printf(\"Hello WAMR\\\\n\"); return 0; }' > /tmp/test.c && \
  /opt/wasi-sdk/bin/clang /tmp/test.c -o /tmp/test.wasm && \
  product-mini/platforms/linux/build/iwasm /tmp/test.wasm"
```

Expected output: `Hello WAMR`

---

## Troubleshooting

### Build Fails with "LLVM Not Found"

**Problem:** CMake can't find LLVM libraries when `WAMR_BUILD_JIT=1`.

**Solution:** Build LLVM first:

```bash
scripts/in-container.sh "cd product-mini/platforms/linux && ./build_llvm.sh"
# Then retry iwasm build
```

### Build Fails with "undefined reference to..."

**Problem:** Missing dependencies or incompatible feature combinations.

**Solution:** Check for conflicting options. Some features require others:

- `WAMR_BUILD_FAST_JIT=1` requires `WAMR_BUILD_INTERP=1`
- `WAMR_BUILD_LIB_PTHREAD=1` requires `WAMR_BUILD_THREAD_MGR=1`
- `WAMR_BUILD_DEBUG_INTERP=1` requires `WAMR_BUILD_INTERP=1`

**Reset to known good configuration:**

```bash
scripts/in-container.sh "cd product-mini/platforms/linux && rm -rf build && mkdir build && cd build && cmake .. && cmake --build . -j\$(nproc)"
```

### Binary Too Large

**Problem:** iwasm binary is larger than expected.

**Solution:** Disable unused features or use size optimization:

```bash
scripts/in-container.sh "cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DWAMR_BUILD_INTERP=1 \
  -DWAMR_BUILD_AOT=0 \
  -DWAMR_BUILD_JIT=0 \
  -DWAMR_BUILD_LIBC_BUILTIN=1 \
  -DWAMR_BUILD_LIBC_WASI=0 && \
  cmake --build . -j\$(nproc)"
```

Use `strip` to remove symbols:

```bash
scripts/in-container.sh "strip product-mini/platforms/linux/build/iwasm"
```

### Slow Build Times

**Problem:** LLVM takes too long to build.

**Solution:** Enable ccache for incremental builds:

```bash
scripts/in-container.sh "cd build-scripts && python3 build_llvm.py --arch X86 --use-ccache"
```

Or use pre-built LLVM from system packages (not recommended for production):

```bash
scripts/in-container.sh "apt-cache search llvm-dev"
```

### Container Not Found

**Problem:** `scripts/in-container.sh` reports "No container found".

**Solution:** See [dev-in-container.md Troubleshooting](dev-in-container.md#troubleshooting) for container issues.

### Permission Errors

**Problem:** Build fails with permission denied errors.

**Solution:** Check file ownership on host:

```bash
ls -la product-mini/platforms/linux/build/

# Fix ownership if needed
sudo chown -R $USER:$USER product-mini/platforms/linux/build/
```

---

## Reference

### Complete Documentation

- **[build_wamr.md](build_wamr.md)** - Complete reference for all CMake options and features
- **[product-mini/README.md](../product-mini/README.md)** - Platform-specific build instructions
- **[wamr-compiler/README.md](../wamr-compiler/README.md)** - AOT compiler build details
- **[dev-in-container.md](dev-in-container.md)** - Devcontainer setup and usage

### Build Locations

| Component | Source Path | Build Path | Binary Location |
|-----------|------------|------------|-----------------|
| iwasm (Linux) | `product-mini/platforms/linux/` | `product-mini/platforms/linux/build/` | `build/iwasm` |
| wamrc | `wamr-compiler/` | `wamr-compiler/build/` | `build/wamrc` |

### Default Settings (Linux)

When you run `cmake ..` without options in `product-mini/platforms/linux/`:

- **WAMR_BUILD_INTERP**: 1 (enabled)
- **WAMR_BUILD_FAST_INTERP**: 1 (enabled)
- **WAMR_BUILD_AOT**: 1 (enabled)
- **WAMR_BUILD_JIT**: 0 (disabled)
- **WAMR_BUILD_LIBC_BUILTIN**: 1 (enabled)
- **WAMR_BUILD_LIBC_WASI**: 1 (enabled)
- **WAMR_BUILD_TARGET**: Auto-detected (X86_64 or X86_32)
- **CMAKE_BUILD_TYPE**: Release

### External Resources

- [WAMR GitHub Repository](https://github.com/bytecodealliance/wasm-micro-runtime)
- [Introduction to WAMR Running Modes](https://bytecodealliance.github.io/wamr.dev/blog/introduction-to-wamr-running-modes/)
- [WebAssembly Proposals](https://github.com/WebAssembly/proposals)
- [WASI Documentation](https://wasi.dev/)

---

**Documentation Version**: 1.0.0  
**Last Updated**: 2026-04-03  
**Maintained By**: WAMR Development Team
