# AOT GC Compatibility Tests

## Purpose

Test AOT backward compatibility: GC-enabled iwasm should be able to load both
GC and non-GC AOT files, while non-GC iwasm can only load non-GC AOT files.

## Test Matrix

**Goal**: Enable backward compatibility for GC builds.

| Build Config | iwasm GC | wamrc --enable-gc | Expected | Status | Test Name |
|--------------|----------|-------------------|----------|--------|-----------|
| WASM_ENABLE_GC=1 | ON | Yes | ✅ Success | ✅ Pass | `gc_aot_simple`, `gc_aot_with_table` |
| WASM_ENABLE_GC=1 | ON | No | ✅ Success | 🔴 **BLOCKED** | `gc_iwasm_nogc_aot_backward_compat` |
| WASM_ENABLE_GC=0 | OFF | No | ✅ Success | ✅ Pass | `nogc_aot_simple`, `nogc_aot_with_table` |
| WASM_ENABLE_GC=0 | OFF | Yes | ❌ Fail | ❌ Not tested | Non-GC iwasm cannot load GC features |

## Current Status

### ✅ Working Cases
- **GC iwasm + GC AOT**: Fully functional
- **Non-GC iwasm + non-GC AOT**: Fully functional

### 🔴 Blocked Case (Known Bug)
**GC iwasm + non-GC AOT** (backward compatibility):
- **Expected**: Should succeed (backward compatibility)
- **Actual**: Crashes or fails with "unexpected end"
- **Root Cause**: AOT loader unconditionally reads table init_expr when `WASM_ENABLE_GC != 0`,
  but non-GC AOT files don't contain init_expr data
- **Location**: `core/iwasm/aot/aot_loader.c:load_table_list()`

The fix requires checking `module->feature_flags` at runtime instead of compile-time
`WASM_ENABLE_GC` macro, but this has structural layout compatibility issues.

## Background

### The Problem We're Solving

When `wamrc` compiles without `--enable-gc`, it doesn't write table init_expr to AOT file.
But when `iwasm` is built with `WAMR_BUILD_GC=1`, the loader uses `#if WASM_ENABLE_GC != 0`
and unconditionally tries to read init_expr, causing:
1. Reading past end of buffer → "unexpected end"
2. Memory corruption → crash

### Why Backward Compatibility Matters

Users want to:
1. Build one GC-enabled iwasm binary
2. Run both old (non-GC) and new (GC) AOT files
3. Gradually migrate workloads without maintaining two separate iwasm builds

This is the **ref.null 0 ambiguity** problem:
- Non-GC mode: `0xD0 0x70` = ref.null funcref
- GC mode: `0xD0 0x00` = ref.null (type 0)

The loader must detect which mode the AOT file was compiled in, not just use
the iwasm's compile-time configuration.

## Test Modules

- `simple-no-table.wat`: Minimal module without tables (baseline test)
- `simple-with-table.wat`: Module with funcref table (triggers init_expr path)

The table test is critical because it exercises the code path where the bug occurs.

## Related Issues

- Original design: `docs/superpowers/plans/2026-04-09-ref-null-gc-runtime-control.md`
- Blocked by: AOT module structure layout compatibility issues
