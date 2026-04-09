# ref.null GC/Non-GC Parsing Tests

## Purpose

Test that WASM loader correctly parses `ref.null` instructions based on the
`LoadArgs.enable_gc` flag, handling the ambiguity of `ref.null 0`:
- Non-GC mode: `0xD0 0x70` = ref.null funcref
- GC mode: `0xD0 0x00` = ref.null (type 0)

## Test Matrix (Interpreter Mode)

**Goal**: Loader should adapt based on LoadArgs.enable_gc, not just compile-time WASM_ENABLE_GC.

| iwasm Build | LoadArgs.enable_gc | WASM Content | Expected | Status | Test Case |
|-------------|-------------------|--------------|----------|--------|-----------|
| GC=1 | true | GC features | ✅ Success | ⚠️ Need WABT | `gc_type_index`, `gc_abstract_type` |
| GC=1 | false | Non-GC | ✅ Success | ⚠️ TBD | Future: backward compat test |
| GC=0 | false | Non-GC | ✅ Success | ✅ Pass | `non_gc_funcref`, `non_gc_externref` |
| GC=0 | true | GC features | ❌ Fail | ❌ Not tested | Non-GC iwasm can't parse GC |

## Current Test Coverage

### Non-GC Build (WASM_ENABLE_GC=0)
- ✅ `non-gc-funcref.wat`: ref.null funcref (0x70)
- ✅ `non-gc-externref.wat`: ref.null externref (0x6F)
- ✅ `non-gc-invalid.wat`: Invalid heap type (negative test)

### GC Build (WASM_ENABLE_GC=1)
- ⚠️ `gc-type-index.wat`: ref.null with type index - **Requires WABT >= 1.0.38**
- ⚠️ `gc-abstract-type.wat`: ref.null with abstract types - **Requires WABT >= 1.0.38**
- ⚠️ `gc-out-of-range.wat`: Invalid type index (negative test) - **Requires WABT >= 1.0.38**

## Known Limitations

### WABT Version Dependency
GC tests use GC proposal syntax (`struct`, `anyref`, etc.) that requires WABT >= 1.0.38.
Current container has WABT 1.0.37, so GC tests cannot compile WAT → WASM.

**Workaround options**:
1. Upgrade WABT in devcontainer
2. Pre-compile WASM binaries for GC tests
3. Mark GC tests as conditional on WABT version

## Test Structure

Each test follows the pattern:
1. Compile WAT → WASM (using wat2wasm)
2. Load WASM with specified LoadArgs.enable_gc
3. Verify loading succeeds/fails as expected
4. If loaded, verify execution

## Related

- Design spec: `docs/superpowers/plans/2026-04-09-ref-null-gc-runtime-control.md`
- AOT tests: `tests/end2end/aot-gc-compatibility/`
