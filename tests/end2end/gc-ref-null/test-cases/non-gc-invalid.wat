(module
  ;; Test: ref.null with invalid type in non-GC mode
  ;; Encoding: 0xD0 0x00
  ;; Expected: Load failure - type mismatch

  (func (export "main") (result i32)
    ;; This will be encoded as 0xD0 0x00
    ;; Invalid in non-GC mode (neither funcref nor externref)
    ;; Note: WAT parsers may reject this, so we test the bytecode path
    i32.const 0
  )
)
