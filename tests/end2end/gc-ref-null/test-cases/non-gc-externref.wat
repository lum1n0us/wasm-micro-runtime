(module
  ;; Test: ref.null externref in non-GC mode
  ;; Encoding: 0xD0 0x6F
  ;; Expected: Success in non-GC mode

  (func $test (result externref)
    ref.null extern  ;; 0xD0 0x6F - valid in non-GC mode
  )

  (func (export "main") (result i32)
    call $test
    ref.is_null
    if (result i32)
      i32.const 42  ;; Success
    else
      i32.const 0
    end
  )
)
