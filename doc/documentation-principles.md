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
- General WAMR architecture (that's in doc/architecture-overview.md)

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
  scripts/in-container.sh "cd tests/unit && ctest --test-dir build"
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
scripts/in-container.sh "cd tests/unit && ctest --test-dir build"

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
Container usage: all commands must use scripts/in-container.sh wrapper
because the devcontainer has necessary tools like WASI-SDK.

See dev-in-container.md for setup instructions:
1. Install Docker
2. Install VS Code
3. Open in container
...
```

**Good**:
```markdown
Container usage: all commands must use scripts/in-container.sh wrapper.

See [dev-in-container.md](dev-in-container.md) for container setup.
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

- Keep strategy docs under 600 lines
- Use comparison tables instead of repetitive prose
- Link to examples instead of including all variations
- Front-load critical information (first 100 lines should cover 80% of decisions)

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

- [ ] No duplicated information across files
- [ ] Concepts in doc/, operations in component READMEs
- [ ] All links work and point to correct sections
- [ ] Code examples tested in devcontainer
- [ ] Strategy docs under 600 lines
- [ ] Progressive loading pattern followed
- [ ] Cross-references use consistent format

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
scripts/in-container.sh "cmake -B build -DCMAKE_BUILD_TYPE=Debug"

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

## Enforcement

These principles are enforced through:

1. **PR reviews**: Check for duplication and proper layering
2. **AI agents**: Trained to follow these patterns via CLAUDE.md/AGENTS.md
3. **Documentation audits**: Periodic review of doc structure
4. **Automated checks**: Scripts to detect duplicate content (future)

---

**Documentation Version**: 1.0.0  
**Last Updated**: 2026-04-04  
**Maintained By**: WAMR Development Team
