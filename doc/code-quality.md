# Code Quality in WAMR

This guide explains WAMR's code quality standards, what they mean, why they matter, and when to apply them. For operational pre-commit checklists and detailed commands, see [linting.md](linting.md).

**Philosophy**: High code quality prevents bugs, improves maintainability, and enables confident refactoring. WAMR enforces quality through automated tools, clear standards, and continuous integration.

---

## Prerequisites

All code quality tools are pre-installed in the devcontainer:

1. **Read [AGENTS.md](../AGENTS.md)** - Platform-specific execution requirements
2. **Read [dev-in-container.md](dev-in-container.md)** - Container technical details

> **Note**: All commands in this guide show raw syntax. See [AGENTS.md](../AGENTS.md) for platform-specific execution.

**Before claiming work complete:**
1. Format check passes (`clang-format-14 --dry-run --Werror`)
2. Build succeeds with no warnings
3. Relevant tests pass

**→ See [linting.md](linting.md) for complete pre-commit workflow**

---

## Code Quality Standards Overview

WAMR maintains multiple quality gates to ensure correctness, consistency, and maintainability:

| Standard | Purpose | When Enforced | Tool | Documentation |
|----------|---------|---------------|------|---------------|
| **Code Formatting** | Consistent visual style | Every commit | clang-format-14 | [linting.md](linting.md) |
| **Compiler Warnings** | Catch potential bugs | Every build | gcc/clang -Wall -Werror | [linting.md](linting.md) |
| **Static Analysis** | Find logic errors | Critical changes | scan-build, clang-tidy | This doc + [linting.md](linting.md) |
| **Python Linting** | Script quality | When changing Python | pylint | [linting.md](linting.md) |
| **Shell Linting** | Script safety | When changing shell | shellcheck | [linting.md](linting.md) |
| **Pre-commit Checks** | Quality gate | Before every commit | Multiple tools | [linting.md](linting.md) |
| **CI Enforcement** | Automated validation | Every PR | GitHub Actions | [linting.md](linting.md) |

---

## Code Formatting Standards

### What is Code Formatting?

WAMR uses **clang-format-14** to enforce consistent visual style based on K&R coding conventions with Mozilla customizations. Configuration lives in `.clang-format` at project root.

### Why Enforce Formatting?

**Benefits**:
- **Readability**: Consistent style makes code easier to scan
- **Cleaner diffs**: Only functional changes appear, not whitespace
- **Eliminates debates**: Tool decides, not personal preference
- **Zero effort**: Automated formatting

**Without enforcement**: Diffs mix style and logic, reviews waste time on formatting, merge conflicts from whitespace.

### Key Style Rules

```c
// Indentation: 4 spaces, brace placement: functions on new line
void function()
{
    if (condition) {  // Control: same line
        do_something();
    }
}

// 80 chars max, pointer right-aligned
char *ptr;
```

### When Enforced

**Required**: Before every commit, PR, merge to main
**CI fails if**: Files don't match clang-format-14 rules

### Quick Example

```bash
# Check formatting
clang-format-14 --dry-run --Werror file.c

# Auto-fix
clang-format-14 -i file.c
```

**→ See [linting.md](linting.md) for complete formatting workflow**

---

## Compiler Warnings

### What Are Compiler Warnings?

Compiler warnings alert you to potential bugs and questionable code patterns. WAMR builds with strict flags: `-Wall -Wextra -Werror`

### Why Treat Warnings as Errors?

**Catches bugs like**:
- Uninitialized variables
- Unused variables (dead code)
- Missing headers (implicit declarations)
- Type mismatches

**Result**: Warning-free builds = safer, more maintainable code

### When Enforced

**Always**: Local builds, CI/CD, pre-commit checks
**Build fails if**: Any warning generated

### Quick Example

```bash
cmake -B build -DCMAKE_C_FLAGS='-Wall -Werror' && cmake --build build
```

**→ See [linting.md](linting.md) for fixing common warnings**

---

## Static Analysis

### What is Static Analysis?

Static analysis examines code without executing it, finding bugs through deep inspection of code paths, data flow, and logic. Catches bugs that compilers and tests miss.

**Tools**: scan-build (memory/logic), clang-tidy (C++ practices), cppcheck (portability)

### Why Use Static Analysis?

**Finds bugs like**:
- Memory leaks, use-after-free, null dereferences
- Resource leaks (files, sockets)
- Dead code, logic errors
- Race conditions

### When to Run

**Required**: Memory management changes, security fixes, large refactors
**Recommended**: New features, complex code paths
**Skip**: Documentation, simple config changes

### Available Tools

| Tool | Checks | When to Use |
|------|--------|-------------|
| scan-build | Memory safety, leaks | Memory changes, critical fixes |
| clang-tidy | C++ practices, readability | C++ code, performance |
| cppcheck | Portability, bug patterns | Cross-platform, embedded |

### Quick Example

```bash
# Analyze build
scan-build cmake --build build
```

**→ See [linting.md](linting.md) for detailed analysis workflows**

---

## Python Code Quality

### Standards

WAMR Python scripts follow PEP 8 using **pylint** for style compliance and bug detection.

**Checks**: PEP 8 style, unused imports, missing docstrings, potential bugs

### Why Lint Python?

Prevents bugs (undefined variables), ensures consistency (PEP 8), maintains readable code.

### When Required

Before committing changes to `ci/`, `tests/`, or new Python utilities.

```bash
pylint ci/coding_guidelines_check.py
```

**→ See [linting.md](linting.md) for complete workflow**

---

## Shell Script Quality

### Standards

Shell scripts use **shellcheck** for static analysis.

**Checks**: Unquoted variables, missing error handling, unreachable code, portability issues

### Why Lint Shell Scripts?

Prevents critical bugs: word splitting from unquoted variables, deleting wrong directories from unchecked `cd` failures, missing error handling.

### Best Practices

1. Always quote: `"$var"` not `$var`
2. Use strict mode: `set -euo pipefail`
3. Check exit codes: `command || exit 1`

### When Required

Before committing: build scripts, test scripts, `devcontainer exec`, new utilities.

```bash
shellcheck devcontainer exec
```

**→ See [linting.md](linting.md) for complete workflow**

---

## Code Review Standards

### Review Philosophy

**Goals**: Catch bugs early, share knowledge, maintain quality, mentor contributors
**Not goals**: Nitpick style (tools handle that), block with perfectionism, assert preferences

### What Reviewers Check

**Automatic** (CI-enforced): Formatting, warnings, tests pass
**Manual focus**:
- Correctness: Logic handles edge cases, errors checked, resources cleaned
- Testing: New tests added, error paths covered, regression tests for bugs
- Code quality: No duplication, appropriate abstractions
- Documentation: Public APIs documented, complex logic explained

### Review Guidelines

**Authors**: Self-review first, ensure CI passes, add tests, update docs
**Reviewers**: Understand purpose, check correctness over style, verify tests, suggest improvements without demanding perfection

### Common Issues to Catch

**Memory management**: Missing `free()` in error paths
**Error handling**: Unchecked return values from risky operations
**Resource cleanup**: File handles, sockets not closed in all paths
**Edge cases**: Boundary conditions, null inputs, empty data

**Examples**:
```c
// ❌ Leak in error path
char *buf = malloc(1024);
if (error) return -1;  // Forgot free()

// ✅ Cleanup all paths
char *buf = malloc(1024);
if (error) { free(buf); return -1; }
```

---

## Code Quality Decision Guide

Use this guide to determine which quality checks to run:

### Before Committing (Always)

```
All changes:
  ├─ Format check (clang-format-14)
  ├─ Build successfully
  └─ No compiler warnings

Python changes:
  └─ pylint passes

Shell changes:
  └─ shellcheck passes
```

**→ [linting.md](linting.md) has complete pre-commit checklist**

### For Different Change Types

**Small bug fix (< 50 lines)**:
- Formatting check ✓
- Build check ✓
- Unit tests (if available) ✓
- Regression test added ✓

**Feature addition**:
- Formatting check ✓
- Build check ✓
- Unit tests ✓
- Integration tests ✓
- Documentation updated ✓

**Refactoring (no behavior change)**:
- Formatting check ✓
- Build check ✓
- All tests pass ✓
- Static analysis recommended

**Memory management changes**:
- Formatting check ✓
- Build check ✓
- Unit tests ✓
- **Static analysis required** ✓
- Valgrind recommended

**Security fix**:
- Formatting check ✓
- Build check ✓
- All tests ✓
- **Static analysis required** ✓
- Regression test required ✓
- Security review required ✓

### When to Use Each Tool

| Situation | Tool | Why |
|-----------|------|-----|
| Every commit | clang-format | Enforced by CI |
| Every build | -Wall -Werror | Catch bugs early |
| Python changes | pylint | Script quality |
| Shell changes | shellcheck | Script safety |
| Memory changes | scan-build | Find leaks |
| Large refactor | scan-build + tests | Safety net |
| Security fixes | All tools | Maximum assurance |
| Documentation | None | No code changes |

---

## Pre-Commit Quality Workflow

Before every commit:

1. **Format check** - `clang-format-14 --dry-run --Werror` (auto-fix with `-i`)
2. **Build check** - `cmake --build build` with `-Wall -Werror`
3. **Test check** - Run relevant tests (unit, spec, regression)

**→ See [linting.md](linting.md) for complete step-by-step workflow**

---

## Best Practices

### Development Workflow

**During**: Format regularly, build with warnings, run tests frequently, fix immediately
**Before commit**: Pre-commit checklist, self-review, tests pass, docs updated
**Before push**: Rebase on main, verify CI locally, full test suite for critical changes

### Quality Mindset

**Do**: Trust tools, fix root causes, add tests for bugs, small focused commits
**Don't**: Disable warnings, commit failing tests, mix formatting and logic, skip checks, ignore analyzer

### Continuous Improvement

Learn from mistakes: Why didn't tests catch it? Why didn't local checks catch it? Add regression tests. Document gotchas.

---

## CI Quality Enforcement

### What CI Checks

Every PR runs: formatting, build (multiple configs), warnings check, unit tests, spec tests, regression tests, platform builds (Linux/macOS/Windows/embedded).

### Why It Matters

**Benefits**: Catches issues pre-merge, maintains quality bar, prevents regressions, enables safe refactoring
**Failures indicate**: Didn't meet standards, local checks skipped, needs revision

### Reproducing Failures

1. Check CI logs for failing command
2. Run same command via `devcontainer exec`
3. Fix issue, verify, push

**Devcontainer ensures local = CI environment.**

**→ See [linting.md](linting.md) for reproducing specific failures**

---

## Troubleshooting

**Format check fails**: Auto-fix with `clang-format-14 -i file.c`
**Build warnings**: Read message, fix root cause, rebuild
**Static analysis warnings**: Determine true/false positive, fix or document

**→ See [linting.md](linting.md) for detailed troubleshooting**

---

## Quick Reference

### Essential Commands

```bash
# Format check (dry run)
clang-format-14 --dry-run --Werror <file>

# Auto-fix formatting
clang-format-14 -i <file>

# Build with strict warnings
cmake -B build -DCMAKE_C_FLAGS='-Wall -Werror' && cmake --build build

# Check shell script
shellcheck <script.sh>

# Lint Python
pylint <script.py>

# Static analysis
scan-build cmake --build build
```

### Quality Standards Summary

| Check | Tool | Enforced | Purpose |
|-------|------|----------|---------|
| Format | clang-format-14 | Every commit | Visual consistency |
| Warnings | -Wall -Werror | Every build | Catch bugs |
| Python | pylint | Python changes | Script quality |
| Shell | shellcheck | Shell changes | Script safety |
| Analysis | scan-build | Critical changes | Deep bug finding |

---

## Related Documentation

- **[linting.md](linting.md)** - Complete pre-commit checklist and operational guide
- **[dev-in-container.md](dev-in-container.md)** - Container setup and usage
- **[building.md](building.md)** - Build configuration and options
- **[testing.md](testing.md)** - Testing strategy and test types
- **[debugging.md](debugging.md)** - Debugging with GDB and Valgrind
- **[CONTRIBUTING.md](../CONTRIBUTING.md)** - Contribution guidelines
- **[AGENTS.md](../AGENTS.md)** - AI agent development guide

---

## External Resources

- [clang-format Documentation](https://clang.llvm.org/docs/ClangFormat.html)
- [Clang Static Analyzer](https://clang-analyzer.llvm.org/)
- [ShellCheck Wiki](https://github.com/koalaman/shellcheck/wiki)
- [Pylint Documentation](https://pylint.pycqa.org/)
- [C Coding Standards (Linux Kernel)](https://www.kernel.org/doc/html/latest/process/coding-style.html)

---

**Documentation Version**: 2.0.0  
**Last Updated**: 2026-04-04  
**Maintained By**: WAMR Development Team
