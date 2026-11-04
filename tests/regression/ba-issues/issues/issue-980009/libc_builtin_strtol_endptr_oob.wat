;; Regression test for the libc-builtin strtol/strtoul endptr write.
;;
;; strtol/strtoul write a full host `char *` (8 bytes on 64-bit) through
;; endptr, while the wrapper only validates the 4-byte guest slot. The
;; wrapper must write exactly the 4-byte guest offset back, leave the bytes
;; after the slot untouched, and treat guest offset 0 as NULL.
;;
;; to_test returns a bit mask; 0 means every check passed:
;;   bit 0  strtol  return value             (mid-memory slot)
;;   bit 1  strtol  slot holds end offset    (mid-memory slot)
;;   bit 2  strtol  slot+4 keeps sentinel    (mid-memory slot)
;;   bit 3  strtol  return value             (guest NULL)
;;   bit 4  strtol  memory[0..8) untouched   (guest NULL)
;;   bit 5  strtol  slot holds end offset    (last 4 bytes of memory)
;;   bit 6..11      same checks for strtoul
;;
;; to_test_oob passes an endptr with only 3 bytes left in memory: the wrapper
;; must reject it with an out-of-bounds exception.
(module
  (import "env" "strtol" (func $strtol (param i32 i32 i32) (result i32)))
  (import "env" "strtoul" (func $strtoul (param i32 i32 i32) (result i32)))
  (memory (export "memory") 1)

  ;; "100" at offset 100, so the end pointer of strtol/strtoul is 103.
  (data (i32.const 100) "100\00")

  ;; Returns bad | bit when cond is non-zero, otherwise bad.
  (func $flag (param $bad i32) (param $cond i32) (param $bit i32) (result i32)
    (select
      (i32.or (local.get $bad) (local.get $bit))
      (local.get $bad)
      (local.get $cond)))

  (func (export "to_test") (result i32)
    (local $bad i32)

    ;; strtol, slot in the middle of memory
    (i32.store (i32.const 1024) (i32.const 0x41414141))
    (i32.store (i32.const 1028) (i32.const 0x41414141))
    (local.set $bad
      (call $flag (local.get $bad)
        (i32.ne (call $strtol (i32.const 100) (i32.const 1024) (i32.const 10))
                (i32.const 100))
        (i32.const 1)))
    (local.set $bad
      (call $flag (local.get $bad)
        (i32.ne (i32.load (i32.const 1024)) (i32.const 103))
        (i32.const 2)))
    (local.set $bad
      (call $flag (local.get $bad)
        (i32.ne (i32.load (i32.const 1028)) (i32.const 0x41414141))
        (i32.const 4)))

    ;; strtol, guest NULL (offset 0)
    (i32.store (i32.const 0) (i32.const 0x41414141))
    (i32.store (i32.const 4) (i32.const 0x41414141))
    (local.set $bad
      (call $flag (local.get $bad)
        (i32.ne (call $strtol (i32.const 100) (i32.const 0) (i32.const 10))
                (i32.const 100))
        (i32.const 8)))
    (local.set $bad
      (call $flag (local.get $bad)
        (i32.or
          (i32.ne (i32.load (i32.const 0)) (i32.const 0x41414141))
          (i32.ne (i32.load (i32.const 4)) (i32.const 0x41414141)))
        (i32.const 16)))

    ;; strtol, slot is the last 4 bytes of memory
    (i32.store (i32.const 65532) (i32.const 0))
    (drop (call $strtol (i32.const 100) (i32.const 65532) (i32.const 10)))
    (local.set $bad
      (call $flag (local.get $bad)
        (i32.ne (i32.load (i32.const 65532)) (i32.const 103))
        (i32.const 32)))

    ;; strtoul, slot in the middle of memory
    (i32.store (i32.const 1024) (i32.const 0x41414141))
    (i32.store (i32.const 1028) (i32.const 0x41414141))
    (local.set $bad
      (call $flag (local.get $bad)
        (i32.ne (call $strtoul (i32.const 100) (i32.const 1024) (i32.const 10))
                (i32.const 100))
        (i32.const 64)))
    (local.set $bad
      (call $flag (local.get $bad)
        (i32.ne (i32.load (i32.const 1024)) (i32.const 103))
        (i32.const 128)))
    (local.set $bad
      (call $flag (local.get $bad)
        (i32.ne (i32.load (i32.const 1028)) (i32.const 0x41414141))
        (i32.const 256)))

    ;; strtoul, guest NULL (offset 0)
    (i32.store (i32.const 0) (i32.const 0x41414141))
    (i32.store (i32.const 4) (i32.const 0x41414141))
    (local.set $bad
      (call $flag (local.get $bad)
        (i32.ne (call $strtoul (i32.const 100) (i32.const 0) (i32.const 10))
                (i32.const 100))
        (i32.const 512)))
    (local.set $bad
      (call $flag (local.get $bad)
        (i32.or
          (i32.ne (i32.load (i32.const 0)) (i32.const 0x41414141))
          (i32.ne (i32.load (i32.const 4)) (i32.const 0x41414141)))
        (i32.const 1024)))

    ;; strtoul, slot is the last 4 bytes of memory
    (i32.store (i32.const 65532) (i32.const 0))
    (drop (call $strtoul (i32.const 100) (i32.const 65532) (i32.const 10)))
    (local.set $bad
      (call $flag (local.get $bad)
        (i32.ne (i32.load (i32.const 65532)) (i32.const 103))
        (i32.const 2048)))

    (local.get $bad))

  ;; endptr has only 3 bytes left in memory: must raise an exception.
  (func (export "to_test_oob") (result i32)
    (call $strtol (i32.const 100) (i32.const 65533) (i32.const 10)))
)
