# Design Spec: Runtime Control of ref.null GC Mode

**Date**: 2026-04-09  
**Author**: AI Agent (Claude Sonnet 4.5)  
**Status**: Draft  
**Related Issue**: wamrc CLI option `--enable-gc` does not dynamically control wasm_loader.c parsing of ref.null opcode

---

## Executive Summary

This design enables runtime selection of GC mode for parsing WebAssembly `ref.null` opcodes, resolving the ambiguity between GC and non-GC encodings. Currently, the parsing mode is determined at compile time via `WASM_ENABLE_GC`, making wamrc's `--enable-gc` CLI flag ineffective. This fix allows the same WAMR binary to load both GC and non-GC modules by specifying the mode at load time through `LoadArgs`.

**Key Changes**:
- Extend `LoadArgs` API with `enable_gc` field
- Store GC mode in `WASMModule` structure
- Replace compile-time `#if WASM_ENABLE_GC` checks with runtime conditionals in three ref.null parsing locations
- Add comprehensive end-to-end testing infrastructure

---

## Table of Contents

1. [Problem Statement](#problem-statement)
2. [Solution Overview](#solution-overview)
3. [Design Details](#design-details)
4. [Error Handling](#error-handling)
5. [Testing Strategy](#testing-strategy)
6. [Implementation Plan](#implementation-plan)
7. [Documentation Updates](#documentation-updates)
8. [Risks and Mitigations](#risks-and-mitigations)

---

## Problem Statement

### The Ambiguity

The WebAssembly `ref.null` opcode has different binary encodings in GC and non-GC modes:

**Non-GC Mode** (MVP reference types):
```
ref.null <reftype>
  where reftype is a single byte:
    0x70 = funcref
    0x6F = externref
```

**GC Mode** (GC proposal):
```
ref.null <heaptype>
  where heaptype is a signed LEB128:
    >= 0: type index (e.g., 0, 1, 112)
    < 0:  abstract heap type (e.g., -0x10 for funcref)
```

**The byte sequence `0xD0 0x70` means**:
- Non-GC: `ref.null funcref` (valid)
- GC: `ref.null <type 112>` (valid only if 113+ types exist)

This creates **unresolvable ambiguity** from the byte stream alone.

### Current Bug

- `wasm_loader.c` uses compile-time `#if WASM_ENABLE_GC` to choose parsing logic
- wamrc's `--enable-gc` CLI flag only affects AOT compilation metadata
- The flag never reaches the wasm loader
- Result: A WAMR binary compiled with `WASM_ENABLE_GC=1` cannot load non-GC modules, and vice versa

### Requirements

1. Enable runtime selection of GC parsing mode
2. Maintain backward compatibility (default = non-GC)
3. Provide clear error messages for mode mismatches
4. Support mixed loading (GC and non-GC modules in same process)
5. Zero performance impact on non-GC builds

---

## Solution Overview

### Architecture

**Data Flow**:
```
wamrc CLI (--enable-gc flag)
  ↓
option.enable_gc = true/false
  ↓
LoadArgs args = { .enable_gc = option.enable_gc }
  ↓
wasm_runtime_load_ex(buf, size, &args, ...)
  ↓
module->is_gc_enabled = args->enable_gc
  ↓
Parse ref.null:
  if (module->is_gc_enabled) { /* GC parsing */ }
  else { /* non-GC parsing */ }
```

### Key Design Decisions

1. **LoadArgs Extension**: Add `enable_gc` field to existing `LoadArgs` structure
2. **Module State**: Store `is_gc_enabled` in `WASMModule` during load
3. **Unified Code Path**: Refactor three ref.null locations to eliminate code duplication
4. **Early Validation**: Detect configuration conflicts at load time, not execution time

### Component Interactions

| Build Config | LoadArgs.enable_gc | Behavior |
|--------------|-------------------|----------|
| `WASM_ENABLE_GC=0` | `false` | ✅ Load non-GC modules |
| `WASM_ENABLE_GC=0` | `true` | ❌ Error: GC not compiled |
| `WASM_ENABLE_GC=1` | `false` | ✅ Load non-GC modules |
| `WASM_ENABLE_GC=1` | `true` | ✅ Load GC modules |

---

## Design Details

### 3.1 API Changes

#### LoadArgs Extension

**File**: `core/iwasm/include/wasm_export.h`

```c
typedef struct LoadArgs {
    char *name;
    bool clone_wasm_binary;
    bool wasm_binary_freeable;
    bool delay_symbol_resolve;
    
    /**
     * Enable GC mode for parsing ref.null and related opcodes.
     * 
     * Default: false (non-GC mode)
     * 
     * In non-GC mode, ref.null accepts only:
     *   - 0x70 (funcref)
     *   - 0x6F (externref)
     * 
     * In GC mode, ref.null accepts:
     *   - Type indices (>= 0)
     *   - Abstract heap types (< 0)
     * 
     * Note: This field only takes effect when WAMR is compiled with
     * WASM_ENABLE_GC=1. If set to true when compiled without GC support,
     * module loading will fail with an error message.
     * 
     * @see WASM_ENABLE_GC compile option
     */
    bool enable_gc;
} LoadArgs;
```

**Backward Compatibility**: `wasm_runtime_load()` initializes `LoadArgs` with `enable_gc = false`.

#### WASMModule Extension

**File**: `core/iwasm/interpreter/wasm.h`

```c
struct WASMModule {
    /* ... existing fields ... */
    
#if WASM_ENABLE_GC != 0
    /* Whether this module uses GC encoding for ref.null and related opcodes.
     * Set from LoadArgs.enable_gc during module loading. */
    bool is_gc_enabled;
    
    /* Existing GC fields */
    HashMap *ref_type_set;
    struct WASMRttType **rtt_types;
    korp_mutex rtt_type_lock;
#endif

    /* ... other fields ... */
};
```

**Rationale**: Field placed inside `#if WASM_ENABLE_GC` to keep non-GC builds minimal.

### 3.2 Code Structure

#### ref.null Parsing Pattern

All three ref.null parsing locations follow this unified structure:

```c
case WASM_OP_REF_NULL:
{
#if WASM_ENABLE_GC != 0
    /* GC mode processing (only when runtime enables GC) */
    if (module->is_gc_enabled) {
        int32 heap_type;
        read_leb_int32(p, p_end, heap_type);
        
        if (heap_type >= 0) {
            /* Type index - boundary check */
            if (!check_type_index(module, module->type_count, heap_type,
                                 error_buf, error_buf_size)) {
                goto fail;
            }
            wasm_set_refheaptype_typeidx(&wasm_ref_type.ref_ht_typeidx,
                                        true, heap_type);
            ref_type = wasm_ref_type.ref_type;
        }
        else {
            /* Abstract heap type */
            if (!wasm_is_valid_heap_type(heap_type)) {
                set_error_buf_v(error_buf, error_buf_size,
                               "unknown heap type %d", heap_type);
                goto fail;
            }
            ref_type = (uint8)((int32)0x80 + heap_type);
        }
        
        PUSH_TYPE(ref_type);
        break;  // GC processing complete
    }
#endif

    /* Non-GC mode processing
     * - Always executed when WASM_ENABLE_GC=0
     * - Executed when WASM_ENABLE_GC=1 but is_gc_enabled=false */
    {
        uint8 ref_type;
        CHECK_BUF(p, p_end, 1);
        ref_type = read_uint8(p);
        
        if (ref_type != VALUE_TYPE_FUNCREF
            && ref_type != VALUE_TYPE_EXTERNREF) {
            set_error_buf_v(error_buf, error_buf_size,
                           "type mismatch: ref.null requires funcref (0x70) or "
                           "externref (0x6F), got 0x%02X", ref_type);
            goto fail;
        }
        
        PUSH_TYPE(ref_type);
    }
    break;
}
```

**Key Properties**:
- GC branch exits with `break` after processing
- Non-GC code appears only once (no duplication)
- Clear control flow: GC special-case, then fallthrough to default

#### Three Implementation Locations

1. **Constant Expression Parsing** (`load_init_expr()`, line ~1000)
   - Context: Global initializers
   - Special handling: Value stored in `InitExpression`

2. **Function Resolution** (`wasm_loader_resolve_functions()`, line ~7848)
   - Context: Skipping bytecode during resolution pass
   - Special handling: Must correctly skip LEB128 vs single byte

3. **Bytecode Preparation** (`wasm_loader_prepare_bytecode()`, line ~13662)
   - Context: Main function body parsing
   - Special handling: Type stack validation, operand encoding

### 3.3 Configuration Validation

**File**: `core/iwasm/interpreter/wasm_loader.c`

```c
static WASMModule *
wasm_load_from_sections(WASMSection *section_list,
                       const LoadArgs *load_args,
                       char *error_buf, uint32 error_buf_size)
{
#if WASM_ENABLE_GC == 0
    if (load_args && load_args->enable_gc) {
        set_error_buf(error_buf, error_buf_size,
                     "GC mode requested but WAMR was compiled without "
                     "WASM_ENABLE_GC support. Rebuild with -DWASM_ENABLE_GC=1");
        return NULL;
    }
#endif

    WASMModule *module = loader_malloc(sizeof(WASMModule), ...);
    // ...
    
#if WASM_ENABLE_GC != 0
    module->is_gc_enabled = (load_args && load_args->enable_gc);
#endif
    
    // Continue loading...
}
```

**Validation Logic**:
- Compile-time check: If `WASM_ENABLE_GC=0`, reject `enable_gc=true`
- Runtime setting: Store `enable_gc` in module for parsing decisions
- Fail-fast: Detect configuration errors at load start, not during parsing

### 3.4 wamrc Integration

**File**: `wamr-compiler/main.c`

```c
/* Before (line ~840) */
if (!(wasm_module = wasm_runtime_load(wasm_file, wasm_file_size, error_buf,
                                      sizeof(error_buf)))) {
    printf("%s\n", error_buf);
    goto fail2;
}

/* After */
LoadArgs load_args = { 0 };
load_args.name = "";
load_args.wasm_binary_freeable = false;
load_args.enable_gc = option.enable_gc;  // Pass CLI flag

if (!(wasm_module = wasm_runtime_load_ex(wasm_file, wasm_file_size, 
                                         &load_args,
                                         error_buf, sizeof(error_buf)))) {
    printf("%s\n", error_buf);
    goto fail2;
}
```

**Change Summary**:
- Switch from `wasm_runtime_load()` to `wasm_runtime_load_ex()`
- Initialize `LoadArgs` with CLI `--enable-gc` value
- Zero functional change for non-GC builds

---

## Error Handling

### 4.1 Configuration Conflicts

**Scenario**: User sets `enable_gc=true` but WAMR compiled with `WASM_ENABLE_GC=0`

**Detection**: At entry to `wasm_load_from_sections()`

**Error Message**:
```
GC mode requested but WAMR was compiled without WASM_ENABLE_GC support. 
Rebuild with -DWASM_ENABLE_GC=1
```

**User Action**: Rebuild WAMR with GC support enabled

### 4.2 ref.null Parsing Errors

#### Non-GC Mode Errors

**Scenario 1**: `ref.null 0x00` in non-GC mode

**Error**:
```
type mismatch: ref.null requires funcref (0x70) or externref (0x6F), got 0x00
```

**Scenario 2**: `ref.null 0xFF` in non-GC mode

**Error**:
```
type mismatch: ref.null requires funcref (0x70) or externref (0x6F), got 0xFF
```

#### GC Mode Errors

**Scenario 3**: `ref.null 112` but only 10 types exist

**Error**:
```
unknown type: type index 112 out of range [0, 9]
```

**Scenario 4**: `ref.null -99` (invalid abstract heap type)

**Error**:
```
unknown heap type: -99 is not a valid abstract heap type
```

### 4.3 Error Behavior Matrix

| Bytecode | Non-GC Mode | GC Mode (10 types) |
|----------|-------------|-------------------|
| `0xD0 0x00` | ❌ type mismatch | ✅ ref.null <type 0> |
| `0xD0 0x70` | ✅ ref.null funcref | ❌ type index 112 out of range |
| `0xD0 0x6F` | ✅ ref.null externref | ❌ type index 111 out of range |
| `0xD0 0xFF 0x00` | ❌ type mismatch: 0xFF | ✅ ref.null <type 127> (if exists) |

### 4.4 Error Detection Timing

**All errors detected at load time**, ensuring:
- No runtime failures due to malformed bytecode
- Clear error messages pointing to root cause
- Safe to cache and reuse loaded modules

---

## Testing Strategy

### 5.1 Test Layers

```
┌─────────────────────────────────┐
│  End-to-End Tests (E2E)         │  ← New test infrastructure
│  WAT → WASM → AOT → Execution   │
└─────────────────────────────────┘
                ↓
┌─────────────────────────────────┐
│  Integration Tests               │
│  wamrc + loader API calls        │
└─────────────────────────────────┘
                ↓
┌─────────────────────────────────┐
│  Unit Tests                      │
│  Individual component tests      │
└─────────────────────────────────┘
```

### 5.2 Unit Tests

**Location**: `tests/unit/gc-loader/test_ref_null_parsing.cc`

**Test Cases**:

1. **Configuration Validation**
   - `WASM_ENABLE_GC=0` + `enable_gc=false` → success
   - `WASM_ENABLE_GC=0` + `enable_gc=true` → error
   - `WASM_ENABLE_GC=1` + `enable_gc=false` → success
   - `WASM_ENABLE_GC=1` + `enable_gc=true` → success

2. **Non-GC Mode Parsing**
   - `ref.null funcref` → success
   - `ref.null externref` → success
   - `ref.null 0x00` → error "type mismatch"
   - `ref.null 0xFF` → error "type mismatch"

3. **GC Mode Parsing** (when `WASM_ENABLE_GC=1`)
   - `ref.null <type 0>` → success (if type exists)
   - `ref.null <type 112>` → error "out of range" (if < 113 types)
   - `ref.null <abstract type>` → success (if valid)
   - `ref.null <invalid abstract>` → error "unknown heap type"

4. **Three Locations Consistency**
   - Verify all three parsing locations behave identically
   - Test: constant expressions, function resolution, bytecode preparation

### 5.3 End-to-End Tests

**Location**: `tests/end2end/gc-ref-null/`

**Infrastructure**: CTest-based framework with WABT integration

**Test Cases**:

| Test Name | WAT File | enable_gc | Compile | Run | Description |
|-----------|----------|-----------|---------|-----|-------------|
| `e2e_gc_ref_null_non_gc_funcref` | non-gc-funcref.wat | false | success | success | ref.null funcref in non-GC mode |
| `e2e_gc_ref_null_non_gc_externref` | non-gc-externref.wat | false | success | success | ref.null externref in non-GC mode |
| `e2e_gc_ref_null_non_gc_invalid` | non-gc-invalid.wat | false | fail | skip | ref.null 0x00 rejected |
| `e2e_gc_ref_null_gc_type_index` | gc-type-index.wat | true | success | success | ref.null <type_idx> in GC mode |
| `e2e_gc_ref_null_gc_abstract_type` | gc-abstract-type.wat | true | success | success | ref.null <abstract> in GC mode |
| `e2e_gc_ref_null_gc_out_of_range` | gc-out-of-range.wat | true | fail | skip | Type index overflow |
| `e2e_gc_ref_null_cross_gc_wasm_non_gc_mode` | gc-type-index.wat | false | fail | skip | GC wasm with non-GC flag |
| `e2e_gc_ref_null_cross_non_gc_wasm_gc_mode` | non-gc-funcref.wat | true | fail | skip | Non-GC wasm with GC flag |

**Running E2E Tests**:
```bash
mkdir build-e2e && cd build-e2e
cmake .. -DWAMR_BUILD_E2E_TEST=ON -DWAMR_BUILD_PLATFORM=linux
ctest -L e2e --output-on-failure
```

### 5.4 Regression Tests

**Existing Test Suites**: Continue to run without modification

- `tests/wamr-test-suites/` - Comprehensive spec tests
- `tests/unit/` - Component unit tests (independent build)

**Validation**: All existing tests must pass with new implementation

### 5.5 CI Integration

**Updated Workflows**:
- `.github/workflows/compilation_on_android_ubuntu.yml`
- `.github/workflows/compilation_on_macos.yml`

**Test Matrix**:
- Ubuntu 22.04: GC enabled, GC disabled
- macOS 13 (Intel): GC enabled, GC disabled
- macOS 14 (ARM): GC enabled, GC disabled

**New CI Jobs**:
- `end-to-end-tests-ubuntu`: E2E tests on Linux
- `end-to-end-tests-macos`: E2E tests on macOS

---

## Implementation Plan

### 6.1 Implementation Order (TDD)

#### Phase 1: Data Structure Preparation
**Duration**: 30 minutes

1. Modify `core/iwasm/include/wasm_export.h`
   - Add `enable_gc` field to `LoadArgs`
   - Add comprehensive comments

2. Modify `core/iwasm/interpreter/wasm.h`
   - Add `is_gc_enabled` field to `WASMModule` (inside `#if WASM_ENABLE_GC`)

**Deliverable**: Compilable code with new fields

#### Phase 2: Write Failing Tests
**Duration**: 2 hours

1. Create `tests/unit/gc-loader/test_ref_null_parsing.cc`
   - Configuration validation tests
   - Non-GC mode parsing tests
   - GC mode parsing tests

2. Create `tests/end2end/gc-ref-null/` infrastructure
   - CMakeLists.txt with 8 test cases
   - run-test.sh script
   - 6 WAT test case files

3. Run tests: All should fail (functionality not implemented)

**Deliverable**: Comprehensive test suite (failing)

#### Phase 3: Implement LoadArgs Propagation
**Duration**: 1 hour

1. Modify `core/iwasm/common/wasm_runtime_common.c`
   - `wasm_runtime_load_ex()`: Pass `load_args` to `wasm_load()`

2. Modify `core/iwasm/interpreter/wasm_loader.c`
   - `wasm_load()`: Pass `load_args` to `wasm_load_from_sections()`
   - `wasm_load_from_sections()`: Add `load_args` parameter
   - Add configuration conflict check
   - Set `module->is_gc_enabled`

**Deliverable**: LoadArgs reaches module loader

#### Phase 4: Implement ref.null Parsing
**Duration**: 3 hours

1. **Location 1**: `load_init_expr()` (line ~1000)
   - Refactor: GC branch + shared non-GC code
   - Add error messages

2. **Location 2**: `wasm_loader_resolve_functions()` (line ~7848)
   - Refactor: GC branch + shared non-GC code
   - Ensure correct byte skipping

3. **Location 3**: `wasm_loader_prepare_bytecode()` (line ~13662)
   - Refactor: GC branch + shared non-GC code
   - Add type stack validation

**Deliverable**: Complete ref.null parsing implementation

#### Phase 5: Run Tests
**Duration**: 1 hour

1. Run unit tests: Should pass
2. Run E2E tests: Should pass
3. Run regression tests: Should pass
4. Fix any failures

**Deliverable**: All tests passing

#### Phase 6: wamrc Integration
**Duration**: 30 minutes

1. Modify `wamr-compiler/main.c`
   - Use `wasm_runtime_load_ex()` with `LoadArgs`
   - Pass `option.enable_gc` to `LoadArgs.enable_gc`

2. Test wamrc manually:
   ```bash
   wamrc --enable-gc -o gc.aot gc.wasm
   wamrc -o non-gc.aot non-gc.wasm
   ```

**Deliverable**: wamrc CLI integration complete

#### Phase 7: Documentation
**Duration**: 2 hours

1. Update `core/iwasm/include/wasm_export.h` comments
2. Update `doc/build_wamr.md`
3. Update `wamr-compiler/main.c` help text
4. Create `tests/end2end/README.md`
5. Add inline code comments

**Deliverable**: Complete documentation

### 6.2 Estimated Code Changes

| File | Lines Added | Lines Modified | Lines Deleted |
|------|-------------|----------------|---------------|
| wasm_export.h | 15 | 0 | 0 |
| wasm.h | 4 | 0 | 0 |
| wasm_loader.c | 30 | 60 | 40 |
| wasm_runtime_common.c | 5 | 3 | 0 |
| wamr-compiler/main.c | 8 | 3 | 1 |
| test_ref_null_parsing.cc | 200 | 0 | 0 |
| E2E test infrastructure | 300 | 0 | 0 |
| **Total** | **562** | **66** | **41** |

**Net Change**: ~587 lines (including tests)

### 6.3 Timeline

**Total Estimated Time**: 10 hours

- Phase 1 (Data structures): 0.5 hours
- Phase 2 (Tests): 2 hours
- Phase 3 (Propagation): 1 hour
- Phase 4 (Parsing): 3 hours
- Phase 5 (Testing): 1 hour
- Phase 6 (wamrc): 0.5 hours
- Phase 7 (Documentation): 2 hours

**Recommended Schedule**: 2 days with review checkpoints

---

## Documentation Updates

### 7.1 API Documentation

**File**: `core/iwasm/include/wasm_export.h`

Add detailed JSDoc-style comments for:
- `LoadArgs` structure
- `LoadArgs.enable_gc` field
- Usage examples

### 7.2 Build Documentation

**File**: `doc/build_wamr.md`

Add new section:
```markdown
### GC (Garbage Collection) Support

#### Compile-time Configuration

Enable GC support at build time:

```bash
cmake -DWASM_ENABLE_GC=1 ...
```

This enables:
- GC-specific data structures and opcodes
- Runtime selection of GC/non-GC parsing modes

#### Runtime Configuration

When loading modules, specify the parsing mode:

```c
LoadArgs args = { .enable_gc = true };
wasm_module_t module = wasm_runtime_load_ex(
    wasm_buf, wasm_size, &args, error_buf, error_buf_size);
```

#### Mode Interaction Table

[Include table from section 2.2]
```

### 7.3 wamrc Help Text

**File**: `wamr-compiler/main.c`

Update help text for `--enable-gc`:
```c
printf("  --enable-gc               Enable GC mode: parse ref.null as GC-encoded\n");
printf("                            (type indices/abstract heap types) instead of\n");
printf("                            simple funcref/externref. Only use this for\n");
printf("                            modules compiled with GC proposal support.\n");
```

### 7.4 Architecture Documentation

**File**: `doc/architecture-overview.md`

Add subsection to "Module Loading & Execution":
```markdown
**GC Mode Selection**

The loader supports both GC and non-GC encoding for `ref.null`:
- Set via `LoadArgs.enable_gc` at load time
- Stored in `module->is_gc_enabled` for parsing
- Enables runtime selection when `WASM_ENABLE_GC=1`
```

### 7.5 End-to-End Test Documentation

**File**: `tests/end2end/README.md`

Complete user guide covering:
- When to use E2E tests vs unit tests
- How to add new test suites
- How to run tests locally and in CI
- Debugging failed tests
- Best practices

---

## Risks and Mitigations

### 8.1 API Breaking Change

**Risk**: Adding field to `LoadArgs` breaks ABI compatibility

**Mitigation**:
- `LoadArgs` is passed by pointer, not value
- New field at end of structure
- Default initialization preserves existing behavior
- `wasm_runtime_load()` wrapper maintains compatibility

**Impact**: Low - structure extension is common pattern

### 8.2 Performance Impact

**Risk**: Runtime branch adds overhead to ref.null parsing

**Mitigation**:
- Branch only exists when `WASM_ENABLE_GC=1`
- Non-GC builds have zero overhead (compile-time eliminated)
- GC builds: one predictable branch per ref.null (minimal cost)
- Modern CPUs handle this efficiently with branch prediction

**Impact**: Negligible - ref.null is not a performance-critical hotspot

### 8.3 Code Duplication

**Risk**: Three parsing locations could diverge

**Mitigation**:
- Unified code pattern applied to all three locations
- E2E tests validate consistency across locations
- Code review focuses on pattern adherence

**Impact**: Low - clear pattern + tests prevent divergence

### 8.4 Test Maintenance

**Risk**: E2E test infrastructure adds maintenance burden

**Mitigation**:
- Well-documented test framework (README.md)
- Clear naming conventions
- CTest integration for discoverability
- CI automation prevents regressions

**Impact**: Low - one-time setup cost, long-term benefit

### 8.5 Backward Compatibility

**Risk**: Existing code breaks

**Mitigation**:
- `wasm_runtime_load()` unchanged (calls `load_ex` with defaults)
- All existing tests continue to pass
- Default `enable_gc=false` preserves current behavior

**Impact**: None - fully backward compatible

### 8.6 Edge Cases

**Risk**: Unforeseen byte sequence interpretations

**Mitigation**:
- Comprehensive test matrix covering boundary conditions
- Cross-mode tests verify mode detection
- Spec compliance tests continue to run

**Impact**: Low - exhaustive testing catches edge cases

---

## Appendix A: Alternative Approaches Considered

### A.1 Global Runtime Configuration

**Approach**: Use `RuntimeInitArgs` to set global GC mode

**Pros**:
- No API change to LoadArgs
- Simple implementation

**Cons**:
- Global state prevents mixed loading
- Cannot load GC and non-GC modules in same process
- Less flexible

**Rejected**: Inferior flexibility

### A.2 Remove --enable-gc Option

**Approach**: Make GC mode purely compile-time

**Pros**:
- Simplest implementation
- Zero runtime overhead

**Cons**:
- Requires two wamrc binaries (wamrc, wamrc-gc)
- Poor user experience
- CI/test matrix explosion
- Ecosystem fragmentation

**Rejected**: Unacceptable user experience

### A.3 Automatic Detection

**Approach**: Heuristically detect GC mode from byte patterns

**Pros**:
- No user configuration needed

**Cons**:
- Cannot reliably distinguish `0x70` (funcref vs type 112)
- False positives/negatives inevitable
- Adds complexity without solving core problem

**Rejected**: Fundamentally unsolvable ambiguity

---

## Appendix B: WebAssembly GC Spec References

- **GC Proposal**: https://github.com/WebAssembly/gc
- **Reference Types**: https://github.com/WebAssembly/reference-types
- **Heap Type Encoding**: https://webassembly.github.io/gc/core/binary/types.html#heap-types

---

## Approval and Sign-off

- [ ] Design reviewed by user
- [ ] Technical approach validated
- [ ] Test strategy approved
- [ ] Documentation plan confirmed
- [ ] Ready to proceed to implementation plan

---

**End of Design Specification**
