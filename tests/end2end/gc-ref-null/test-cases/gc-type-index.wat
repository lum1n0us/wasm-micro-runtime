(module
  ;; Test: ref.null with type index in GC mode
  ;; Encoding: 0xD0 0x00 (ref.null type_0)
  ;; Expected: Success in GC mode when type 0 exists

  (type $my_struct (struct (field i32)))

  (func $test (result (ref null $my_struct))
    ref.null $my_struct  ;; 0xD0 0x00 - valid in GC mode
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
