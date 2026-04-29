# Building WAMR

This guide explains WAMR's build system architecture, when to use different build configurations, and how to make build decisions. For detailed commands and all build options, see the platform-specific operational guides.

WAMR consists of two main components:
- **iwasm** - The WebAssembly runtime that executes WASM/AOT modules
- **wamrc** - The AOT (Ahead-of-Time) compiler that compiles WASM to native code

---

## Prerequisites

Before building WAMR:

1. **Read [AGENTS.md](../AGENTS.md)** - Platform-specific execution requirements
2. **Read [dev-in-container.md](dev-in-container.md)** - Container technical details
3. All build tools are pre-installed in the devcontainer (CMake, GCC, Clang, LLVM)

> **Note**: All commands in this guide show raw syntax. See [AGENTS.md](../AGENTS.md) for platform-specific execution.

---

## Quick Start

Get a working iwasm binary in under 60 seconds:

```bash
cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. && cmake --build . -j\$(nproc)
```

Binary location: `product-mini/platforms/linux/build/iwasm`

See [product-mini/platforms/linux/README.md](../product-mini/platforms/linux/README.md) for detailed build instructions.

---

## Build Types

### What Are Build Types?

WAMR supports different CMake build types that control optimization levels and debug information.

### Build Types Explained

| Build Type | Purpose | When to Use |
|------------|---------|-------------|
| **Release** (default) | Optimized for performance (-O3) | Production, performance testing |
| **Debug** | Debug symbols, no optimization (-O0, -g) | Debugging WAMR or WASM modules with GDB |
| **RelWithDebInfo** | Optimized with debug symbols (-O2, -g) | Performance profiling with debugger |
| **MinSizeRel** | Optimized for size (-Os) | Embedded systems, size-constrained environments |

### Quick Example

```bash
# Debug build
cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug && cmake --build . -j\$(nproc)
```

See [product-mini/platforms/linux/README.md](../product-mini/platforms/linux/README.md) for all build type configurations.

---

## Execution Modes

### What Are Execution Modes?

WAMR supports multiple ways to execute WebAssembly code, each with different performance and startup characteristics.

### Execution Modes Comparison

| Mode | Startup Speed | Peak Performance | Memory Usage | Build Complexity |
|------|---------------|------------------|--------------|------------------|
| **Classic Interpreter** | Fastest | Baseline (1x) | Lowest | Simple |
| **Fast Interpreter** | Fast | 2-3x | Low | Simple |
| **AOT** | N/A (pre-compiled) | 5-10x | Low | Requires wamrc |
| **Fast JIT** | Fast | 2-5x | Medium | Simple |
| **LLVM JIT** | Slow | 8-12x | High | Requires LLVM build |
| **Multi-tier JIT** | Fast | 8-12x (after warmup) | High | Requires LLVM build |

### When to Use Each Mode

**Classic Interpreter**:
- Minimal memory footprint needed
- Simplest possible build
- Embedded systems with size constraints

**Fast Interpreter** (default):
- General purpose usage
- Development and testing
- Good balance of speed and simplicity

**AOT**:
- Maximum performance required
- Can pre-compile offline
- Production deployments

**Fast JIT**:
- Better performance than interpreter
- Don't want AOT compilation step
- Quick prototyping

**LLVM JIT**:
- Maximum performance without pre-compilation
- Long-running applications
- Server-side workloads

**Multi-tier JIT**:
- Need fast startup AND peak performance
- Production servers with varied workloads
- Best of both worlds

### Quick Examples

```bash
# Fast Interpreter (default)
cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. -DWAMR_BUILD_INTERP=1 && cmake --build .

# AOT (runtime only - requires wamrc to compile wasm to aot)
cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. -DWAMR_BUILD_AOT=1 && cmake --build .

# LLVM JIT (requires LLVM build first)
cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. -DWAMR_BUILD_JIT=1 && cmake --build .
```

See [product-mini/platforms/linux/README.md](../product-mini/platforms/linux/README.md) for detailed mode configurations and examples.

---

## Feature Flags

### What Are Feature Flags?

WAMR uses CMake variables to enable/disable runtime features and WebAssembly proposals. Features are organized into categories.

### Feature Categories

**Core Execution** (mode selection):
- `WAMR_BUILD_INTERP` - Classic/Fast interpreter
- `WAMR_BUILD_AOT` - AOT runtime support
- `WAMR_BUILD_JIT` - LLVM JIT compilation
- `WAMR_BUILD_FAST_JIT` - Fast JIT compilation

**Standard Library**:
- `WAMR_BUILD_LIBC_BUILTIN` - Minimal built-in libc
- `WAMR_BUILD_LIBC_WASI` - WASI system interface

**WebAssembly Proposals**:
- `WAMR_BUILD_SIMD` - 128-bit SIMD instructions
- `WAMR_BUILD_BULK_MEMORY` - Bulk memory operations
- `WAMR_BUILD_REF_TYPES` - Reference types
- `WAMR_BUILD_TAIL_CALL` - Tail call optimization
- `WAMR_BUILD_MULTI_MODULE` - Module linking
- `WAMR_BUILD_GC` - Garbage collection

**Threading**:
- `WAMR_BUILD_LIB_PTHREAD` - POSIX threads
- `WAMR_BUILD_SHARED_MEMORY` - Shared memory with atomics

**Debugging**:
- `WAMR_BUILD_DEBUG_INTERP` - Interpreter debugging
- `WAMR_BUILD_DUMP_CALL_STACK` - Call stack on errors

### Decision Guide for Common Scenarios

| Scenario | Recommended Flags |
|----------|------------------|
| **Development/Testing** | INTERP=1, AOT=1, FAST_INTERP=1, LIBC_WASI=1 |
| **Production (Performance)** | AOT=1, SIMD=1, BULK_MEMORY=1 |
| **Embedded (Size)** | INTERP=1, LIBC_BUILTIN=1, FAST_INTERP=0 |
| **Debugging** | INTERP=1, DEBUG_INTERP=1, DUMP_CALL_STACK=1 |
| **Modern Wasm** | SIMD=1, BULK_MEMORY=1, REF_TYPES=1 |

### Quick Example

```bash
# Enable SIMD and bulk memory
cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. -DWAMR_BUILD_SIMD=1 -DWAMR_BUILD_BULK_MEMORY=1 && cmake --build .
```

See [doc/build_wamr.md](build_wamr.md) for complete CMake flag reference (all 100+ options).

---

## Platform-Specific Builds

### Supported Platforms

WAMR supports building for multiple platforms and architectures. The build process varies by platform.

| Platform | Status | Platform Directory |
|----------|--------|-------------------|
| **Linux** | Production | `product-mini/platforms/linux/` |
| **macOS** | Production | `product-mini/platforms/darwin/` |
| **Windows** | Production | `product-mini/platforms/windows/` |
| **Android** | Production | `product-mini/platforms/android/` |
| **Zephyr** | Production | `product-mini/platforms/zephyr/` |
| **ESP-IDF** | Production | `product-mini/platforms/esp-idf/` |
| **VxWorks** | Supported | `product-mini/platforms/vxworks/` |
| **Linux SGX** | Supported | `product-mini/platforms/linux-sgx/` |
| **NuttX** | Supported | `product-mini/platforms/nuttx/` |
| **RT-Thread** | Supported | `product-mini/platforms/rt-thread/` |

### When to Use Each Platform

**Linux**: General development, servers, testing, CI/CD
**macOS**: Development on Mac hardware
**Windows**: Windows-based development and deployment
**Android**: Mobile applications
**Zephyr/ESP-IDF**: IoT and embedded devices
**Linux SGX**: Trusted execution environments
**RTOS platforms**: Real-time embedded systems

### Cross-Compilation

To build for a different architecture, specify the target:

```bash
cd product-mini/platforms/linux && mkdir -p build && cd build && cmake .. -DWAMR_BUILD_TARGET=AARCH64 && cmake --build .
```

**Supported architectures**: X86_64, X86_32, AARCH64, ARM, ARMV7, RISCV64, RISCV32, MIPS, XTENSA

See platform-specific READMEs for detailed instructions:
- Linux: [product-mini/platforms/linux/README.md](../product-mini/platforms/linux/README.md) (to be created)
- All platforms: [product-mini/README.md](../product-mini/README.md)

---

## Building wamrc AOT Compiler

### What is wamrc?

wamrc is WAMR's AOT (Ahead-of-Time) compiler that converts WASM modules to native machine code for maximum runtime performance.

### When to Use wamrc

**Use wamrc when:**
- Maximum performance is required (5-10x faster than interpreter)
- You can pre-compile offline before deployment
- Running on production systems
- Workload is CPU-intensive

**Don't use wamrc when:**
- Dynamic code loading is needed
- Build pipeline complexity is a concern
- Interpreter performance is sufficient

### Quick Example

```bash
# Build LLVM (one-time setup)
cd wamr-compiler && ./build_llvm.sh

# Build wamrc
cd wamr-compiler && mkdir -p build && cd build && cmake .. && cmake --build . -j\$(nproc)

# Compile WASM to AOT
wamr-compiler/build/wamrc -o app.aot app.wasm

# Run AOT file with iwasm
product-mini/platforms/linux/build/iwasm app.aot
```

See [wamr-compiler/README.md](../wamr-compiler/README.md) for complete wamrc documentation and all compiler options.

---

## Build Configuration Decision Guide

Use this table to choose the right build configuration for your use case:

| Use Case | Build Type | Execution Mode | Key Flags | Trade-offs |
|----------|------------|----------------|-----------|------------|
| **Quick Testing** | Release | Fast Interpreter | `INTERP=1, FAST_INTERP=1` | Fast build, moderate performance |
| **Production (Max Performance)** | Release | AOT | `AOT=1, SIMD=1` | Requires pre-compilation, best performance |
| **Production (Balanced)** | Release | Multi-tier JIT | `FAST_JIT=1, JIT=1` | Fast startup + peak performance, large binary |
| **Debugging WAMR** | Debug | Interpreter | `DEBUG_INTERP=1, DUMP_CALL_STACK=1` | Slow, easy to debug |
| **Embedded (Size)** | MinSizeRel | Classic Interpreter | `INTERP=1, FAST_INTERP=0, LIBC_BUILTIN=1` | Smallest binary, slowest |
| **Embedded (Speed)** | Release | AOT | `AOT=1, MINI_LOADER=1` | Pre-compile required, fast |
| **Development** | RelWithDebInfo | Fast Interpreter + AOT | `INTERP=1, AOT=1` | Good balance for dev |
| **Modern WebAssembly** | Release | Fast Interpreter | `SIMD=1, BULK_MEMORY=1, REF_TYPES=1` | Full WASM feature support |

### Decision Tree

```
What's your priority?
├─ Performance
│  ├─ Can pre-compile? → Use AOT
│  └─ Need dynamic loading? → Use Multi-tier JIT
├─ Size
│  ├─ Minimal footprint? → Classic Interpreter + MinSizeRel
│  └─ Balanced? → Fast Interpreter + Release
├─ Debugging
│  └─ Use Debug build + DEBUG_INTERP=1
└─ Development
   └─ Use Fast Interpreter + AOT (both modes)
```

---

## Common Build Issues

### Feature Dependencies

Some CMake flags require others to be enabled:

| Flag | Requires | Reason |
|------|----------|--------|
| `WAMR_BUILD_FAST_JIT=1` | `WAMR_BUILD_INTERP=1` | Fast JIT needs interpreter fallback |
| `WAMR_BUILD_LIB_PTHREAD=1` | `WAMR_BUILD_THREAD_MGR=1` | Pthread needs thread manager |
| `WAMR_BUILD_DEBUG_INTERP=1` | `WAMR_BUILD_INTERP=1` | Debug mode for interpreter only |
| `WAMR_BUILD_LAZY_JIT=1` | `WAMR_BUILD_JIT=1` | Lazy compilation is JIT feature |

### LLVM Build Issues

**Problem**: LLVM build takes too long or fails

**Solutions**:
- Use ccache: `build_llvm.py --use-ccache`
- LLVM only needs to be built once (cached for future builds)
- Pre-built LLVM can be used (see build_wamr.md)

**Problem**: CMake can't find LLVM

**Solution**: Ensure LLVM is built before configuring iwasm with JIT enabled

See [product-mini/platforms/linux/README.md](../product-mini/platforms/linux/README.md) for detailed troubleshooting.

---

## Reference

### Documentation Hierarchy

**This document (building.md)**: Strategy layer - concepts, decisions, when/why

**Operational guides** (complete commands and options):
- [doc/build_wamr.md](build_wamr.md) - Complete CMake flag reference (100+ options)
- [product-mini/README.md](../product-mini/README.md) - Platform build instructions
- [product-mini/platforms/linux/README.md](../product-mini/platforms/linux/README.md) - Linux operational guide (to be created)
- [wamr-compiler/README.md](../wamr-compiler/README.md) - AOT compiler details
- [dev-in-container.md](dev-in-container.md) - Devcontainer setup

### Build Locations

| Component | Build Directory | Binary Output |
|-----------|-----------------|---------------|
| iwasm (Linux) | `product-mini/platforms/linux/build/` | `iwasm` |
| wamrc | `wamr-compiler/build/` | `wamrc` |

### Default Configuration (Linux)

Default flags when running `cmake ..` without options:
- Interpreter: Fast interpreter enabled
- AOT: Enabled (runtime support)
- JIT: Disabled
- Libc: Both WASI and builtin enabled
- Build type: Release

### External Resources

- [WAMR GitHub Repository](https://github.com/bytecodealliance/wasm-micro-runtime)
- [Blog: Introduction to WAMR Running Modes](https://bytecodealliance.github.io/wamr.dev/blog/introduction-to-wamr-running-modes/)
- [WebAssembly Proposals](https://github.com/WebAssembly/proposals)
- [WASI Documentation](https://wasi.dev/)

---

**Documentation Version**: 2.0.0  
**Last Updated**: 2026-04-04  
**Maintained By**: WAMR Development Team
