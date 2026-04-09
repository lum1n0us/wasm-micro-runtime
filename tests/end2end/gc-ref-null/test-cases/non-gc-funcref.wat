(module
  ;; Test: ref.null funcref in non-GC mode
  ;; Encoding: 0xD0 0x70
  ;; Expected: Success in non-GC mode

  (type (func (result i32)))

  (func $test (result funcref)
    ref.null func    ;; 0xD0 0x70 - valid in non-GC mode
  )

  (func (export "main") (result i32)
    call $test
    ref.is_null
    if (result i32)
      i32.const 42  ;; Success: ref.null returned null
    else
      i32.const 0   ;; Failure
    end
  )
)
