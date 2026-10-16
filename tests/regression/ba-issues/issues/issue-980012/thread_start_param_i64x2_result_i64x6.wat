(module
  (import "wasi" "thread-spawn" (func $spawn (param i32) (result i32)))
  (memory (export "memory") 1 1 shared)

  ;; wasi-threads mandates (param i32 i32) with no results.
  ;; Wrong parameter types: the callee reads 4 cells out of the 2-cell host
  ;; argv, and the six i64 results write back past it.
  (func (export "wasi_thread_start") (param i64 i64)
                                     (result i64 i64 i64 i64 i64 i64)
    local.get 0
    local.get 1
    i64.const 0
    i64.const 0
    i64.const 0x6262626262626262
    i64.const 0x6363636363636363)

  ;; The spawn must be rejected (-1). An accepted spawn traps.
  (func (export "_start")
    (if (i32.ne (call $spawn (i32.const 0)) (i32.const -1))
      (then unreachable)))
)
