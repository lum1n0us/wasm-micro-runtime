# WAMR End-to-End Test Matrix

## Goal: Backward Compatibility

Enable GC-built iwasm to load both GC and non-GC WASM/AOT files, supporting gradual migration.

## Complete Test Matrix

### AOT Mode

| iwasm Build | wamrc --enable-gc | Expected | Current Status | Test Suite |
|-------------|-------------------|----------|----------------|------------|
| GC=1 | Yes | ✅ Success | ✅ **PASS** | `aot-gc-compatibility` |
| GC=1 | No | ✅ Success | 🔴 **BLOCKED** (disabled) | `aot-gc-compatibility` |
| GC=0 | No | ✅ Success | ✅ **PASS** | `aot-gc-compatibility` |
| GC=0 | Yes | ❌ Fail | ❌ Not tested | Feature check rejects at load |

### Interpreter Mode

| iwasm Build | LoadArgs.enable_gc | WASM Content | Expected | Current Status | Test Suite |
|-------------|-------------------|--------------|----------|----------------|------------|
| GC=1 | true | GC features | ✅ Success | ⚠️ **WABT** version | `gc-ref-null` |
| GC=1 | false | Non-GC | ✅ Success | ⏳ **TODO** | `gc-ref-null` |
| GC=0 | false | Non-GC | ✅ Success | ✅ **PASS** | `gc-ref-null` |
| GC=0 | true | GC features | ❌ Fail | ❌ Not tested | Invalid config |

## Status Legend

- ✅ **PASS**: Test exists and passes
- 🔴 **BLOCKED**: Test exists but disabled due to known bug
- ⚠️ **WABT**: Test blocked by WABT version (needs >= 1.0.38 for GC syntax)
- ⏳ **TODO**: Test not yet implemented
- ❌ **Not tested**: Invalid/unsupported configuration

## Blocked Issues

### 🔴 AOT Backward Compatibility (Critical)

**Test**: `e2e_aot_gc_compat_gc_iwasm_nogc_aot_with_table`

**Problem**: GC iwasm crashes when loading non-GC AOT files with tables.

**Root Cause**: 
```c
// In aot_loader.c:load_table_list()
#if WASM_ENABLE_GC != 0
    // This runs unconditionally when iwasm built with GC
    if (!load_init_expr(...))  // But non-GC AOT has no init_expr!
        return false;
#endif
```

**Impact**: Users cannot use one GC iwasm binary for both old and new AOT files.

**Needed Fix**: Check `module->feature_flags` at runtime instead of compile-time macro.

**Blocked By**: Structural layout compatibility between wamrc and iwasm when adding
`feature_flags` field conditionally.

### ⚠️ WABT Version

**Tests**: All GC mode interpreter tests in `gc-ref-null/`

**Problem**: WAT files use GC syntax (`struct`, `anyref`) requiring WABT >= 1.0.38.

**Current**: Container has WABT 1.0.37.

**Options**:
1. Upgrade WABT in devcontainer
2. Pre-compile WASM binaries
3. Conditional test based on WABT version detection

## Test Suites

### `aot-gc-compatibility/`
Tests AOT file loading compatibility between iwasm builds and wamrc compilation modes.

**Focus**: Backward compatibility for AOT deployment.

**Key Files**:
- `simple-no-table.wat`: Baseline test
- `simple-with-table.wat`: Triggers init_expr bug

### `gc-ref-null/`
Tests interpreter's `ref.null` parsing based on LoadArgs.enable_gc flag.

**Focus**: Resolver for `ref.null 0` ambiguity:
- Non-GC: 0x70 = funcref
- GC: 0x00 = type index 0

**Key Files**:
- `non-gc-*.wat`: Tests without GC features
- `gc-*.wat`: Tests requiring GC syntax (WABT >= 1.0.38)

## Running Tests

```bash
# GC build tests (current default)
cd build-e2e
ctest -R e2e

# Non-GC build tests
cd tests/end2end
cmake -B build-nogc -DWAMR_BUILD_GC=0
cd build-nogc
ctest

# Run only enabled tests (skip disabled)
ctest -R aot_gc_compat --output-on-failure
```

## Next Steps

1. **Fix AOT backward compatibility** (blocked tests)
   - Solve `feature_flags` structural layout issue
   - Enable `gc_iwasm_nogc_aot_*` tests

2. **Upgrade WABT** or pre-compile GC test WASMs
   - Enable GC mode interpreter tests

3. **Add interpreter backward compat tests**
   - Test GC iwasm loading non-GC WASM (LoadArgs.enable_gc=false)
