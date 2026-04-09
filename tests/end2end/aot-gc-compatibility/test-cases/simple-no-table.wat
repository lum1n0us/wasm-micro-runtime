(module
  ;; Test: Simple module without table
  ;; Used as a baseline to verify basic AOT functionality

  (func (export "main") (result i32)
    i32.const 42
  )
)
