## Tired Supported

- A. Production Ready. full tested. the highest level of support, confidence, and correctness for a component.
- B. Almost Production Ready. partially tested.
- C. Not Production Ready. experimental or not finished.

> [!TODO]
> ⬇️

| Target | Tired |
| ------ | ----- |
|        |       |

| Platform | Tired |
| -------- | ----- |
|          |       |

## All compilation flags

| Compilation flags                           | Tired | Default | on Ubuntu | v1            | v2  |
| ------------------------------------------- | ----- | ------- | --------- | ------------- | --- |
| WAMR_APP_THREAD_STACK_SIZE_MAX              | B     | ND[^1]  |           |               |     |
| WAMR_BH_LOG                                 | B     | ND      |           | local_log     |     |
| WAMR_BH_VPRINTF                             | B     | ND      |           | local_vprintf |     |
| WAMR_BUILD_ALLOC_WITH_USAGE                 | B     | ND      |           | 1             |     |
| WAMR_BUILD_ALLOC_WITH_USER_DATA             | B     | ND      |           | 1             |     |
| WAMR_BUILD_AOT                              | A     | ND      | 1         | 0             | 1   |
| WAMR_BUILD_AOT_INTRINSICS                   | A     | 1[^2]   |           |               | 1   |
| WAMR_BUILD_AOT_STACK_FRAME                  | A     | ND      |           |               | 1   |
| WAMR_BUILD_AOT_VALIDATOR                    | B     | ND      |           |               | 1   |
| WAMR_BUILD_BULK_MEMORY                      | A     | 1       |           | 0             |     |
| WAMR_BUILD_COPY_CALL_STACK                  | B     | ND      |           |               |     |
| WAMR_BUILD_CUSTOM_NAME_SECTION              | B     | ND      |           |               |     |
| WAMR_BUILD_DEBUG_AOT                        | B     | ND      |           |               | 1   |
| WAMR_BUILD_DEBUG_INTERP                     | B     | ND      |           | 1             |     |
| WAMR_BUILD_DUMP_CALL_STACK                  | A     | ND      |           | 1             |     |
| WAMR_BUILD_DYNAMIC_AOT_DEBUG                | B     | ND      |           |               | 1   |
| WAMR_BUILD_EXCE_HANDLING                    | B     | 0       |           |               |     |
| WAMR_BUILD_EXTENDED_CONST_EXPR              | A     | 0       |           | ?             |     |
| WAMR_BUILD_FAST_INTERP                      | A     | ND      | 1         | 1             |     |
| WAMR_BUILD_FAST_JIT                         | B     | ND      |           |               |     |
| WAMR_BUILD_FAST_JIT_DUMP                    | B     | ND      |           |               |     |
| WAMR_BUILD_GC                               | B     | 0       |           |               |     |
| WAMR_BUILD_GC_HEAP_VERIFY                   | B     | ND      |           |               |     |
| WAMR_BUILD_GLOBAL_HEAP_POOL                 | A     | ND      |           | 1             |     |
| WAMR_BUILD_GLOBAL_HEAP_SIZE                 | A     | ND      |           | ?             |     |
| WAMR_BUILD_INSTRUCTION_METERING             | C     | ND      |           |               |     |
| WAMR_BUILD_INTERP                           | A     | ND      | 1         | 1             |     |
| WAMR_BUILD_INVOKE_NATIVE_GENERAL            | B     | ND      |           |               |     |
| WAMR_BUILD_JIT                              | B     | ND      |           |               |     |
| WAMR_BUILD_LAZY_JIT                         | B     | 1[^3]   |           |               |     |
| WAMR_BUILD_LIBC_BUILTIN                     | A     | ND      | 1         | 0             |     |
| WAMR_BUILD_LIBC_EMCC                        | C     | ND      |           |               |     |
| WAMR_BUILD_LIBC_UVWASI                      | B     | ND      |           |               |     |
| WAMR_BUILD_LIBC_WASI                        | A     | ND      | 1         | 0             |     |
| WAMR_BUILD_LIB_PTHREAD                      | A     | ND      |           |               |     |
| WAMR_BUILD_LIB_PTHREAD_SEMAPHORE            | A     | ND      |           |               |     |
| WAMR_BUILD_LIB_RATS                         | B     | ND      |           |               |     |
| WAMR_BUILD_LIB_WASI_THREADS                 | A     | ND      |           |               |     |
| WAMR_BUILD_LINUX_PERF                       | B     | ND      |           | 0             |     |
| WAMR_BUILD_LOAD_CUSTOM_SECTION              | A     | ND      |           |               |     |
| WAMR_BUILD_MEMORY64                         | A     | 0       |           |               |     |
| WAMR_BUILD_MEMORY_PROFILING                 | A     | ND      |           | 1             |     |
| WAMR_BUILD_MINI_LOADER                      | B     | ND      |           |               |     |
| WAMR_BUILD_MODULE_INST_CONTEXT              | B     | ND      | 1         |               |     |
| WAMR_BUILD_MULTI_MEMORY                     | A     | 0       |           |               |     |
| WAMR_BUILD_MULTI_MODULE                     | B     | ND      |           |               |     |
| WAMR_BUILD_PERF_PROFILING                   | A     | ND      |           | 1             |     |
| WAMR_BUILD_PLATFORM                         |       | ND      | linux     | zephyr        |     |
| WAMR_BUILD_QUICK_AOT_ENTRY                  | A     | 1[^4]   |           |               |     |
| WAMR_BUILD_REF_TYPES                        | A     | ND      | 1         | 0             |     |
| WAMR_BUILD_SANITIZER                        | B     | ND      |           |               |     |
| WAMR_BUILD_SGX_IPFS                         | B     | ND      |           |               |     |
| WAMR_BUILD_SHARED_HEAP                      | A     | ND      |           | 1             |     |
| WAMR_BUILD_SHARED_MEMORY                    | A     | 0       | 1         | 0             |     |
| WAMR_BUILD_SHRUNK_MEMORY                    | A     | ND      | 1         | 0             |     |
| WAMR_BUILD_SIMD                             | A     | ND      | 1         | 0             |     |
| WAMR_BUILD_SIMDE                            | B     | ND      | 1         | 0             |     |
| WAMR_BUILD_SPEC_TEST                        | A     | ND      |           |               |     |
| WAMR_BUILD_STACK_GUARD_SIZE                 | A     | ND      |           |               |     |
| WAMR_BUILD_STATIC_PGO                       | B     | ND      |           |               |     |
| WAMR_BUILD_STRINGREF                        | B     | 0       |           |               |     |
| WAMR_BUILD_TAIL_CALL                        | A     | 0       | 1         |               |     |
| WAMR_BUILD_TARGET                           |       | ND      | X86-64    | ARC/X86-32 ?  |     |
| WAMR_BUILD_THREAD_MGR                       | A     | ND      |           |               |     |
| WAMR_BUILD_WAMR_COMPILER                    | A     | ND      |           |               |     |
| WAMR_BUILD_WASI_EPHEMERAL_NN                | B     | ND      |           |               |     |
| WAMR_BUILD_WASI_NN                          | B     | ND      |           |               |     |
| WAMR_BUILD_WASI_NN_ENABLE_EXTERNAL_DELEGATE | B     | ND      |           |               |     |
| WAMR_BUILD_WASI_NN_ENABLE_GPU               | B     | ND      |           |               |     |
| WAMR_BUILD_WASI_NN_EXTERNAL_DELEGATE_PATH   | B     | ND      |           |               |     |
| WAMR_BUILD_WASI_NN_LLAMACPP                 | B     | ND      |           |               |     |
| WAMR_BUILD_WASI_NN_ONNX                     | B     | ND      |           |               |     |
| WAMR_BUILD_WASI_NN_OPENVINO                 | B     | ND      |           |               |     |
| WAMR_BUILD_WASI_NN_TFLITE                   | B     | ND      |           |               |     |
| WAMR_BUILD_WASI_TEST                        | B     | ND      |           |               |     |
| WAMR_BUILD_WASM_CACHE                       | B     | ND      |           |               |     |
| WAMR_CONFIGURABLE_BOUNDS_CHECKS             | C     | ND      |           |               |     |
| WAMR_DISABLE_APP_ENTRY                      | A     | ND      |           |               |     |
| WAMR_DISABLE_HW_BOUND_CHECK                 | A     | ND      |           | 1             |     |
| WAMR_DISABLE_STACK_HW_BOUND_CHECK           | A     | ND      |           | 1             |     |
| WAMR_DISABLE_WAKEUP_BLOCKING_OP             | B     | ND      |           |               |     |
| WAMR_DISABLE_WRITE_GS_BASE                  | B     | ND      |           | 1             |     |
| WAMR_TEST_GC                                | B     | ND      |           |               |     |

[^1]: _ND_ represents _not defined_
[^2]: active if `WAMR_BUILD_AOT` is 1
[^3]: active if `WAMR_BUILD_FAST_JIT` or `WARM_BUILD_JIT1` is 1
[^4]: active if `WAMR_BUILD_AOT` or `WAMR_BUILD_JIT` is 1
