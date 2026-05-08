# WAMR Documentation Baseline Audit Report

**Date**: 2026-05-08  
**Auditor**: Manual baseline before optimization  
**Scope**: Entry docs + 7 core strategy docs  
**Files Scanned**: 10 documentation files

---

## Summary

- **Critical Issues**: 8+
- **Important Issues**: 5+
- **Advisory Issues**: Not counted in baseline

**Status**: ⚠️ Significant duplication found. Phase 1-3 will address all critical issues.

---

## Critical Issues (Zero Duplication Violations)

### A1.3 Execution Pattern Duplication

**Found devcontainer execution patterns in multiple files:**
- CLAUDE.md: Lines 15-22 (8 lines with command examples)
- AGENTS.md: Lines 17-52 (35 lines with detailed explanation)
- doc/testing.md: Lines 10-20 (execution pattern note)
- doc/building.md: Lines 16-20 (execution pattern note)
- doc/debugging.md: Similar pattern
- doc/dev-in-container.md: Lines 1-50+ (technical details)

**Estimated total**: ~150+ lines of duplication
**Recommendation**: AGENTS.md should be SSOT, all others link to it

### A1.4 Navigation Duplication

**Task navigation found in:**
- AGENTS.md: Lines 100-192 (complete navigation by task type)
- CLAUDE.md: Lines 4-9 (summary of what AGENTS.md contains)
- Individual doc/*.md files: Various "when to read" guidance

**Recommendation**: AGENTS.md should be sole navigation hub

### A1.2 Command Duplication

**Common duplicated commands:**
- `ctest --test-dir build --output-on-failure` in 3+ files
- `cmake -B build` variations in 5+ files
- `devcontainer exec` wrapper pattern in 8+ files

**Recommendation**: Strategy docs should show 1 decision-example, operations docs have full syntax

---

## Important Issues (Architecture Violations)

### A2.1 Layer Boundary Check

**Files exceeding line limits:**
- doc/linting.md: 916 lines (limit: 600) - exceeds by 316 lines
  - Contains detailed explanations that should link to code-quality.md
- doc/debugging.md: ~620 lines (limit: 600) - exceeds by ~20 lines

**Files within limits:**
- CLAUDE.md: 45 lines (limit: 40) - minor excess
- AGENTS.md: 193 lines (limit: 200) - OK but can be more concise
- All other core docs: Within 300-600 range

### A2.2 SSOT Violations

**Concept duplication found:**
- "Debug build" concept explained in both building.md and debugging.md
- "AOT compilation" explained in architecture-overview.md, building.md, and wamr-compiler/README.md
- "Container execution" explained in CLAUDE.md, AGENTS.md, dev-in-container.md

**Recommendation**: Establish clear SSOT per concept as defined in SSOT Registry

---

## Metrics (Baseline)

**Documentation size:**
- CLAUDE.md: 45 lines
- AGENTS.md: 193 lines
- 7 core doc/*.md: 3,692 lines total
- Total files in scope: ~3,930 lines

**Duplication estimates:**
- Execution pattern mentions: ~60+ occurrences
- Concept definition duplicates: ~15+ cases
- Command syntax duplicates: ~80+ instances

**Target after Phase 1-3:**
- Total lines: ~2,800 (-28%)
- Execution pattern mentions: 1 (AGENTS.md only)
- Concept duplicates: 0
- Command duplicates: Minimal (decision-level examples only)

---

## Next Steps

**Phase 2:** Refactor CLAUDE.md and AGENTS.md to eliminate top-layer duplication

**Phase 3:** Optimize 7 core doc/*.md files using standard structure

**Phase 4:** Full audit and optimization of remaining docs

**Next Audit**: After Phase 2 completion (~2026-05-10)
