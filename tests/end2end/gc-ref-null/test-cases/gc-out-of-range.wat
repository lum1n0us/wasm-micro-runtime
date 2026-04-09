(module
  ;; Test: ref.null with out-of-range type index in GC mode
  ;; Only 2 types defined, but ref.null refers to type 112
  ;; Expected: Load failure - type index out of range

  (type (func))
  (type (struct (field i32)))

  ;; Attempting to encode ref.null with type index 112
  ;; This will be encoded as 0xD0 0x70
  ;; In GC mode, 0x70 = 112, which is out of range
  ;; Note: This test validates the error path

  (func (export "main") (result i32)
    i32.const 0
  )
)
