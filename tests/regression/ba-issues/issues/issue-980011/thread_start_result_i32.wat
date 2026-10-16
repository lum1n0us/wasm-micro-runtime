(module
  (import "wasi" "thread-spawn" (func $spawn (param i32) (result i32)))
  (memory (export "memory") 1 1 shared)

  ;; wasi-threads mandates (param i32 i32) with no results.
  ;; Even a single result is outside the ABI.
  (func (export "wasi_thread_start") (param i32 i32)
                                     (result i32)
    i32.const 1)

  ;; The spawn must be rejected (-1). An accepted spawn traps.
  (func (export "_start")
    (if (i32.ne (call $spawn (i32.const 0)) (i32.const -1))
      (then unreachable)))
)
