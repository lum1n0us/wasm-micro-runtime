# Performance Tuning

SSOT for performance tuning strategy.

**Prerequisites**:
1. [AGENTS.md](../AGENTS.md) - Command execution patterns
2. [build_wamr.md](./build_wamr.md) - Build configuration (SSOT)
3. [architecture-overview.md](./architecture-overview.md) - Runtime modes (SSOT)

> **Execution**: Commands in pure form. See [AGENTS.md § Command Execution Pattern](../AGENTS.md#command-execution-pattern).

## Strategy Overview

Performance optimization methods:

## 1. Optimize Wasm Binary

Use `wasm-opt` from [binaryen](https://github.com/WebAssembly/binaryen/releases):

```bash
wasm-opt -O4 -o test_opt.wasm test.wasm
```

## 2. Enable SIMD

WebAssembly [128-bit SIMD](https://github.com/WebAssembly/simd) is supported on x86-64 and aarch64. Add `-msimd128`:

```bash
clang -msimd128 -O3 -o app.wasm app.c
```

## 3. Segue Optimization (AOT)

[Segue](https://plas2022.github.io/files/pdf/SegueColorGuard.pdf) uses x86 segment register for linear memory base, reducing SFI overhead:

- Improves AOT/JIT performance
- Reduces code size
- Reduces compilation time

**Platform**: Linux x86-64 only

```bash
wamrc --enable-segue -o aot_file wasm_file
```

**Flags**: i32.load, i64.load, f32.load, f64.load, v128.load, i32.store, i64.store, f32.store, f64.store, v128.store. Use `--enable-segue=i32.load,i64.store` for selective optimization.

> Note: For CoreMark, `--enable-segue=i32.store` may outperform `--enable-segue`.

## 4. Segue Optimization (JIT)

Enable segue for LLVM-JIT mode:

```bash
iwasm --enable-segue wasm_file
```

**Platform**: Linux x86-64 only

## 5. Profile-Guided Optimization (PGO)

LLVM PGO optimizes code based on runtime behavior. Tested on Linux x86-64/x86-32.

**Workflow**:

1. Generate instrumented AOT:
```bash
wamrc --enable-llvm-pgo -o aot_pgo.aot app.wasm
```

2. Profile execution (requires `cmake -DWAMR_BUILD_STATIC_PGO=1`):
```bash
iwasm --gen-prof-file=raw.prof aot_pgo.aot
```

3. Merge profile:
```bash
llvm-profdata merge -output=merged.prof raw.prof
```

4. Generate optimized AOT:
```bash
wamrc --use-prof-file=merged.prof -o app.aot app.wasm
```

**Memory-constrained environments**: Use `wasm_runtime_get_pgo_prof_data_size()` and `wasm_runtime_dump_pgo_prof_data_to_buf()` for network export.

**Examples**: See [test_pgo.sh](../tests/benchmarks/coremark/test_pgo.sh)

## 6. Disable Bounds Checks (UNSAFE)

**WARNING**: Security risk. Only for trusted, well-tested apps on platforms without hardware trap support.

**Configuration**:
1. Build: `cmake -DWAMR_CONFIGURABLE_BOUNDS_CHECKS=1`
2. Compile: `wamrc --bounds-check=0 -o app.aot app.wasm`
3. Run: `iwasm --disable-bounds-checks app.aot`

**Tradeoffs**:
- Smaller AOT files
- Crashes instead of exceptions on boundary violations
- Fails Wasm spec tests

## 7. Linux Perf Profiling

Profile JIT/AOT code with linux-perf. Requires `--enable-linux-perf` flag.

**Supported modes**: LLVM-JIT, AOT

**Basic workflow**:

1. Record:
```bash
perf record --output=perf.data.raw -- iwasm --enable-linux-perf foo.wasm
```

2. Inject (JIT mode only):
```bash
perf inject --jit --input=perf.data.raw --output=perf.data
```

3. Analyze:
```bash
perf report --input=perf.data
```

**Debug symbols**: Use debug builds to avoid "[unknown]" functions. Add `-g2` (EMCC) to preserve custom name section.

### 7.1 Flamegraph Visualization

[Flamegraph](https://www.brendangregg.com/flamegraphs.html) visualizes hot code paths.

**Workflow**:
```bash
perf record -k mono --call-graph=fp --output=perf.data.raw -- iwasm --enable-linux-perf foo.wasm
perf inject --jit --input=perf.data.raw --output=perf.data  # JIT only
perf script > out.perf
./FlameGraph/stackcollapse-perf.pl out.perf > out.folded
./FlameGraph/flamegraph.pl out.folded > perf.svg
```

**Filtering**: Use `grep "wasm_runtime_invoke_native" out.folded` for Wasm-only stacks.

**Name translation**: Use [trans_wasm_func_name.py](../test-tools/trans-jitted-func-name/trans_wasm_func_name.py) to convert `aot_func#N` to original names (requires wabt and name section).

## 8. Optimize Native/Wasm Call Boundary

Reduce overhead for frequent native ↔ Wasm calls.

### 8.1 Register Native Signatures to AOT Compiler

AOT compiler generates slow `aot_invoke_native` calls for unknown signatures. Register native APIs to wamrc for optimized LLVM IR:

```bash
wamrc --native-lib=./libtest_add.so -o app.aot app.wasm
```

See [export_native_api.md](./export_native_api.md) for implementation.

> Note: JIT automatically knows signatures registered at runtime.

### 8.2 Optimize Wasm-C-API Imports (AOT)

For imports registered via `wasm_instance_new()`:

```bash
wamrc --invoke-c-api-import -o app.aot app.wasm
```

> Note: JIT handles this automatically.

### 8.3 Quick Entry Optimization (Native → Wasm)

WAMR pre-registers fast call paths for common signatures (enabled by default via `WAMR_BUILD_QUICK_AOT_ENTRY`):

- 0-4 i32/i64 args, i32/i64/void return
- 5 i32 args, i32/i64/void return

**Design recommendation**: Export functions with these signatures. For incompatible types (e.g., f32), add wrapper functions with type punning:

```C
int32 foo1(int32 arg_i32) {
    float arg_f32 = *(float *)&arg_i32;
    float res_f32 = foo(arg_f32);
    return *(int32 *)&res_f32;
}
```

See [embed_wamr.md](./embed_wamr.md) for host-side calling conventions.

---

## Related Documentation

- [memory_tune.md](./memory_tune.md) - Memory optimization (SSOT)
- [build_wamr.md](./build_wamr.md) - Build configuration (SSOT)
- [debugging.md](./debugging.md) - Debugging and profiling tools
