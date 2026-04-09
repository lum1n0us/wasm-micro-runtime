(module
  ;; Test: ref.null with abstract heap type in GC mode
  ;; Uses negative LEB128 for abstract types
  ;; Expected: Success in GC mode

  (func $test (result anyref)
    ref.null any  ;; Abstract heap type - valid in GC mode
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
