# WAMR Documentation Optimization Design Specification

**Date**: 2026-05-08  
**Status**: Approved  
**Implementation**: 4 Phases over 1.5-2 weeks

---

## Executive Summary

This specification defines a comprehensive optimization of WAMR's documentation system to eliminate duplication, establish clear boundaries between documentation layers, and create a sustainable audit mechanism for long-term documentation quality.

**Key Goals**:
1. Achieve strict zero duplication across all documentation
2. Establish clear three-layer documentation architecture
3. Create AI-powered periodic audit checklist
4. Reduce documentation size by 15-25% while improving clarity

**Approach**: Progressive optimization across 4 phases, starting with rules establishment and ending with full validation and sustainable mechanisms.

---

## Problem Statement

### Current Issues

**A. CLAUDE.md duplicates AGENTS.md content**
- Both files explain devcontainer requirements
- Redundant navigation guidance
- Unclear which is authoritative

**B. AGENTS.md, doc/dev-in-container.md boundary unclear**
- Container execution patterns explained in multiple places
- ~50-75 lines of duplication across files

**C. Workflow navigation scattered**
- AGENTS.md, doc/testing.md, doc/building.md all contain task guidance
- No single navigation authority

**D. doc/*.md contains operational details**
- Should be decision-level guidance only
- Contains multiple command variations that belong in component READMEs

**E. Commands duplicated across documents**
- Same commands shown in 3+ locations
- Execution patterns (`devcontainer exec`) repeated ~60+ times
- Context window waste

**F. No systematic documentation quality checks**
- Problems accumulate over time
- No checklist for reviewing documentation changes
- Manual detection of duplication issues

---

## Design Principles

### 1. Strict Zero Duplication

**Definition**: Every piece of information has exactly ONE authoritative location.

**Rules**:
- ✅ Allowed: Mention concept name + link to definition
- ✅ Allowed: 1 command example showing decision outcome
- ❌ Forbidden: Explain same concept twice
- ❌ Forbidden: Show same command syntax twice
- ❌ Forbidden: Duplicate "when to use" guidance

### 2. Three-Layer Architecture

```
Layer 0: Tool Entry (CLAUDE.md)
└─ Tool-specific rules + pointer to AGENTS.md
   ↓
Layer 1: Navigation Hub (AGENTS.md)
└─ Project overview + structure + task navigation
   ↓
Layer 2: Strategy & Guidance (doc/*.md)
└─ Concepts + decisions + minimal examples + links
   ↓
Layer 3: Operations (component READMEs)
└─ Complete commands + all options + troubleshooting
```

### 3. Single Source of Truth (SSOT)

Each information type has exactly one authoritative source:

| Information Type | SSOT Location | Others Must |
|-----------------|---------------|-------------|
| Tool usage rules | CLAUDE.md | Link to CLAUDE.md |
| Execution patterns | AGENTS.md § Command Execution Pattern | Link to AGENTS.md |
| Task navigation | AGENTS.md § Navigation | Link to AGENTS.md |
| Build concepts | doc/building.md | Link to building.md |
| Test concepts | doc/testing.md | Link to testing.md |
| Core concepts (AOT, WASI) | doc/architecture-overview.md | Link to architecture-overview.md |
| Component commands | component/*/README.md | Link to specific README |

### 4. Decision-Level Command Pattern

**doc/*.md files should contain**:
- Concepts (What/Why)
- Decision guidance (When/Which)
- 1-3 command examples per major option
- Links to detailed command references

**doc/*.md files should NOT contain**:
- All command parameters
- Multiple command variations (> 3)
- Detailed troubleshooting
- Step-by-step operational procedures

---

## Target Architecture

### CLAUDE.md Structure

**Purpose**: Claude Code tool-specific rules  
**Length**: < 30 lines  
**Content**:
```markdown
# Claude Code Instructions for WAMR

**Start here**: [AGENTS.md](./AGENTS.md)

## Critical Rules

1. Linux requires devcontainer - See [AGENTS.md § Execution](...)
2. Pre-commit checklist - See [linting.md](...)
3. Follow documentation principles - See [documentation-principles.md](...)

**Next**: Open [AGENTS.md](./AGENTS.md) for navigation
```

**Removes**:
- Devcontainer command examples
- "Why" explanations
- Pre-commit details
- AGENTS.md content summary

### AGENTS.md Structure

**Purpose**: Project navigation hub  
**Length**: 100-150 lines  
**Content**:
```markdown
# AI Agent Guide for WAMR Development

## Project Overview
[3-5 sentences + component list]

## Project Structure
[Tree diagram]

## Command Execution Pattern
[Platform-specific execution rules - SSOT]
[Examples table]

## Documentation Navigation
[Task type → docs to read]

## Quick Reference
[5-7 most frequent docs]
```

**Removes**:
- Detailed devcontainer explanations (→ dev-in-container.md)
- "What AI Agents Can Help With" section
- Long documentation organization explanation
- "Getting Started: Read This First" (redundant)
- Extensive quick reference lists

### doc/*.md Standard Structure

**Every doc/*.md follows**:
```markdown
# [Topic]

[2-3 sentence purpose]

**Prerequisites**: [Optional links]

> **Execution**: See [AGENTS.md § Execution](...)

---

## [Concept/Topic 1]

### What Is It?
[2-3 sentences]

### Why [Topic]?
[Benefits]

### When to Use
[Scenarios]

### Decision Guide
[Table or tree]

### Quick Example
[1 command showing decision outcome]

**→ Complete guide**: [component/README.md](...)

---

[Repeat for other concepts]

---

**Related**: See [AGENTS.md § Navigation](...) or 2-3 most relevant links
```

**Standard elements**:
- ✅ Minimal execution note (1 line + link to AGENTS.md)
- ✅ What/Why/When structure
- ✅ Decision tables/trees
- ✅ 1 example per major option
- ✅ Clear links to operational guides
- ✅ 300-600 lines total

**Removes from current docs**:
- ❌ Detailed execution pattern explanations (10-15 lines)
- ❌ Multiple command variations
- ❌ Concept definitions from other domains
- ❌ Long related documentation lists

---

## Phase 1: Rules & Checklist

### Objectives

1. Establish complete documentation rules
2. Create AI-powered audit checklist
3. Verify checklist effectiveness

### Tasks

#### 1.1 Update documentation-principles.md

**Add new sections**:

**A. "Zero Duplication Rules"**
- What counts as duplication (4 types)
- What is NOT duplication (3 exceptions)
- Single Source of Truth Registry (table)

**B. "Periodic Documentation Audit Checklist"**
- Purpose and usage pattern
- 4 categories of checks:
  - Category 1: Zero Duplication (Critical) - 4 items
  - Category 2: Information Architecture (Important) - 4 items
  - Category 3: Content Quality (Important) - 3 items
  - Category 4: Maintenance (Advisory) - 3 items
- Audit output format (markdown template)

**Update existing sections**:
- Strengthen "Think Once, Document Once" with examples
- Add duplication detection techniques
- Clarify layer boundaries

#### 1.2 Verify Checklist

**Process**:
1. Run checklist manually against current documentation
2. Generate baseline audit report
3. Verify checklist detects known issues:
   - CLAUDE.md/AGENTS.md duplication
   - Execution pattern duplication
   - Command duplication in doc/*.md
4. Adjust checklist items if needed

**Success criteria**:
- ✅ Checklist detects all 5 known problem types (A-E)
- ✅ Baseline report generated
- ✅ AI tool can execute checklist

### Deliverables

- Updated documentation-principles.md with:
  - Zero Duplication Rules section
  - SSOT Registry
  - Audit Checklist (14 items)
- Baseline audit report
- Checklist validation confirmation

### Timeline

**Duration**: 1-2 days

---

## Phase 2: Top Layer Refactor

### Objectives

1. Eliminate CLAUDE.md and AGENTS.md duplication
2. Establish clear entry layer hierarchy
3. Make AGENTS.md the SSOT for execution patterns and navigation

### Tasks

#### 2.1 Refactor CLAUDE.md

**Target**: < 30 lines

**Structure**:
```markdown
# Claude Code Instructions for WAMR

**Start here**: [AGENTS.md](./AGENTS.md)

## Critical Rules

### 1. Linux Development Requires Devcontainer
[2 sentences + link to AGENTS.md § Execution]

### 2. Pre-commit Checklist Required
[1 sentence + link to linting.md]

### 3. Follow Documentation Principles
[1 sentence + link to documentation-principles.md]

---

**Next**: [AGENTS.md](./AGENTS.md)
```

**Remove**:
- All devcontainer command examples
- "Why" explanations (move to dev-in-container.md if needed)
- "AGENTS.md contains" list
- Detailed pre-commit content

**Changes**: 45 lines → ~25 lines (-44%)

#### 2.2 Refactor AGENTS.md

**Target**: 100-150 lines

**New structure**:

1. **Project Overview** (10 lines)
   - 3-5 sentence description
   - Component list
   - Link to README.md

2. **Project Structure** (10 lines)
   - Tree diagram
   - Brief explanation

3. **Command Execution Pattern** (30 lines)
   - Critical rule (Linux requires container)
   - Pattern explanation
   - Examples table
   - Link to dev-in-container.md for details
   - **This is the SSOT for execution patterns**

4. **Documentation Navigation** (60 lines)
   - Lazy loading principle (1 sentence + link)
   - By Task Type sections:
     - Bug Fixes (5 docs)
     - Adding Features (6 docs)
     - PR Reviews (3 docs)
     - Test Writing (4 docs)
     - Refactoring (4 docs)

5. **Quick Reference** (10 lines)
   - 5-7 most frequent docs only
   - Link to doc/ for all docs

**Remove**:
- "What AI Agents Can Help With" section
- Long documentation organization explanation
- "Getting Started: Read This First" (redundant with Navigation)
- Extensive quick reference lists
- Detailed devcontainer technical content

**Changes**: 193 lines → ~130 lines (-33%)

#### 2.3 Update doc/dev-in-container.md

**Clarify boundary with AGENTS.md**:
- AGENTS.md: Basic execution rules (how to wrap commands)
- dev-in-container.md: Container technical details (image, VS Code, troubleshooting)

**Remove from dev-in-container.md**:
- Basic command wrapping patterns (now in AGENTS.md only)

**Keep in dev-in-container.md**:
- Container image details
- VS Code devcontainer integration
- Building custom images
- Advanced container configuration
- Container troubleshooting

#### 2.4 Validation

**Run audit checklist items**:
- ✅ A1.3: Execution pattern duplication (should be 0)
- ✅ A1.4: Navigation duplication (should be 0)
- ✅ A2.1: Layer boundary check (CLAUDE.md < 40, AGENTS.md < 200)
- ✅ A2.2: SSOT validation for execution patterns

**Manual verification**:
- User can find needed docs from AGENTS.md
- CLAUDE.md → AGENTS.md flow is clear
- No confusion about where execution patterns are documented

### Deliverables

- New CLAUDE.md (~25 lines)
- New AGENTS.md (~130 lines)
- Updated doc/dev-in-container.md (boundary clarified)
- Phase 2 audit report

### Timeline

**Duration**: 0.5-1 day

---

## Phase 3: Strategy Layer Optimization

### Objectives

1. Optimize 7 core high-frequency documents
2. Eliminate all duplication between them
3. Establish standard structure for doc/*.md
4. Reduce total lines by ~900 (24%)

### Target Documents (Priority Order)

1. **doc/architecture-overview.md** - Core concepts SSOT
2. **doc/building.md** - Build concepts SSOT
3. **doc/testing.md** - Test concepts SSOT
4. **doc/debugging.md** - Debug concepts SSOT
5. **doc/dev-in-container.md** - Container details (boundary with AGENTS.md)
6. **doc/code-quality.md** - Quality standards
7. **doc/linting.md** - Pre-commit checklist

### Standard Optimization Process

**For each document**:

#### Step 1: Apply Standard Header
```markdown
# [Topic]

[2-3 sentence purpose and scope]

**Prerequisites**: [If needed]
1. [AGENTS.md](../AGENTS.md) - Navigation
2. [Other prereq](./other.md) - Why needed

> **Execution**: Commands in pure form. See [AGENTS.md § Command Execution Pattern](../AGENTS.md#command-execution-pattern).

---
```

#### Step 2: Remove Execution Pattern Duplication
- Delete all explanations of devcontainer execution (10-15 lines each)
- Replace with single line linking to AGENTS.md
- Ensure commands shown in pure form

#### Step 3: Apply Standard Topic Structure
```markdown
## [Topic/Concept]

### What Is It?
[2-3 sentences]

### Why [Topic]?
[2-4 bullets]

### When to Use
[Scenarios]

### Decision Guide
[Table or tree]

### Quick Example
```bash
[1 command showing decision outcome]
```

**→ Complete guide**: [path/to/README.md](...)

---
```

#### Step 4: Remove Concept Duplication
- Identify concepts explained in this doc
- Verify this is the SSOT for these concepts
- Replace any concepts from other domains with links

#### Step 5: Minimize Command Examples
- Keep 1-3 examples per major topic
- Remove detailed parameter explanations
- Remove command variations (keep in component READMEs)

#### Step 6: Simplify Related Documentation
- Remove long related doc lists
- Keep only 2-3 most relevant links, or
- Single link to AGENTS.md § Navigation

### Document-Specific Plans

#### 3.1 doc/architecture-overview.md

**Current**: 507 lines  
**Target**: ~450 lines (-11%)

**Optimizations**:
- Add standard header
- Remove execution pattern note (replace with 1 line)
- Verify SSOT for: AOT, JIT, WASI, VMcore, wasm-app, module lifecycle
- No concept duplication with building.md or testing.md
- Minimal command examples

**Key validation**: Other docs should link here for core concept definitions

#### 3.2 doc/building.md

**Current**: 372 lines  
**Target**: ~350 lines (-6%)

**Optimizations**:
- Add standard header (already relatively clean)
- Remove/minimize execution pattern note
- Verify SSOT for: build types, execution modes, feature flags, platforms
- Ensure debugging.md links here for "Debug build" concept
- Keep decision tables (good structure)

**Key validation**: No other doc explains build concepts

#### 3.3 doc/testing.md

**Current**: 531 lines  
**Target**: ~400 lines (-25%)

**Optimizations**:
- Remove lines 22-28 (execution pattern duplication)
- Standardize header
- Reduce command examples:
  - Unit tests: Keep 1 example (line 78)
  - Spec tests: Keep 1 example (lines 137-145, choose 1)
  - Regression tests: Keep 1 example (lines 217-220)
  - Integration tests: Keep 1 example (line 277)
- Remove duplicate "Test Workflow" explanations
- Simplify related docs section

**Key validation**: testing.md = test strategy, tests/*/README.md = test operations

#### 3.4 doc/debugging.md

**Current**: ~620 lines  
**Target**: ~480 lines (-23%)

**Optimizations**:
- Remove execution pattern duplication
- Link to building.md § Build Types for "Debug build" concept
- Don't re-explain what Debug builds are
- Reduce GDB command examples (keep 2-3 key ones)
- Focus on debugging strategy, not GDB tutorial
- Link to GDB documentation for full reference

**Key validation**: No duplication with building.md on build concepts

#### 3.5 doc/dev-in-container.md

**Current**: 286 lines  
**Target**: ~250 lines (-13%)

**Optimizations**:
- Remove basic execution pattern (now in AGENTS.md § Command Execution Pattern)
- Focus on:
  - Container image details
  - VS Code integration
  - Building custom images
  - Advanced configuration
  - Container troubleshooting
- Link to AGENTS.md for basic execution rules

**Key validation**: Clear boundary with AGENTS.md

#### 3.6 doc/code-quality.md

**Current**: 437 lines  
**Target**: ~350 lines (-20%)

**Optimizations**:
- Clarify boundary with linting.md:
  - code-quality.md = standards and principles (why)
  - linting.md = execution checklist (what to run)
- Remove execution pattern duplication
- Reduce command examples
- Remove any duplication with linting.md

**Key validation**: No overlap with linting.md

#### 3.7 doc/linting.md

**Current**: ~916 lines  
**Target**: ~500 lines (-45%)

**Optimizations**:
- Keep checklist format (this is correct)
- For each checklist item:
  - Keep: What to check
  - Keep: 1 command to run
  - Remove: Detailed explanation of why (link to code-quality.md)
  - Remove: All command variations
  - Remove: Detailed troubleshooting
- Link each item to detailed guide
- Remove execution pattern duplication

**Example transformation**:
```markdown
# Before (30 lines per item)
## Code Formatting

Why formatting matters: [10 lines]

How to check:
clang-format --version
clang-format -i file.c
clang-format -i --style=file file.c
[15 lines of variations]

Troubleshooting: [5 lines]

# After (5 lines per item)
## Code Formatting

Run: `clang-format -i $(find . -name '*.[ch]')`

See [code-quality.md § Formatting](./code-quality.md#formatting) for standards.
```

**Key validation**: Remains usable as quick checklist, but not duplicating details

### Validation Per Document

**After optimizing each doc, verify**:
- ✅ Standard header applied
- ✅ Execution pattern appears only as 1-line link
- ✅ No concept duplication with other optimized docs
- ✅ Command examples ≤ 3 per major topic
- ✅ Links to operational guides present
- ✅ Line count within target range

### Phase 3 Validation

**Run full audit checklist on 7 documents**:
- ✅ A1.1: Concept duplication (should be 0 between these 7)
- ✅ A1.2: Command duplication (should be minimal)
- ✅ A1.3: Execution pattern duplication (should be 0)
- ✅ A2.1: Layer boundary check (all 300-600 lines except linting.md)
- ✅ A2.2: SSOT validation
- ✅ A3.1: Command form consistency
- ✅ A3.2: Decision-level pattern

**Manual verification**:
- Read through optimized docs for flow and clarity
- Verify links work and point to right content
- Ensure no critical information was lost (only moved)

### Deliverables

- 7 optimized core documents
- Phase 3 audit report
- Line reduction: ~900 lines (-24% across 7 docs)

### Timeline

**Duration**: 2-3 days  
**Breakdown**:
- Day 1: architecture-overview.md, building.md, testing.md
- Day 2: debugging.md, dev-in-container.md, code-quality.md
- Day 3: linting.md, validation

---

## Phase 4: Full Validation & Mechanisms

### Objectives

1. Validate optimization across ALL documentation
2. Fix remaining issues in other doc/*.md files
3. Finalize SSOT Registry
4. Establish sustainable audit mechanism

### Tasks

#### 4.1 Full Documentation Audit

**Scope**: All .md files in repository

**Process**:
1. Run complete audit checklist
2. Generate comprehensive audit report
3. Identify all remaining issues:
   - Critical (Zero Duplication)
   - Important (Architecture)
   - Advisory (Maintenance)

**Expected findings**:
- Phase 1-3 docs should have 0 critical issues
- Other doc/*.md may have issues
- Component READMEs may have minor issues

#### 4.2 Fix Remaining doc/*.md Issues

**Priority 1 (High-frequency, API docs)**:
- doc/embed_wamr.md - Embedding API
- doc/export_native_api.md - Native function API
- doc/build_wamr.md - Complete CMake reference

**Priority 2 (Performance/memory)**:
- doc/perf_tune.md - Performance tuning
- doc/memory_tune.md - Memory optimization

**Priority 3 (Specialized topics)**:
- doc/source_debugging.md - Source-level debugging
- doc/build_wasm_app.md - Building Wasm apps
- doc/port_wamr.md - Porting guide
- Other specialized docs

**Process for each**:
1. Run checklist
2. Apply standard optimizations:
   - Standard header
   - Remove execution pattern duplication
   - Remove concept duplication
   - Minimize command examples
3. Verify with checklist

**Note**: May not complete all Priority 3 docs if issues are minor

#### 4.3 Update SSOT Registry

**Finalize Single Source of Truth Registry** in documentation-principles.md

**Process**:
1. Review all optimized documents
2. Identify all major information types
3. Document authoritative source for each
4. Create comprehensive SSOT table

**Example additions to registry**:
| Information Type | SSOT | Others Must |
|-----------------|------|-------------|
| Embedding API concepts | doc/embed_wamr.md | Link to embed_wamr.md |
| Native API registration | doc/export_native_api.md | Link to export_native_api.md |
| CMake flag reference | doc/build_wamr.md | Link to build_wamr.md for complete list |
| GDB debugging workflow | doc/debugging.md | Link to debugging.md |
| Performance tuning strategy | doc/perf_tune.md | Link to perf_tune.md |

#### 4.4 Establish Periodic Audit Mechanism

**Create documentation audit process**:

**1. Document the process** in documentation-principles.md:
```markdown
## Periodic Audit Process

**Frequency**: Monthly (or quarterly for mature documentation)

**Executor**: AI tool (Claude Code, custom script)

**Process**:
1. Run audit checklist against all .md files
2. Generate audit report
3. Create GitHub issue for findings
4. Assign to documentation maintainer
5. Fix critical issues within 1 week
6. Fix important issues within 1 month

**Tools**:
- Manual: Use checklist in documentation-principles.md
- Automated: [Link to audit script if created]
```

**2. Create audit execution guide**:
```markdown
### Running Documentation Audit

**Using AI (Claude Code)**:
```bash
claude-code audit-docs --checklist=doc/documentation-principles.md
```

**Manual execution**:
1. Open documentation-principles.md § Audit Checklist
2. For each category, check all items
3. Record findings in audit report template
4. Generate summary with counts

**Output**: docs/audits/YYYY-MM-DD-audit-report.md
```

**3. Optional: Create automation script**:

Simple script for automatic detection:
```bash
#!/bin/bash
# scripts/audit-docs.sh

echo "# Documentation Audit - $(date +%Y-%m-%d)"
echo

echo "## A1.2 Command Duplication Scan"
# Find commands that appear in multiple files
grep -rh '^\s*```bash' doc/*.md | sort | uniq -c | sort -rn | head -20

echo "## A1.3 Execution Pattern Duplication"
# Count devcontainer exec appearances
grep -r "devcontainer exec" doc/*.md AGENTS.md CLAUDE.md | wc -l

echo "## A2.1 Line Count Check"
# Check doc/*.md line counts
wc -l doc/*.md | awk '$1 > 600 { print $2, "exceeds 600 lines:", $1 }'
```

**4. Optional: CI integration**:

If desired, add to GitHub Actions:
```yaml
# .github/workflows/doc-audit.yml
name: Documentation Audit
on:
  schedule:
    - cron: '0 0 1 * *'  # Monthly
  workflow_dispatch:

jobs:
  audit:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Run audit script
        run: bash scripts/audit-docs.sh > audit-report.md
      - name: Create issue if problems found
        # Create GitHub issue with audit results
```

#### 4.5 Create Audit Report Archive

**Set up audit history**:
```bash
mkdir -p docs/audits/
```

**Store audit reports**:
- docs/audits/2026-05-08-baseline-audit.md (from Phase 1)
- docs/audits/2026-05-10-phase2-audit.md
- docs/audits/2026-05-13-phase3-audit.md
- docs/audits/2026-05-15-final-audit.md

**Future audits** go in same directory with date prefix

### Validation Criteria

**Phase 4 complete when**:
- ✅ Full audit report generated
- ✅ All Critical issues resolved (A1.x categories)
- ✅ All Important issues resolved (A2.x, A3.x categories)
- ✅ SSOT Registry complete and comprehensive
- ✅ Audit process documented
- ✅ At least one successful audit execution
- ✅ Audit mechanism established (manual or automated)

### Deliverables

- Comprehensive audit report for all documentation
- Optimized remaining doc/*.md files (Priority 1 & 2)
- Complete SSOT Registry in documentation-principles.md
- Documented periodic audit process
- Audit script (optional)
- CI integration (optional)
- Audit report archive

### Timeline

**Duration**: 2-3 days  
**Breakdown**:
- Day 1: Full audit + Priority 1 docs (embed_wamr.md, export_native_api.md, build_wamr.md)
- Day 2: Priority 2 docs + SSOT Registry finalization
- Day 3: Audit mechanism setup + validation + documentation

---

## Success Metrics

### Quantitative Targets

**Documentation size**:
- ✅ Overall: 15-25% reduction in total .md file lines
- ✅ CLAUDE.md: 45 → 25 lines (-44%)
- ✅ AGENTS.md: 193 → 130 lines (-33%)
- ✅ 7 core docs: ~3,700 → ~2,800 lines (-24%)

**Duplication metrics**:
- ✅ Execution pattern mentions: ~60 → 1 (AGENTS.md only)
- ✅ Concept definition duplicates: X → 0
- ✅ Command syntax duplicates: Y → minimal (decision examples only)

**Audit results**:
- ✅ Critical issues (A1.x): Start → 0
- ✅ Important issues (A2.x, A3.x): Start → 0
- ✅ Advisory issues (A4.x): Documented for future attention

### Qualitative Targets

**Information architecture**:
- ✅ Every concept has exactly one definition source
- ✅ Clear boundaries between all documentation layers
- ✅ SSOT Registry covers all major information types
- ✅ No ambiguity about where information belongs

**User experience**:
- ✅ Users can find needed docs via AGENTS.md navigation
- ✅ Documentation flow is logical (high-level → detailed)
- ✅ Links connect related information effectively
- ✅ No dead-end documents (all reachable from AGENTS.md)

**Maintainability**:
- ✅ Maintainers can determine where new info belongs (via SSOT Registry)
- ✅ Changes to one doc don't require updates to multiple docs
- ✅ Periodic audit can catch quality regressions
- ✅ Clear process for reviewing documentation changes

**AI tool effectiveness**:
- ✅ AI can execute audit checklist
- ✅ AI audit reports are actionable
- ✅ AI can determine where information belongs using SSOT Registry

---

## Timeline & Milestones

### Overall Timeline

```
Week 1:
├─ Mon-Tue: Phase 1 (Rules & Checklist)
│           M1: Checklist validated ✓
├─ Wed:     Phase 2 (CLAUDE.md + AGENTS.md)
│           M2: Entry layer optimized ✓
└─ Thu-Fri: Phase 3 Start (architecture, building, testing)

Week 2:
├─ Mon:     Phase 3 Continue (debugging, dev-in-container, code-quality)
├─ Tue:     Phase 3 Complete (linting.md, validation)
│           M3: Core docs optimized ✓
├─ Wed-Thu: Phase 4 (Full audit + remaining docs)
└─ Fri:     Phase 4 Complete (Mechanisms + validation)
            M4: System complete ✓

Total: 8-10 working days (1.5-2 weeks)
```

### Milestones

**M1: Checklist Validated** (End of Phase 1)
- documentation-principles.md updated with checklist
- Baseline audit completed
- Checklist proven to detect known issues

**M2: Entry Layer Optimized** (End of Phase 2)
- CLAUDE.md and AGENTS.md refactored
- Zero duplication between them
- Execution pattern SSOT established in AGENTS.md

**M3: Core Docs Optimized** (End of Phase 3)
- 7 high-frequency docs optimized
- ~900 lines removed
- Zero critical issues in core docs

**M4: System Complete** (End of Phase 4)
- All documentation audited
- SSOT Registry complete
- Sustainable audit mechanism established
- All critical and important issues resolved

---

## Risk Management

### Identified Risks

| Risk | Probability | Impact | Mitigation |
|------|------------|--------|------------|
| **Break existing links during refactor** | Medium | High | Validate all links at end of each phase |
| **Remove content users need** | Low | High | Move content, don't delete; verify in final audit |
| **Boundary definitions cause confusion** | Medium | Medium | Clear SSOT Registry; examples in principles doc |
| **AI tool cannot execute checklist** | Low | Medium | Provide manual checklist as fallback |
| **Users can't find information** | Low | High | Strong AGENTS.md navigation; validate in Phase 4 |
| **Documentation maintainer resistance** | Low | Medium | Show metrics; emphasize maintainability benefits |
| **Scope creep (optimize all READMEs)** | Medium | Low | Strict phase boundaries; READMEs in Phase 4 only if critical |

### Mitigation Strategies

**For link breakage**:
- Test all links at end of each phase
- Use relative paths consistently
- Automated link checking if possible

**For lost content**:
- Review all deletions
- Confirm content moved to appropriate location
- Keep git history for recovery

**For confusion**:
- Document all decisions in SSOT Registry
- Provide examples in documentation-principles.md
- Open channel for questions during implementation

**For scope management**:
- Stick to defined phase objectives
- Document "nice to have" items for future
- Phase 4 focuses on critical issues only

---

## Implementation Notes

### Who Implements

**Phase 1-2**: Can be single implementer (AI agent or human with AI assistance)  
**Phase 3**: Can parallelize (3 docs per day with single implementer)  
**Phase 4**: Requires review from documentation maintainer

### Review Points

**After Phase 1**: Review checklist effectiveness  
**After Phase 2**: Review entry layer clarity  
**After Phase 3**: Review each optimized doc  
**After Phase 4**: Final review of entire system

### Rollback Strategy

**If critical issues found**:
- All changes in git, can rollback per phase
- Phase 2-4 independent: can rollback one phase without affecting others
- SSOT Registry allows quick identification of impact

### Post-Implementation

**After completion**:
- Run first official monthly audit in 30 days
- Collect user feedback on documentation discoverability
- Adjust SSOT Registry if boundary issues found
- Consider additional automation based on audit results

---

## Appendix: Detailed Examples

### Example: Standard doc/*.md Header

```markdown
# Testing WAMR

This guide explains WAMR's test types, when to use each, and how to make testing decisions. For detailed commands and troubleshooting, see component-specific test READMEs.

**Prerequisites**:
1. [AGENTS.md](../AGENTS.md) - Navigation and execution patterns
2. [building.md](./building.md) - Building iwasm and wamrc

> **Execution**: Commands shown in pure form. See [AGENTS.md § Command Execution Pattern](../AGENTS.md#command-execution-pattern).

---
```

### Example: Standard Topic Structure

```markdown
## Unit Tests

### What Are Unit Tests?

Unit tests verify WAMR's core components using Google Test (C++) and custom harnesses (C). They test individual functions and modules in isolation.

Located in: `tests/unit/`

### Why Unit Tests?

- **Fast feedback**: Run in seconds
- **Precise isolation**: Pinpoint exact bug location
- **Refactoring safety**: Ensure changes don't break functionality

### When to Use

- Adding new WAMR features
- Modifying runtime logic
- Fixing component bugs
- Before committing code

### Quick Example

```bash
cd tests/unit && cmake -S . -B build && cmake --build build && ctest --test-dir build
```

**→ Complete guide**: [tests/unit/README.md](../tests/unit/README.md) - all commands, options, troubleshooting

---
```

### Example: Audit Report Output

```markdown
# WAMR Documentation Audit Report

**Date**: 2026-05-15  
**Auditor**: Claude Code  
**Scope**: All .md files in repository  
**Files Scanned**: 39 documentation files

---

## Summary

- **Critical Issues**: 0
- **Important Issues**: 0
- **Advisory Issues**: 3

**Status**: ✅ Documentation system meets all critical and important quality standards.

---

## Critical Issues (Zero Duplication)

✅ **No critical issues found**

All Critical checks passed:
- ✅ A1.1: No concept duplication
- ✅ A1.2: Minimal command duplication (decision-level only)
- ✅ A1.3: Execution pattern appears only in AGENTS.md
- ✅ A1.4: Navigation guidance only in AGENTS.md

---

## Important Issues (Architecture & Quality)

✅ **No important issues found**

All Important checks passed:
- ✅ A2.1: All files within line limits
- ✅ A2.2: SSOT Registry validated
- ✅ A2.3: All links valid
- ✅ A2.4: No orphaned documents
- ✅ A3.1: Command form consistent
- ✅ A3.2: Decision-level pattern followed
- ✅ A3.3: Link phrases descriptive

---

## Advisory Issues (Maintenance)

### A4.1 Outdated Content

1. **doc/build_wamr.md:145**
   - Mentions "LLVM 10.0" but devcontainer uses LLVM 17
   - **Recommendation**: Update version references

2. **doc/linux_sgx.md:78**
   - References deprecated SGX SDK v2.9
   - **Recommendation**: Update to current SGX SDK version

### A4.3 Terminology Consistency

3. **Minor**: "AOT compiler" vs "wamrc" used inconsistently across docs
   - Most docs use "wamrc", some use "AOT compiler"
   - **Recommendation**: Standardize on "wamrc (AOT compiler)" on first mention

---

## Metrics

**Documentation size**:
- Total lines: 12,450 → 9,825 (-21%)
- CLAUDE.md: 45 → 25 lines (-44%)
- AGENTS.md: 193 → 130 lines (-33%)
- Core doc/*.md: 3,692 → 2,780 lines (-25%)

**Duplication eliminated**:
- Execution pattern mentions: 62 → 1
- Concept definition duplicates: 15 → 0
- Command syntax duplicates: 87 → 21 (decision-level examples only)

---

## Next Audit

**Scheduled**: 2026-06-15 (30 days)  
**Focus**: Advisory issues + any new changes since 2026-05-15
```

---

## Conclusion

This specification provides a complete, phased approach to optimizing WAMR's documentation system. By following these four phases, we will:

1. **Eliminate all duplication** through strict zero-duplication rules
2. **Establish clear architecture** with well-defined layer boundaries
3. **Create sustainable mechanisms** for ongoing documentation quality
4. **Improve user experience** through better navigation and information discovery

The result will be a documentation system that is:
- **Efficient**: 15-25% smaller while more comprehensive
- **Maintainable**: Clear rules prevent future duplication
- **Discoverable**: Strong navigation from entry points
- **Sustainable**: Periodic audits catch quality regressions

**Implementation can begin immediately with Phase 1.**

---

**Document Version**: 1.0  
**Status**: Approved for Implementation  
**Next Review**: After Phase 4 completion
