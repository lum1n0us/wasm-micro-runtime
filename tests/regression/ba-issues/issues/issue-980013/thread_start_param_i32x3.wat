(module
  (import "wasi" "thread-spawn" (func $spawn (param i32) (result i32)))
  (memory (export "memory") 1 1 shared)

  ;; wasi-threads mandates exactly two i32 parameters.
  ;; Three parameters make the callee read one cell past the 2-cell host argv.
  (func (export "wasi_thread_start") (param i32 i32 i32))

  ;; The spawn must be rejected (-1). An accepted spawn traps.
  (func (export "_start")
    (if (i32.ne (call $spawn (i32.const 0)) (i32.const -1))
      (then unreachable)))
)
