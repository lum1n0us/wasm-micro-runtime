# Memory Optimization

SSOT for memory optimization.

**Prerequisites**:
2. [architecture_overview.md](./architecture_overview.md) - Memory model overview (SSOT)
3. [embed_wamr.md](./embed_wamr.md) - Runtime initialization (SSOT)


**External Resources**:
- [Blog: Understand WAMR heap](https://bytecodealliance.github.io/wamr.dev/blog/understand-the-wamr-heaps/)
- [Blog: Understand WAMR stacks](https://bytecodealliance.github.io/wamr.dev/blog/understand-the-wamr-stacks/)

## Memory Model

<center><img src="./pics/wamr_memory_model.png" width="75%" height="75%"></img></center>

**Key components**:

- **Global heap**: Runtime data structures (module, instance, exec_env, operand stack). Configured via `wasm_runtime_full_init()`. See [embed_wamr.md § Runtime Initialization](./embed_wamr.md#the-runtime-initialization).

- **Wasm operand stack**: Stack machine operands. Size set by `wasm_runtime_create_exec_env()` or `wasm_runtime_instantiate()`.

- **Linear memory**: Contiguous byte array (data + aux stack + heap). Size controlled at compile time:
  ```bash
  clang -Wl,--initial-memory=N1,--max-memory=N2  # wasi-sdk
  ```
  Uses `os_mmap` when hardware trap enabled (default on x86-64).

- **Aux stack**: Temporary data for complex arguments. Size: `-z stack-size=n` (wasi-sdk) or `-s TOTAL_STACK=n` (emsdk).

- **App/libc heap**: Wasm app allocations. Export `malloc/free` to use libc heap (recommended):
  ```bash
  clang -Wl,--export=malloc,--export=free
  ```

- **`__data_end`/`__heap_base` globals**: Export these for automatic linear memory truncation (reduces footprint).

## Optimization Strategies

### Runtime Configuration

- **Global heap**: `wasm_runtime_full_init(RuntimeInitArgs)`
- **Operand stack**: `wasm_runtime_create_exec_env()` or `wasm_runtime_instantiate()`
- **App heap**: `wasm_runtime_instantiate()` (or export malloc/free to use libc heap)

### Compile-Time Reduction

- **Linear memory**: `-Wl,--initial-memory=N,--max-memory=M`
- **Aux stack**: `-z stack-size=N`
- **Binary size**: `-Wl,--strip-all` with nostdlib. See [build_wasm_app.md § Reduce Footprint](./build_wasm_app.md#2-how-to-reduce-the-footprint).

### Advanced Techniques

- **XIP mode**: Execute in-place without copying. See [xip.md](./xip.md).
- **Early buffer release**: Set `clone_wasm_binary=false` (Wasm-C-API) or `wasm_binary_freeable=true` (loader), then free with `wasm_byte_vec_delete()` after loading. Check `wasm_module_is_underlying_binary_freeable()` first. Example: [free_buffer_early.c](../samples/basic/src/free_buffer_early.c).
- **Shrunk memory**: `cmake -DWAMR_BUILD_SHRUNK_MEMORY=1` (may affect spec compliance).

---

## Related Documentation

- [perf_tune.md](./perf_tune.md) - Performance tuning (SSOT)
- [embed_wamr.md](./embed_wamr.md) - Runtime initialization (SSOT)
- [build_wasm_app.md](./build_wasm_app.md) - Application build options
- [xip.md](./xip.md) - Execution in-place feature