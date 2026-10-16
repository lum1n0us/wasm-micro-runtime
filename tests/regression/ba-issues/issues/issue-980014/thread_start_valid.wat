(module
  (import "wasi" "thread-spawn" (func $spawn (param i32) (result i32)))
  (memory (export "memory") 1 1 shared)

  ;; Signature-correct control: (param i32 i32) with no results.
  (func (export "wasi_thread_start") (param i32 i32))

  ;; The spawn must succeed (>= 0). Spin so the thread can run before exit.
  (func (export "_start") (local $i i32)
    (if (i32.eq (call $spawn (i32.const 0)) (i32.const -1))
      (then unreachable))
    (loop $l
      (local.set $i (i32.add (local.get $i) (i32.const 1)))
      (br_if $l (i32.lt_u (local.get $i) (i32.const 3000000)))))
)
