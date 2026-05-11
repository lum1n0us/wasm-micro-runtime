# WAMR Documentation Principles

This guide defines the organizational principles and best practices for WAMR documentation. Following these principles ensures consistency, maintainability, and efficient context window usage for both humans and AI agents.

---

## Core Principle: Think Once, Document Once

**Never duplicate the same information in multiple places.**

Each piece of information should have a single authoritative location. Other documents should link to it rather than repeating it.

**Why:**
- **Maintainability**: Updates happen in one place, not scattered across files
- **Consistency**: No risk of conflicting information
- **Context efficiency**: AI agents load only what's needed, saving tokens
- **Clarity**: Clear source of truth for every piece of information

---

## Zero Duplication Rules

**Principle: Every piece of information has exactly ONE authoritative location.**

### Duplication Detection

**What counts as duplication:**

1. **Concept Duplication**
   - Explaining the same concept (e.g., "what is AOT") in multiple files
   - ❌ Bad: Both building.md and architecture_overview.md explain AOT
   - ✅ Good: architecture_overview.md explains AOT, building.md links to it

2. **Command Duplication**
   - Showing the same command syntax in multiple places
   - ❌ Bad: `devcontainer exec --workspace-folder . -- cmake -B build` appears in 5 files
   - ✅ Good: Shown once in AGENTS.md, other files use pure syntax `cmake -B build`

3. **Instruction Duplication**
   - Repeating the same "how to do X" steps
   - ❌ Bad: Multiple files explain "how to run tests in container"
   - ✅ Good: AGENTS.md explains execution pattern once

4. **Decision Guidance Duplication**
   - Same "when to use X vs Y" advice in multiple places
   - ❌ Bad: Both testing.md and debugging.md explain when to use Debug build
   - ✅ Good: building.md owns "build type decisions", others link to it

**What is NOT duplication:**

1. **Naming without explaining**
   - Mentioning "AOT mode" without explaining what it is → OK, link to definition
   
2. **Context-specific examples**
   - testing.md shows `ctest --test-dir build` for testing context
   - building.md shows `cmake --build build` for building context
   - Different commands for different purposes → OK

3. **Layer-appropriate detail levels**
   - AGENTS.md: "Use devcontainer for Linux" (rule statement)
   - dev_in_container.md: Complete devcontainer technical details
   - Same topic, different depth → OK if no overlap in actual content

### Single Source of Truth Registry

| Information Type | Authoritative Source | All Others Must |
|-----------------|---------------------|-----------------|
| Claude Code tool usage | CLAUDE.md | Link with "See CLAUDE.md" |
| Container technical details | doc/dev_in_container.md | Link with "See dev_in_container.md" |
| Build concepts & decisions | doc/building.md | Link with "See building.md" |
| Testing concepts & decisions | doc/testing.md | Link with "See testing.md" |
| Debug concepts & decisions | doc/debugging.md | Link with "See debugging.md" |
| Core concepts (AOT, JIT, WASI) | doc/architecture_overview.md | Link with "See architecture_overview.md" |
| Component-specific commands | component/*/README.md | Link to specific README |
| Embedding API concepts | doc/embed_wamr.md | Link to embed_wamr.md |
| Native API registration | doc/export_native_api.md | Link to export_native_api.md |
| CMake flags (complete reference) | doc/build_wamr.md | Link to build_wamr.md for complete list |
| Performance tuning strategy | doc/perf_tune.md | Link to perf_tune.md |
| Memory optimization | doc/memory_tune.md | Link to memory_tune.md |
| GDB debugging workflow | doc/debugging.md | Link to debugging.md |
| Code quality standards | doc/code_quality.md | Link to code_quality.md |
| Pre-commit checklist | doc/linting.md | Link to linting.md |

---

## Documentation Hierarchy

WAMR documentation follows a hierarchical structure with progressive loading:

```
CLAUDE.md / AGENTS.md          [Entry points]
    ↓
doc/*.md                       [Strategy layer - concepts, when/why]
    ↓
component/*/README.md          [Operations layer - how, commands, examples]
    ↓
component/*/detailed-guide.md  [Deep dives - advanced topics]
```

### Level 1: Entry Points

**Files**: `CLAUDE.md`, `AGENTS.md`

**Purpose**: 
- Auto-discovered by Claude Code and other AI tools
- Provide critical rules and navigation
- Route readers to appropriate documentation

**Content**:
- Hard rules (container usage, pre-commit checks)
- Navigation guide (what to read for each task type)
- Links to doc/ for details

**Length**: 40-200 lines

**DO NOT include**:
- Detailed command examples
- Architecture explanations
- Complete workflows

### Level 2: Strategy Layer (doc/)

**Files**: `doc/testing.md`, `doc/building.md`, `doc/debugging.md`, etc.

**Purpose**:
- Explain concepts and purpose
- Guide decision-making (when to use what)
- Provide high-level workflows

**Content structure for each topic**:
```markdown
## Topic Name

### What Is It?
[2-3 sentences explaining the concept]

### Why Does WAMR Use It?
[2-3 sentences on purpose and benefits]

### When to Use
- [Bulleted list of scenarios]
- [Clear, actionable guidance]

### Quick Example
[1-3 commands showing basic pattern]

### Decision Guide
[Table or decision tree for choosing between options]

→ **See [path/to/detailed/README.md] for complete operational guide**
```

**Length**: 300-600 lines per doc

**DO include**:
- Concepts and terminology
- Purpose and rationale
- Decision trees and comparison tables
- Quick examples (1-3 commands per section)
- Links to detailed guides

**DO NOT include**:
- All command options and flags
- Multiple examples for variations
- Detailed troubleshooting
- Complete step-by-step workflows
- Implementation details

### Level 3: Operations Layer (component READMEs)

**Files**: `tests/unit/README.md`, `core/iwasm/README.md`, etc.

**Purpose**:
- Provide complete operational instructions
- Document all commands, options, examples
- Troubleshoot specific issues

**Content**:
- All command variations and flags
- Multiple examples for different scenarios
- Complete step-by-step workflows
- Detailed troubleshooting sections
- Configuration and build details
- Advanced options and customization

**Length**: No limit - as detailed as needed

**DO include**:
- Every command option explained
- Comprehensive examples
- Edge cases and advanced usage
- Component-specific troubleshooting
- Complete reference material

**DO NOT include**:
- Conceptual explanations (link to doc/ instead)
- Cross-component strategy (that's in doc/)
- General WAMR architecture (that's in doc/architecture_overview.md)

### Level 4: Deep Dives (optional)

**Files**: `doc/deep-dive-*.md`, component-specific guides

**Purpose**: Advanced topics, internals, research

**When to create**: When a topic is too detailed even for operations layer

---

## Progressive Loading Pattern

Documentation should be **progressively loadable** - readers start with high-level concepts and drill down only when needed.

### Example: Testing Documentation

**Bad (flat structure, everything in one file)**:
```
doc/testing.md (3,000 lines)
  - What tests are
  - When to use them
  - All commands for unit tests (200 lines)
  - All commands for spec tests (300 lines)
  - All commands for regression tests (250 lines)
  - Troubleshooting unit tests (100 lines)
  - Troubleshooting spec tests (150 lines)
  - ...
```
**Problem**: Must load 3,000 lines to understand basic concepts. Context window waste.

**Good (hierarchical structure)**:
```
doc/testing.md (500 lines)
  - Test types overview table
  - Each test type: what/why/when + 1 example
  - Test selection decision tree
  - Links to detailed guides

tests/unit/README.md (1,600 lines)
  - All unit test commands
  - All examples and variations
  - Unit test troubleshooting

tests/wamr-test-suites/README.md (900 lines)
  - All spec test commands
  - All examples and variations
  - Spec test troubleshooting
```
**Benefit**: Load 500 lines for concepts, drill to 1,600 lines only if doing unit testing.

### Progressive Loading Checklist

When writing or reviewing docs, ask:

- [ ] Does this doc explain concepts OR operations? (Not both)
- [ ] Are command details in the operations layer?
- [ ] Do cross-references link rather than duplicate?
- [ ] Can a reader understand the strategy without loading operational details?
- [ ] Is the doc under 600 lines if it's strategy layer?
- [ ] Does each section have "→ See [link] for details"?

---

## Separation of Concerns

### Strategy vs. Operations

**Strategy (doc/)**: Decisions, concepts, when/why
**Operations (component READMEs)**: Actions, commands, how

| Type | Strategy Layer | Operations Layer |
|------|---------------|------------------|
| **Goal** | Help decide what to do | Help execute the decision |
| **Questions answered** | What is this? Why use it? When to use it? | How to use it? What are all the options? |
| **Content** | Concepts, comparisons, decision trees | Commands, flags, examples, troubleshooting |
| **Examples** | 1-3 basic patterns | Comprehensive variations |
| **Length** | Short (save context) | As long as needed |

### Cross-References, Not Duplication

**Bad**:
```markdown
# doc/testing.md
## Unit Tests
To run unit tests:
cd tests/unit && cmake -B build ...
[30 lines of commands]

# tests/unit/README.md  
## Running Tests
To run unit tests:
cd tests/unit && cmake -B build ...
[30 lines of commands]
```

**Good**:
```markdown
# doc/testing.md
## Unit Tests
What: Tests for individual components
When: Testing specific functions
Quick example:
  cd tests/unit && ctest --test-dir build
→ See tests/unit/README.md for all commands and options

# tests/unit/README.md
## Running Tests
[Complete 200-line guide with all commands, options, examples]
```

---

## Writing Guidelines

### Concepts (Strategy Layer)

**Structure**:
1. One-sentence definition
2. 2-3 sentences on purpose
3. Bulleted "when to use" list
4. 1 quick example
5. Link to detailed guide

**Voice**: Explanatory, comparative, decision-oriented

**Example**:
```markdown
## Unit Tests

### What Are Unit Tests?
Unit tests verify individual components in isolation using Google Test (C++) 
and custom harnesses (C).

### Why Unit Tests?
- Fast feedback (seconds)
- Precise bug location
- Safe refactoring

### When to Use
- Adding new features
- Modifying runtime logic
- Before committing code

### Quick Example
cd tests/unit && ctest --test-dir build

→ **See [tests/unit/README.md](../tests/unit/README.md) for complete guide**
```

### Operations (Component READMEs)

**Structure**:
1. Quick start (minimal commands)
2. Common workflows (step-by-step)
3. Complete reference (all options)
4. Troubleshooting (specific issues)
5. Advanced topics

**Voice**: Instructional, imperative, comprehensive

**Example**:
```markdown
# Unit Tests - Complete Guide

## Quick Start
cd tests/unit
cmake -B build
cmake --build build
ctest --test-dir build

## Running Specific Tests
ctest --test-dir build -R pattern
ctest --test-dir build -E pattern
...
[50+ lines of all variations]

## Troubleshooting
### Test Fails to Build
...
### Test Times Out
...
[Detailed troubleshooting]
```

---

## Naming Conventions

### File Names

**Strategy layer (doc/)**:
- `{topic}.md` - General topic guide
- `{workflow}.md` - Workflow guide (testing, building, debugging)
- `{component}-overview.md` - High-level overviews

**Operations layer (component/)**:
- `README.md` - Main operational guide for that component
- `{specific-topic}.md` - Deep dive on specific topic

### Section Headings

Use consistent heading structures:

**Strategy layer**:
- `## {Topic Name}` (h2 for main topics)
- `### What Is {Topic}?`
- `### Why {Topic}?`
- `### When to Use {Topic}`
- `### Quick Example`

**Operations layer**:
- `## Quick Start`
- `## Common Workflows`
- `## Complete Reference`
- `## Troubleshooting`
- `## Advanced Topics`

---

## Link Strategy

### Always Link, Never Copy

When referencing information from another doc:

**Bad**:
```markdown
Platform requirements: see AGENTS.md for execution patterns
because specific platforms require devcontainer with tools like WASI-SDK.

Step-by-step setup:
1. Install Docker
2. Install VS Code  
3. Open in container
...
```

**Good**:
```markdown
```

### Link Text Best Practices

**Be specific about what the link contains**:

**Bad**: "See here for more info"
**Good**: "See [tests/unit/README.md](../tests/unit/README.md) for all commands and options"

**Bad**: "More details in the README"
**Good**: "→ Complete operational guide: [tests/unit/README.md](../tests/unit/README.md)"

**Use visual indicators**:
- `→` arrow to indicate "detailed guide is here"
- `**See [link]**` to emphasize the link
- `**→ See [link] for complete guide**` combines both

---

## Context Window Efficiency

### For AI Agents

AI agents have limited context windows. Good documentation structure helps them:

1. **Load entry points first** (CLAUDE.md, AGENTS.md) to understand project rules
2. **Load strategy layer** (doc/*.md) to understand concepts and make decisions
3. **Load operations layer** (component READMEs) only when executing specific tasks

### Token Budget Example

**Bad (flat) structure**:
```
AI reads doc/testing.md: 3,000 lines × 4 tokens/line = 12,000 tokens
Just to understand "what are the test types?"
```

**Good (hierarchical) structure**:
```
AI reads doc/testing.md: 500 lines × 4 tokens/line = 2,000 tokens
Understands test types, makes decision
Only if needed: loads tests/unit/README.md for 6,400 more tokens
```

**Savings**: 12,000 → 2,000 tokens for strategy understanding (83% reduction)

### Writing for Context Efficiency

**Core principle**: Minimize repetition, maximize information density.

#### Quantitative Guidelines

- **Strategy docs**: < 600 lines (ideally 300-500)
- **Entry points** (CLAUDE.md, AGENTS.md): < 200 lines
- **Concept-to-detail ratio**: First 100 lines should answer 80% of decisions
- **Duplication tolerance**: Zero - every piece of information documented once

#### Techniques to Reduce Context Usage

1. **Use comparison tables** instead of repetitive prose
   ```markdown
   # Bad (repetitive, 20 lines)
   Google Test is good for feature-level tests. It has rich assertions.
   Google Test supports fixtures. Google Test is C++.
   CMocka is good for function-level tests. It supports mocking.
   CMocka supports stubs. CMocka is pure C.
   ...
   
   # Good (table, 8 lines)
   | Framework | Best For | Language | Key Features |
   |-----------|----------|----------|--------------|
   | Google Test | Feature/module | C++ | Fixtures, assertions |
   | CMocka | Function-level | C | Mocks, stubs |
   ```

2. **Link to examples** instead of including all variations
   ```markdown
   # Bad (50 lines of examples)
   ctest --test-dir build -R pattern1
   ctest --test-dir build -R pattern2
   ctest --test-dir build -E exclude
   ...
   
   # Good (link, 2 lines)
   ctest --test-dir build -R <pattern>
   → See [examples section](#examples) for all patterns
   ```

3. **Single source for repeated concepts**
   ```markdown
   # Bad (repeated in 5 files, 50 lines each = 250 lines total)
   doc/testing.md: "devcontainer exec usage..."
   doc/building.md: "devcontainer exec usage..."
   tests/unit/README.md: "devcontainer exec usage..."
   ...
   
   # Good (once in AGENTS.md, 10 lines total)
   AGENTS.md: "devcontainer exec usage..."
   Savings: 240 lines (96% reduction)
   ```

4. **Progressive disclosure of complexity**
   ```markdown
   # Quick example (covers 80% of cases)
   cmake -B build && cmake --build build
   
   # Advanced options (for remaining 20%)
   → See [Advanced Build Options](#advanced)
   ```

#### Red Flags (Indicators of Context Waste)

- ⚠️ Same command pattern appears in 3+ files
- ⚠️ Phrase "as mentioned earlier/above" (should be a link)
- ⚠️ Strategy doc > 800 lines (split into strategy + operations)
- ⚠️ Explaining how to use standard tools (devcontainer, docker, git)
- ⚠️ Multiple files explaining the same workflow

#### Context Budget Monitoring

When reviewing documentation changes:

```bash
# Check for duplicated content
git diff main -- '*.md' | grep -o "devcontainer exec" | wc -l
# Should be: 0-2 (only in AGENTS.md and possibly CLAUDE.md)

# Check documentation size
wc -l doc/*.md
# Strategy docs should be < 600 lines each
```

---

## Maintenance

### When to Update Documentation

**Update immediately when**:
- Adding new features
- Changing workflows
- Fixing bugs that affect usage
- Receiving user feedback about confusion

**Don't update if**:
- It would duplicate information elsewhere (link instead)
- It's implementation detail (code comments are better)
- It's temporary workaround (issue tracker is better)

### Review Checklist

Before merging documentation changes:

**Information Architecture**:
- [ ] No duplicated information across files
- [ ] Concepts in doc/, operations in component READMEs
- [ ] Platform-specific execution documented once (AGENTS.md only)
- [ ] Commands shown in pure form (no devcontainer/wrapper prefixes)
- [ ] Standard tools not explained (devcontainer, docker, git assumed knowledge)

**Links and References**:
- [ ] All links work and point to correct sections
- [ ] Cross-references use consistent format
- [ ] Top of operational docs reference AGENTS.md for execution

**Content Quality**:
- [ ] Code examples tested in devcontainer
- [ ] Strategy docs under 600 lines
- [ ] Progressive loading pattern followed
- [ ] Comparison tables used instead of repetitive prose

**Context Efficiency**:
- [ ] No repeated command patterns (check with grep)
- [ ] Each concept documented exactly once
- [ ] Links used instead of duplicating content
- [ ] First 100 lines cover 80% of common decisions

---

## Examples

### Good: Building Documentation

**doc/building.md** (strategy):
```markdown
## Build Types

### What Are Build Types?
WAMR supports debug and release builds with different optimizations.

### When to Use Each
- **Debug**: Development, debugging, testing
- **Release**: Production, performance testing

### Quick Example
cmake -B build -DCMAKE_BUILD_TYPE=Debug

→ See [product-mini/platforms/linux/README.md] for all build options
```

**product-mini/platforms/linux/README.md** (operations):
```markdown
## Build Configuration

### Debug Build
cmake -B build -DCMAKE_BUILD_TYPE=Debug \
  -DWAMR_BUILD_INTERP=1 \
  -DWAMR_BUILD_DEBUG_INTERP=1 \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=1

### Release Build
cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DWAMR_BUILD_INTERP=1 \
  -DWAMR_BUILD_AOT=1 \
  -DWAMR_BUILD_FAST_INTERP=1

[50+ more lines with all CMake options documented]
```

### Bad: Duplicated Content

**Don't do this**:

Both `doc/testing.md` and `tests/unit/README.md` contain:
```markdown
## Running Unit Tests

To run all unit tests:
cd tests/unit
cmake -B build
cmake --build build
ctest --test-dir build --output-on-failure

To run specific tests:
ctest --test-dir build -R pattern

[Same 100 lines repeated in both files]
```

---

## Platform-Specific Execution Patterns

### Principle: Separate Command from Execution

**Commands should be documented in their pure form.** Platform-specific execution wrappers (like `devcontainer exec`) should be documented centrally, not repeated with every command.

### Single Source of Truth for Execution

**Bad (repetition everywhere)**:
```markdown
# tests/unit/README.md
devcontainer exec --workspace-folder . -- bash -c "cd tests/unit && cmake -B build"
devcontainer exec --workspace-folder . -- bash -c "cd tests/unit && cmake --build build"
devcontainer exec --workspace-folder . -- bash -c "cd tests/unit && ctest --test-dir build"
[50+ commands, all with devcontainer exec prefix]

# doc/testing.md  
devcontainer exec --workspace-folder . -- bash -c "ctest --test-dir build"
[More commands with same prefix]

# doc/building.md
devcontainer exec --workspace-folder . -- bash -c "cmake -B build"
[More commands with same prefix]
```

**Result**: 
- `devcontainer exec` documentation repeated 60+ times
- Hard to maintain (need to update 60+ places if wrapper changes)
- Context window waste (same information loaded repeatedly)
- Poor readability (hard to see the actual command)

**Good (separation of concerns)**:
```markdown
# AGENTS.md (single source of truth)
## Command Execution Pattern

All commands in WAMR documentation show raw command syntax.
On Linux, prefix with: devcontainer exec --workspace-folder . -- <command>

Examples:
| Documentation Shows | Execute on Linux |
|---------------------|------------------|
| cmake -B build | devcontainer exec --workspace-folder . -- cmake -B build |
| ctest --test-dir build | devcontainer exec --workspace-folder . -- ctest --test-dir build |

# tests/unit/README.md

cd tests/unit && cmake -B build
cd tests/unit && cmake --build build
cd tests/unit && ctest --test-dir build
[50+ commands in pure form]

# doc/testing.md

ctest --test-dir build
[Commands in pure form]

# doc/building.md

cmake -B build
[Commands in pure form]
```

**Benefits**:
- **Single source of truth**: Execution pattern documented once in AGENTS.md
- **Maintainability**: Change execution wrapper in 1 place, not 60+
- **Context efficiency**: ~200 lines saved across all docs (-8%)
- **Clarity**: Commands are readable (pure form, no wrapper noise)
- **Portability**: Commands work on macOS/Windows VS Code directly

### Implementation Guidelines

**When documenting commands**:

1. ✅ **DO**: Write commands in pure form
   ```bash
   cmake -B build
   ctest --test-dir build
   ```

2. ❌ **DON'T**: Include platform-specific wrappers
   ```bash
   # Wrong - platform-specific wrapper in docs
   devcontainer exec --workspace-folder . -- cmake -B build
   ./cmake -B build
   ```

3. ✅ **DO**: Add reference to AGENTS.md at document top
   ```markdown
   > **For AI Agents**: All commands show raw syntax. 
   ```

4. ✅ **DO**: Document execution pattern centrally in AGENTS.md
   - Platform requirements (Linux requires devcontainer)
   - Command pattern (how to wrap commands)
   - Examples table (doc command → actual execution)
   - Shell features (when to use `bash -c`)

### Context Window Impact

**Example: tests/unit/README.md optimization**

Before (with devcontainer exec everywhere):
```
devcontainer exec --workspace-folder . -- bash -c "..." 
Repeated ~50 times = ~2,500 characters = ~600 tokens
```

After (pure commands):
```
Pure commands without wrapper
Same commands = ~1,000 characters = ~250 tokens
Savings: ~350 tokens per document
```

With 5 major documentation files:
- **Total savings**: ~1,750 tokens (87.5% reduction)
- **Actual project savings**: 206 lines removed (-8.4% total documentation)

---

## For AI Agents: Quick Reference

When writing documentation:

1. **Identify the layer**: Am I writing strategy or operations?
2. **Check for duplication**: Does this info exist elsewhere?
3. **Link, don't copy**: Reference other docs rather than duplicating
4. **Stay within scope**: Strategy = concepts/decisions, Operations = commands/execution
5. **Test progressive loading**: Can someone understand the strategy without loading operations?

When reading documentation:
1. **Start at entry point**: CLAUDE.md or AGENTS.md
2. **Load strategy first**: Read doc/*.md to understand what/why/when
3. **Make decision**: Use comparison tables and decision trees
4. **Load operations only if needed**: Follow links to component READMEs for execution details

---

## Periodic Documentation Audit Checklist

**Purpose**: To be run by AI tools (Claude Code, custom scripts) on a regular schedule (monthly/quarterly) to detect accumulated documentation debt.

**Usage Pattern**:
```bash
# AI agent invocation
claude-code audit-docs --checklist=documentation_principles.md
```

### Audit Checklist

**Category 1: Zero Duplication (Critical)**

- [ ] **A1.1 Concept Duplication Scan**
  - Search for key concepts (AOT, JIT, WASI, devcontainer, etc.) across all .md files
  - Verify each concept is explained in exactly ONE file
  - Action: List all files where concept X appears, flag if > 1 has full explanation

- [ ] **A1.2 Command Duplication Scan**
  - Extract all command examples from all .md files
  - Identify identical commands (same tool + same flags)
  - Action: Flag any command that appears in > 2 files with full syntax

- [ ] **A1.3 Execution Pattern Duplication**
  - Search for `devcontainer exec`, container execution instructions
  - Should only appear in AGENTS.md § Command Execution Pattern
  - Action: Flag any other file with > 5 lines about container execution

- [ ] **A1.4 Navigation Duplication**
  - Search for "when to read", "read this if", workflow guidance
  - Should primarily be in AGENTS.md § Navigation
  - Action: Flag excessive navigation guidance in other files

**Category 2: Information Architecture (Important)**

- [ ] **A2.1 Layer Boundary Check**
  - CLAUDE.md: Verify < 40 lines, only tool-specific rules
  - AGENTS.md: Verify 100-200 lines, only navigation + critical constraints
  - doc/*.md: Verify 300-600 lines, concept + decision + minimal examples
  - Action: Flag files exceeding line limits

- [ ] **A2.2 Single Source of Truth Validation**
  - For each entry in SSOT Registry, verify:
    - Authoritative source contains the information
    - Other mentions link to authoritative source
    - No other file provides the same level of detail
  - Action: Report violations with file:line references

- [ ] **A2.3 Cross-Reference Integrity**
  - Extract all markdown links
  - Verify all links point to valid files and sections
  - Verify linked sections actually contain the referenced information
  - Action: Report broken or misleading links

- [ ] **A2.4 Orphaned Information**
  - Identify information not linked from AGENTS.md navigation
  - Check if information exists but is not discoverable
  - Action: Report orphaned docs that should be in navigation

**Category 3: Content Quality (Important)**

- [ ] **A3.1 Command Form Consistency**
  - All commands in doc/*.md should be in pure form (no platform wrappers)
  - Action: Flag files with `devcontainer exec` in examples

- [ ] **A3.2 Decision-Level Command Pattern**
  - doc/*.md should have max 1-3 command examples per concept
  - Full command references should be in component READMEs
  - Action: Flag doc/*.md sections with > 5 command variations

- [ ] **A3.3 Link Phrase Quality**
  - Links should describe what they point to
  - Avoid "click here", "see more", prefer "See [tests/unit/README.md] for all commands"
  - Action: Flag vague link phrases

**Category 4: Maintenance (Advisory)**

- [ ] **A4.1 Outdated Content Detection**
  - Compare doc mentions of versions, tools with current project state
  - Look for "TODO", "TBD", "FIXME" markers
  - Action: Report potentially stale content

- [ ] **A4.2 Documentation Coverage**
  - Check if major components (core/iwasm/*, product-mini/platforms/*) have READMEs
  - Verify AGENTS.md navigation references all major workflows
  - Action: Report missing documentation

- [ ] **A4.3 Consistency Check**
  - Terminology consistency (do we say "AOT compiler" or "wamrc" consistently?)
  - Heading format consistency (## vs ###)
  - Action: Report inconsistencies

### Audit Output Format

```markdown
# WAMR Documentation Audit Report
**Date**: YYYY-MM-DD
**Auditor**: [AI Tool Name]
**Files Scanned**: X markdown files

## Critical Issues (Zero Duplication Violations)

### A1.2 Command Duplication
- Command `ctest --test-dir build --output-on-failure` found in:
  - doc/testing.md:77
  - tests/unit/README.md:45
  - AGENTS.md:113
  **Recommendation**: Keep full syntax only in tests/unit/README.md

## Important Issues (Architecture Violations)

### A2.1 Layer Boundary Check
- CLAUDE.md: 45 lines (limit: 40) - exceeds by 5 lines
  **Recommendation**: Remove redundant content

## Advisory Items

### A4.1 Outdated Content
- doc/build_wamr.md mentions "LLVM 10.0" but .devcontainer uses LLVM 17
  **Recommendation**: Update version references

---
**Summary**: X critical, Y important, Z advisory issues found.
```

---

## Periodic Documentation Audit Process

**Purpose**: Ensure documentation quality remains high and duplication stays at zero over time.

**Frequency**:
- **Monthly**: Quick audit (Critical + Important items only) - ~15 minutes
- **Quarterly**: Full audit (all 14 items) - ~30-45 minutes
- **Major release**: Full audit before release
- **After large doc changes**: Targeted audit of affected areas

**Who Runs Audits**:
- AI agents (recommended): Use the checklist systematically
- Human reviewers: For architectural decisions and strategy validation
- Automation: CI checks for basic issues (broken links, line limits)

**Audit Workflow**:

1. **Create audit document**: `docs/audits/YYYY-MM-DD-<type>-audit.md`
   - `<type>` = monthly / quarterly / pre-release / post-change

2. **Run checklist systematically**:
   - Work through each item in order
   - Document findings: PASS ✅ or issues found
   - Record metrics (line counts, duplication patterns)

3. **Fix critical issues immediately**:
   - A1.1-A1.4 (duplication) must be zero
   - A2.1-A2.2 (line limits, SSOT) must pass

4. **Plan fixes for other issues**:
   - A3.x (content quality): Fix within 1-2 weeks
   - A4.x (user experience): Prioritize based on impact

5. **Commit audit report**:
   - Save findings to `docs/audits/`
   - Reference in commit message
   - Track trends over time

**Audit Report Template**:

````markdown
# [Type] Documentation Audit - YYYY-MM-DD

**Scope**: [What was audited]
**Duration**: [Time taken]
**Auditor**: [Human/AI]

## Critical Items (A1.x)

### A1.1 Concept Duplication
- Status: PASS ✅ / FAIL ❌
- Findings: [List any issues]

[Repeat for A1.2, A1.3, A1.4]

## Important Items (A2.x)

[Same structure]

## Advisory Items (A3.x, A4.x)

[Same structure]

## Summary

**Critical Issues**: X found, Y fixed
**Important Issues**: X found, Y fixed
**Total Line Count**: [Current vs. previous audit]
**Duplication Score**: [0 = perfect, >0 = needs work]

## Action Items

1. [Issue → Fix → Owner → Due date]
2. [...]

## Trends

- Line count: [Direction and magnitude]
- Duplication: [Better / same / worse]
- Link health: [Stats]
````

**Integration with Development**:

- **Pre-commit hook suggestion**: Check line limits automatically
- **CI integration**: Run A4.1 (broken links) on every PR
- **Documentation PRs**: Reviewer uses checklist
- **Release process**: Full audit required before tagging

**Success Metrics**:

- **Zero duplication**: A1.1-A1.4 always pass
- **Line limit compliance**: 95%+ of docs within limits
- **Link health**: 99%+ links working
- **Audit completion**: Monthly audits done on schedule

**Historical Audits**:

All audit reports are preserved in `docs/audits/`:
- `2026-05-08-baseline-audit.md` - Pre-optimization baseline
- `2026-05-10-phase2-audit.md` - After Phase 2
- `2026-05-13-phase3-audit.md` - After Phase 3
- [Future audits will be added here]

These historical records help track documentation quality trends over time.

---

## Enforcement

These principles are enforced through:

1. **PR reviews**: Check for duplication and proper layering
2. **AI agents**: Trained to follow these patterns via CLAUDE.md/AGENTS.md
3. **Documentation audits**: Periodic review of doc structure
4. **Automated checks**: Scripts to detect duplicate content (future)

---

**Documentation Version**: 2.0.0  
**Last Updated**: 2026-04-29  
**Maintained By**: WAMR Development Team

**Changelog**:
- **v2.0.0** (2026-04-29): Added platform-specific execution patterns, context window efficiency techniques
- **v1.0.0** (2026-04-04): Initial documentation principles
