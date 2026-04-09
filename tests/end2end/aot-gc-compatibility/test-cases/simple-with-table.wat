(module
  ;; Test: Simple module with table to trigger init_expr parsing
  ;; This ensures the AOT loader's feature_flags check is exercised

  (table 1 funcref)

  (func $dummy (result i32)
    i32.const 0
  )

  (func (export "main") (result i32)
    i32.const 42
  )
)
