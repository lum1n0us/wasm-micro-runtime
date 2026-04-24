# AI-Friendly Documentation System Implementation Spec

> **For agentic workers:** This spec defines the design for making WAMR accessible to AI agents for development assistance.

**Goal:** Enable AI agents to effectively assist with WAMR development tasks (bug fixes, PR reviews, testing, refactoring) by providing structured documentation that addresses information gaps, scattered knowledge, and implicit conventions.

**Architecture:** Phased rollout starting with navigation guide (AGENTS.md) and architecture overview, iterating based on real AI agent usage, then adding workflow and testing documentation.

**Approach:** Leverage existing documentation where possible, create supplementary docs only for gaps, plan to merge improvements back to existing docs over time.

---

## Problem Statement

AI agents need to help developers improve efficiency in the WAMR development process, with the long-term goal of agents taking on development work independently. Current challenges:

1. **Information is scattered** - Testing info across tests/unit/, tests/benchmarks/, doc/build_wamr.md
2. **Missing context** - Docs assume knowledge humans have but AI agents don't
3. **Implicit conventions** - Obvious to WAMR developers but never written down

## Target Use Cases

Priority development activities for AI agents (ordered by importance):

1. **Bug fixes and debugging** - Understanding code paths, identifying root causes, implementing fixes
2. **PR reviews** - Code quality checks, convention compliance, architectural consistency
3. **Test writing** - Unit tests, integration tests, following project patterns
4. **Refactoring** - Code cleanup, optimization, maintaining architectural integrity

## Design Principles

1. **AGENTS.md as entry point** - Single navigation document that guides agents to appropriate resources
2. **Leverage existing docs** - Reference doc/, tests/, samples/ extensively to minimize duplication
3. **Supplementary docs fill gaps** - Create new docs under doc/ only for missing information
4. **Platform agnostic** - Works with Claude, Gemini, GitHub Copilot, Cursor, etc.
5. **Phased learning approach** - Build incrementally, learn from real usage, adjust based on feedback

## File Structure

```
/
├── AGENTS.md                          # Entry point - navigation guide for AI agents
└── doc/
    ├── architecture-overview.md       # Phase 1: Component relationships & abstractions
    ├── dev-workflows.md               # Phase 3: Build, test, debug procedures
    └── testing-guide.md               # Phase 4: Comprehensive testing strategy
```

## AGENTS.md Content Specification

**Purpose:** Navigation guide that tells AI agents when to read which document and how to approach development tasks.

**Structure:**

```markdown
# AI Agent Guide for WAMR Development

## What is WAMR?
[2-3 sentence summary with link to README.md]

## What AI Agents Can Help With
- Bug fixes and debugging
- PR reviews and code quality
- Writing and maintaining tests
- Code refactoring and optimization

## Getting Started: Read This First
1. Read this entire AGENTS.md
2. Read doc/architecture-overview.md for mental model
3. Read relevant existing docs based on your task

## Navigation: When to Read What

### For Bug Fixes
- Start: doc/architecture-overview.md (understand components)
- Build: doc/build_wamr.md
- Debug: doc/source_debugging.md
- Test: doc/testing-guide.md (available Phase 4)

### For Adding Features
- Architecture: doc/architecture-overview.md
- Build config: doc/build_wamr.md
- API design: doc/embed_wamr.md, doc/export_native_api.md
- Testing: doc/testing-guide.md (available Phase 4)

### For PR Reviews
- Architecture: doc/architecture-overview.md
- Conventions: doc/dev-workflows.md (available Phase 3)
- Testing expectations: doc/testing-guide.md (available Phase 4)

### For Test Writing
- Test strategy: doc/testing-guide.md (available Phase 4)
- Unit tests: tests/unit/README.md
- Wasm spec tests: tests/wamr-test-suites/
- Integration examples: samples/README.md

### For Refactoring
- Architecture principles: doc/architecture-overview.md
- Conventions: doc/dev-workflows.md (available Phase 3)
- Performance: doc/perf_tune.md, doc/memory_tune.md

## Quick Reference
- Project overview: README.md
- Build instructions: doc/build_wamr.md
- Embedding API: doc/embed_wamr.md
- Architecture: doc/architecture-overview.md
- All docs: doc/ directory

**Testing & Examples:**
- Unit tests: tests/unit/
- Wasm spec tests: tests/wamr-test-suites/
- Benchmarks: tests/benchmarks/
- Samples: samples/
```

**Key characteristics:**
- Task-oriented navigation structure
- Explicit phase indicators for docs not yet available
- Links to existing documentation wherever possible
- Minimal inline content

## doc/architecture-overview.md Content Specification

**Purpose:** Provide AI agents with mental model of WAMR's structure, components, and design decisions.

**Structure:**

```markdown
# WAMR Architecture Overview

## Component Hierarchy & Relationships

### Core Components
1. **VMcore** (core/iwasm/, core/shared) - Runtime libraries and platform abstraction
   - core/iwasm/aot/ - AOT runtime implementation
   - core/iwasm/interpreter/ - Interpreter implementation, Wasm loader, and runtime
   - core/iwasm/compilation/ - LLVM compilation engine (shared with JIT and AOT compiler)
   - core/iwasm/fast-jit/ - Fast JIT implementation
   - core/iwasm/common/ - Shared code between AOT and interpreter
   - core/iwasm/include/ - Public API headers (wasm_export.h, wasm_c_api.h)
   - core/iwasm/libraries/ - Native libraries (WASI, libc-builtin, socket, threading)
   - core/shared/mem-alloc/ - Memory management
   - core/shared/platform/ - Platform abstraction layer
   - core/shared/utils/ - Utilities built on platform abstraction

2. **iwasm** (product-mini/platforms) - Executable binary with WASI support

3. **wamrc** (wamr-compiler/) - AOT compiler

### Component Relationships
```
wamrc (AOT compiler)
  └─> Uses LLVM to compile Wasm → native code
  └─> Generates .aot files

VMcore (runtime libraries)
  ├─> Interpreter: Executes .wasm files directly
  ├─> AOT Runtime: Loads and executes .aot files
  └─> JIT: Compiles .wasm to native code at runtime

iwasm (executable)
  └─> Links against VMcore
  └─> Provides CLI
```

**Dependency Rules:**
- Always use platform abstraction layer for OS interactions in VMcore
- Avoid direct dependencies between iwasm and wamrc
- VMcore should be embeddable in any host application

**Design Principles:**
- Maintain minimal and efficient core runtime
- Marco-based design for modularity and configurability
- Clear separation of concerns between loading, instantiation, execution
- Handle errors gracefully with informative error messages
- Release resources properly to avoid memory leaks

## Key Abstractions & Data Structures

### Module Loading & Execution
- wasm_module_t: Loaded module (parsed, validated)
- wasm_module_inst_t: Instantiated module (with memory allocated)
- wasm_exec_env_t: Execution context
- wasm_function_inst_t: Function instances

### Memory Model
- Linear memory allocation (mem-alloc subsystem)
- Stack vs heap boundaries
- WASI vs builtin libc memory implications
- Aligned allocation support (gc_alloc_vo_aligned)

### Execution Modes
- Interpreter: Direct bytecode execution
- AOT: Pre-compiled native code
- JIT: Runtime compilation
- Tier-up: Fast JIT → LLVM JIT transitions

## Critical Code Paths

### Module Load & Execute Flow
1. wasm_runtime_load() - Parse and validate Wasm module
2. wasm_runtime_instantiate() - Allocate memory, initialize
3. wasm_runtime_lookup_function() - Find function to call
4. wasm_runtime_call_wasm() - Execute Wasm function

### Memory Allocation Path
- Entry points: gc_alloc_vo, gc_alloc_vo_aligned
- Metadata tracking for aligned allocations
- Heap management strategies

## Platform Abstraction Layer
- Location: core/shared/platform/
- Purpose: OS abstraction for portability
- Porting guide: doc/port_wamr.md

## Build System Architecture
- CMake primary build system
- Entry point: build-scripts/runtime_lib.cmake
- Feature flags: WAMR_BUILD_* variables
- Reference: doc/build_wamr.md

## Design Patterns & Conventions

**Error Handling:**
- APIs return bool (true=success) or pointer (NULL=failure)
- Error messages written to caller-provided error_buf
- Always check return values

**Memory Management:**
- Explicit allocation/deallocation
- Pair wasm_runtime_load() with wasm_runtime_unload()
- Pair wasm_runtime_instantiate() with wasm_runtime_deinstantiate()

**Thread Safety:**
- wasm_module_t is read-only, shareable across threads
- wasm_module_inst_t must not be shared between threads
- wasm_exec_env_t is thread-local

**Naming Conventions:**
- Public API: wasm_runtime_* prefix
- OS abstraction: os_* prefix
- Memory allocator: mem_allocator_*, gc_* prefix

**Build-time Configuration:**
- Use WAMR_BUILD_* variables for features
- Conditional compilation with #if WASM_ENABLE_* macros
```

**Key characteristics:**
- Focuses on understanding "how things fit together"
- Explains key abstractions that appear throughout codebase
- Links to existing docs for detailed information
- Updated incrementally as agents encounter gaps

## doc/dev-workflows.md Content Specification (Phase 3)

**Purpose:** Make implicit development conventions explicit for AI agents.

**Structure outline:**

```markdown
# WAMR Development Workflows

## Build System Usage
- Common build configurations
- Feature flag combinations
- Platform-specific builds

## Test Execution Patterns
- Running unit tests
- Running benchmarks
- Platform-specific test procedures

## Debugging Workflows
- Common debugging scenarios
- Tool usage (gdb, lldb, debuggers)
- Log interpretation

## Common Development Tasks
- Adding a new API
- Adding platform support
- Implementing a Wasm proposal
- Fixing a bug (typical workflow)

## File Organization Conventions
- Where different types of code live
- Naming conventions
- Header file organization
```

**Note:** Detailed content determined during Phase 2 based on what AI agents actually struggle with.

## doc/testing-guide.md Content Specification (Phase 4)

**Purpose:** Unify testing strategy and expectations across the codebase.

**Structure outline:**

```markdown
# WAMR Testing Guide

## Testing Philosophy
- What needs tests
- Test coverage expectations

## Test Organization
- tests/unit/ - Unit tests
- tests/benchmarks/ - Performance tests
- tests/regression/ - Regression tests
- samples/ - Example-based tests

## Writing Tests
- Unit test patterns
- Integration test patterns
- Platform-specific testing
- Mock and fixture usage

## Test Execution
- Running specific test suites
- CI/CD test execution
- Platform-specific test commands

## Test-Driven Development
- Write failing test first
- Minimal implementation
- Refactor
- Commit frequently
```

**Note:** Content informed by Phase 2-3 learnings about test writing patterns.

## Implementation Phases

### Phase 1: Foundation (Week 1-2)

**Deliverables:**
- AGENTS.md with complete navigation structure
- doc/architecture-overview.md with component relationships, key abstractions, critical code paths
- Commit both files to repository

**Success criteria:**
- AI agent can read AGENTS.md and find relevant existing documentation
- AI agent understands WAMR component structure from architecture doc
- Both files pass spec-document-reviewer review

**Testing approach:**
- Use AI agent to read codebase and explain architecture back
- Try simple bug fix task with only Phase 1 docs available
- Document any confusion or missing information

### Phase 2: Learning Period (Week 3-4)

**Deliverables:**
- Document of learnings: what agents struggled with, what worked well
- List of gaps to address in dev-workflows.md and testing-guide.md

**Activities:**
- Assign real development tasks to AI agents:
  - Fix a bug from issue tracker
  - Review a PR
  - Write tests for untested code
- Observe where agents get stuck or make mistakes
- Note questions agents ask repeatedly
- Identify missing conventions and implicit knowledge

**Success criteria:**
- At least 3 different task types attempted
- Clear list of gaps documented
- Priorities established for Phase 3 and 4 content

### Phase 3: Development Workflows (Week 5)

**Deliverables:**
- doc/dev-workflows.md with build, test, debug workflows
- Updated AGENTS.md navigation
- Commit changes

**Content priorities (based on Phase 2):**
- Most frequently asked questions addressed first
- Common mistakes codified as conventions
- Workflow recipes for typical tasks

**Success criteria:**
- AI agent can execute common development tasks without clarification
- Workflow doc passes spec-document-reviewer review
- Measurable improvement in agent task completion

### Phase 4: Testing Strategy (Week 6)

**Deliverables:**
- doc/testing-guide.md with comprehensive testing guidance
- Updated AGENTS.md navigation
- Commit changes

**Content priorities:**
- Test writing patterns used in codebase
- Coverage expectations for different code types
- TDD workflow integration

**Success criteria:**
- AI agent writes tests matching project patterns
- Tests follow established conventions
- Testing guide passes spec-document-reviewer review

### Phase 5: Feedback Loop (Week 7+)

**Deliverables:**
- Updated existing documentation (README.md, doc/build_wamr.md, etc.)
- Improved clarity based on AI agent learnings
- Benefits both AI agents and human developers

**Activities:**
- Review AI agent questions/mistakes across all phases
- Identify improvements to merge into existing docs
- Update existing docs with clarifications
- Consider CI/CD checks for doc updates

**Success criteria:**
- Reduced duplication between supplementary and existing docs
- Existing docs improved based on concrete usage feedback
- Both humans and AI agents benefit from clearer documentation

## Maintenance Strategy

### When to Update Docs

**AGENTS.md updates:**
- When new supplementary docs added
- When existing doc locations change
- Rarely needs updates to core navigation structure

**architecture-overview.md updates:**
- When new core components added
- When major architectural refactors occur
- When new abstractions introduced

**dev-workflows.md updates:**
- When build system changes
- When new tools or workflows adopted
- When common procedures evolve

**testing-guide.md updates:**
- When testing patterns change
- When new test frameworks adopted
- When coverage expectations change

### CI/CD Integration

**Phase 1-4:** Manual PR review checklist
- "Did you update AGENTS.md/architecture docs if you changed core components?"

**Phase 5+:** Consider automated checks
- Parse git diffs for core component changes
- Suggest doc review if needed
- Optional: CI check that fails with reminder

### Success Metrics

**Quantitative:**
- Time from task assignment to working code
- Percentage of tasks completed without human intervention
- Reduction in clarifying questions from agents

**Qualitative:**
- AI agents fix bugs without human clarification
- AI agents write tests matching project patterns
- PR reviews by AI agents catch real issues
- Human developers find supplementary docs useful

## Risk Mitigation

**Risk: Duplication with existing docs**
- Mitigation: Aggressive referencing to existing docs in AGENTS.md
- Mitigation: Phase 5 feedback loop to merge improvements back

**Risk: Docs become stale**
- Mitigation: CI/CD reminders for doc updates
- Mitigation: Regular review of agent questions to spot gaps

**Risk: Over-engineering before learning what's needed**
- Mitigation: Phased approach, learn from Phase 2 before building Phase 3-4
- Mitigation: Can stop or pivot after Phase 1 if not valuable

**Risk: AI agents still struggle despite docs**
- Mitigation: Phase 2 learning period identifies real issues
- Mitigation: Iterative improvement based on actual usage
- Mitigation: Can adjust approach if agents need different information format

## Open Questions for Implementation

1. Should architecture-overview.md include UML diagrams or stick to text descriptions?
2. How much implementation detail in architecture doc vs. "read the code"?
3. Should we create CLAUDE.md symlink to AGENTS.md for Claude-specific workflows?
4. What's the review process for updating these docs? (Same as code PRs?)

## References

**Existing WAMR documentation:**
- README.md - Project overview
- doc/build_wamr.md - Build system configuration
- doc/embed_wamr.md - Embedding API guide
- doc/architecture.md - (if exists, leverage or replace)
- tests/unit/README.md - Unit testing information
- samples/README.md - Example code

**AI Agent platforms:**
- Claude Code - Anthropic's development assistant
- GitHub Copilot - Code completion and chat
- Cursor - AI-powered IDE
- Gemini CLI - Google's development assistant
