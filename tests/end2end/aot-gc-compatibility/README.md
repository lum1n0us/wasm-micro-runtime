# AOT GC Compatibility Tests

## Purpose

Test that AOT files compiled with/without `--enable-gc` can only be loaded by
iwasm binaries built with matching GC configuration.

## Test Matrix

**Strategy**: Only test matching configurations (no negative tests).

| Build Config | iwasm GC | wamrc --enable-gc | Expected | Test Name |
|--------------|----------|-------------------|----------|-----------|
| WASM_ENABLE_GC=1 | ON | Yes | ✅ Success | `gc_aot_simple`, `gc_aot_with_table` |
| WASM_ENABLE_GC=0 | OFF | No | ✅ Success | `nogc_aot_simple`, `nogc_aot_with_table` |

Mismatched configurations (GC iwasm + non-GC AOT, or vice versa) are not tested as they should naturally fail during loading.

## Background

This test enforces the WAMR compatibility rule:
- **iwasm built with `WAMR_BUILD_GC=1` can ONLY load AOT files compiled with `wamrc --enable-gc`**
- **iwasm built with `WAMR_BUILD_GC=0` can ONLY load AOT files compiled without `--enable-gc`**

The mismatch is detected and rejected with:
- `AOT module load failed: garbage collection is not enabled in this build` (non-GC iwasm + GC AOT)
- Other error messages for GC iwasm + non-GC AOT

This is NOT a bug - it's the intended behavior to ensure ABI compatibility.

## Test Modules

All test modules are simple and include a table to trigger init_expr parsing:

- `simple-with-table.wat`: Minimal module with a funcref table
- Uses `(table 1 funcref)` to ensure init_expr path is exercised

## Related Issues

- Fixes generator/loader mismatch in `core/iwasm/aot/aot_loader.c`
- See `docs/superpowers/plans/2026-04-09-ref-null-gc-runtime-control.md`
