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

| Compilation flags                           | Default | nvu.          | +aot |
| ------------------------------------------- | ------- | ------------- | ---- |
| WAMR_APP_THREAD_STACK_SIZE_MAX              | ND[^3]  |               |      |
| WAMR_BH_LOG                                 | ND      | local_log     |      |
| WAMR_BH_VPRINTF                             | ND      | local_vprintf |      |
| WAMR_BUILD_ALLOC_WITH_USAGE                 | ND      | 1             |      |
| WAMR_BUILD_ALLOC_WITH_USER_DATA             | ND      | 1             |      |
| WAMR_BUILD_AOT                              | ND      | 0             | 1    |
| WAMR_BUILD_AOT_INTRINSICS                   | 1[^4]   |               | 1    |
| WAMR_BUILD_AOT_STACK_FRAME                  | ND      |               | 1    |
| WAMR_BUILD_AOT_VALIDATOR                    | ND      |               | 1    |
| WAMR_BUILD_BULK_MEMORY                      | 1       | 0             |      |
| WAMR_BUILD_COPY_CALL_STACK                  | ND      |               |      |
| WAMR_BUILD_CUSTOM_NAME_SECTION              | ND      |               |      |
| WAMR_BUILD_DEBUG_AOT                        | ND      |               | 1    |
| WAMR_BUILD_DEBUG_INTERP                     | ND      | 1             |      |
| WAMR_BUILD_DUMP_CALL_STACK                  | ND      | 1             |      |
| WAMR_BUILD_DYNAMIC_AOT_DEBUG                | ND      |               | 1    |
| WAMR_BUILD_EXCE_HANDLING                    | 0       |               |      |
| WAMR_BUILD_EXTENDED_CONST_EXPR              | 0       | 1             |      |
| WAMR_BUILD_FAST_INTERP                      | ND      | 1             |      |
| WAMR_BUILD_FAST_JIT                         | ND      |               |      |
| WAMR_BUILD_FAST_JIT_DUMP                    | ND      |               |      |
| WAMR_BUILD_GC                               | 0       |               |      |
| WAMR_BUILD_GC_HEAP_VERIFY                   | ND      |               |      |
| WAMR_BUILD_GLOBAL_HEAP_POOL                 | ND      | 1             |      |
| WAMR_BUILD_GLOBAL_HEAP_SIZE                 | ND      | 1M            |      |
| WAMR_BUILD_INSTRUCTION_METERING             | ND      |               |      |
| WAMR_BUILD_INTERP                           | ND      | 1             |      |
| WAMR_BUILD_INVOKE_NATIVE_GENERAL            | ND      |               |      |
| WAMR_BUILD_JIT                              | ND      |               |      |
| WAMR_BUILD_LAZY_JIT                         | 1[^5]   |               |      |
| WAMR_BUILD_LIBC_BUILTIN                     | ND      | 0             |      |
| WAMR_BUILD_LIBC_EMCC                        | ND      |               |      |
| WAMR_BUILD_LIBC_UVWASI                      | ND      |               |      |
| WAMR_BUILD_LIBC_WASI                        | ND      | 0             |      |
| WAMR_BUILD_LIB_PTHREAD                      | ND      |               |      |
| WAMR_BUILD_LIB_PTHREAD_SEMAPHORE            | ND      |               |      |
| WAMR_BUILD_LIB_RATS                         | ND      |               |      |
| WAMR_BUILD_LIB_WASI_THREADS                 | ND      |               |      |
| WAMR_BUILD_LINUX_PERF                       | ND      | 0             |      |
| WAMR_BUILD_LOAD_CUSTOM_SECTION              | ND      |               |      |
| WAMR_BUILD_MEMORY64                         | 0       |               |      |
| WAMR_BUILD_MEMORY_PROFILING                 | ND      | 1             |      |
| WAMR_BUILD_MINI_LOADER                      | ND      |               |      |
| WAMR_BUILD_MODULE_INST_CONTEXT              | ND      |               |      |
| WAMR_BUILD_MULTI_MEMORY                     | 0       |               |      |
| WAMR_BUILD_MULTI_MODULE                     | ND      |               |      |
| WAMR_BUILD_PERF_PROFILING                   | ND      | 1             |      |
| WAMR_BUILD_PLATFORM                         | ND      | zephyr        |      |
| WAMR_BUILD_QUICK_AOT_ENTRY                  | 1[^6]   |               |      |
| WAMR_BUILD_REF_TYPES                        | ND      | 0             |      |
| WAMR_BUILD_SANITIZER                        | ND      |               |      |
| WAMR_BUILD_SGX_IPFS                         | ND      |               |      |
| WAMR_BUILD_SHARED_HEAP                      | ND      | 1             |      |
| WAMR_BUILD_SHARED_MEMORY                    | 0       | 0             |      |
| WAMR_BUILD_SHRUNK_MEMORY                    | ND      | 0             |      |
| WAMR_BUILD_SIMD                             | ND      | 0             |      |
| WAMR_BUILD_SIMDE                            | ND      | 0             |      |
| WAMR_BUILD_SPEC_TEST                        | ND      |               |      |
| WAMR_BUILD_STACK_GUARD_SIZE                 | ND      |               |      |
| WAMR_BUILD_STATIC_PGO                       | ND      |               |      |
| WAMR_BUILD_STRINGREF                        | 0       |               |      |
| WAMR_BUILD_TAIL_CALL                        | 0       |               |      |
| WAMR_BUILD_TARGET                           | ND      | ARC/X86-32 ?  |      |
| WAMR_BUILD_THREAD_MGR                       | ND      |               |      |
| WAMR_BUILD_WAMR_COMPILER                    | ND      |               |      |
| WAMR_BUILD_WASI_EPHEMERAL_NN                | ND      |               |      |
| WAMR_BUILD_WASI_NN                          | ND      |               |      |
| WAMR_BUILD_WASI_NN_ENABLE_EXTERNAL_DELEGATE | ND      |               |      |
| WAMR_BUILD_WASI_NN_ENABLE_GPU               | ND      |               |      |
| WAMR_BUILD_WASI_NN_EXTERNAL_DELEGATE_PATH   | ND      |               |      |
| WAMR_BUILD_WASI_NN_LLAMACPP                 | ND      |               |      |
| WAMR_BUILD_WASI_NN_ONNX                     | ND      |               |      |
| WAMR_BUILD_WASI_NN_OPENVINO                 | ND      |               |      |
| WAMR_BUILD_WASI_NN_TFLITE                   | ND      |               |      |
| WAMR_BUILD_WASI_TEST                        | ND      |               |      |
| WAMR_BUILD_WASM_CACHE                       | ND      |               |      |
| WAMR_CONFIGURABLE_BOUNDS_CHECKS             | ND      |               |      |
| WAMR_DISABLE_APP_ENTRY                      | ND      |               |      |
| WAMR_DISABLE_HW_BOUND_CHECK                 | ND      | 1             |      |
| WAMR_DISABLE_STACK_HW_BOUND_CHECK           | ND      | 1             |      |
| WAMR_DISABLE_WAKEUP_BLOCKING_OP             | ND      |               |      |
| WAMR_DISABLE_WRITE_GS_BASE                  | ND      | 1             |      |
| WAMR_TEST_GC                                | ND      |               |      |

[^3]: _ND_ represents _not defined_
[^4]: active if `WAMR_BUILD_AOT` is 1
[^5]: active if `WAMR_BUILD_FAST_JIT` or `WARM_BUILD_JIT1` is 1
[^6]: active if `WAMR_BUILD_AOT` or `WAMR_BUILD_JIT` is 1
