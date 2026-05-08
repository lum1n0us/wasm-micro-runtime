# Code Quality Standards

This guide defines WAMR's code quality standards and principles. For the execution checklist, see [linting.md](./linting.md).

**Boundary with linting.md**:
- **code-quality.md** (this doc): Standards and principles (WHY)
- **linting.md**: Execution checklist (WHAT to run)

**Prerequisites**:
1. [AGENTS.md](../AGENTS.md) - Execution patterns

> **Execution**: Commands in pure form. See [AGENTS.md § Command Execution Pattern](../AGENTS.md#command-execution-pattern).

---

## Philosophy

High code quality prevents bugs, improves maintainability, and enables confident refactoring. WAMR enforces quality through automated tools, clear standards, and continuous integration.

**Quality Gates**: Formatting, compiler warnings, static analysis, Python/shell linting, pre-commit checks, CI enforcement.

**→ See [linting.md § Pre-Commit Checklist](./linting.md) for complete workflow**

---

## Code Formatting

### What and Why

WAMR uses **clang-format-14** to enforce consistent visual style based on K&R coding conventions with Mozilla customizations.

**Why enforce formatting**:
- **Readability**: Consistent style makes code easier to scan
- **Cleaner diffs**: Only functional changes appear, not whitespace
- **Eliminates debates**: Tool decides, not personal preference
- **Zero effort**: Automated formatting

**Without enforcement**: Diffs mix style and logic, reviews waste time, merge conflicts from whitespace.

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

### When Required

**Required**: Before every commit, PR, merge to main
**CI fails if**: Files don't match clang-format-14 rules

**→ See [linting.md § Code Formatting](./linting.md) for execution checklist**

---

## Compiler Warnings

### What and Why

Compiler warnings alert you to potential bugs and questionable code patterns. WAMR builds with strict flags: `-Wall -Wextra -Werror`

**Why treat warnings as errors**:
- Catches uninitialized variables
- Detects unused variables (dead code)
- Finds missing headers (implicit declarations)
- Prevents type mismatches

**Result**: Warning-free builds = safer, more maintainable code

### When Required

**Always**: Local builds, CI/CD, pre-commit checks
**Build fails if**: Any warning generated

**→ See [linting.md § Compiler Warnings](./linting.md) for fixing common warnings**

---

## Static Analysis

### What and Why

Static analysis examines code without executing it, finding bugs through deep inspection of code paths, data flow, and logic. Catches bugs that compilers and tests miss.

**Available tools**:
- **scan-build**: Memory safety, leaks
- **clang-tidy**: C++ practices, readability
- **cppcheck**: Portability, bug patterns

**Why use static analysis**:
- Finds memory leaks, use-after-free, null dereferences
- Detects resource leaks (files, sockets)
- Catches dead code, logic errors
- Identifies race conditions

### When to Run

**Required**: Memory management changes, security fixes, large refactors
**Recommended**: New features, complex code paths
**Skip**: Documentation, simple config changes

**→ See [linting.md § Static Analysis](./linting.md) for detailed workflows**

---

## Python Code Quality

### Standards

WAMR Python scripts follow **PEP 8** using **pylint** for style compliance and bug detection.

**Why lint Python**: Prevents bugs (undefined variables), ensures consistency, maintains readable code.

**Checks**: PEP 8 style, unused imports, missing docstrings, potential bugs

### When Required

Before committing changes to `ci/`, `tests/`, or new Python utilities.

**→ See [linting.md § Python Linting](./linting.md) for complete workflow**

---

## Shell Script Quality

### Standards

Shell scripts use **shellcheck** for static analysis.

**Why lint shell scripts**: Prevents critical bugs (word splitting from unquoted variables, deleting wrong directories from unchecked `cd` failures, missing error handling).

**Checks**: Unquoted variables, missing error handling, unreachable code, portability issues

### Best Practices

1. Always quote: `"$var"` not `$var`
2. Use strict mode: `set -euo pipefail`
3. Check exit codes: `command || exit 1`

### When Required

Before committing: build scripts, test scripts, new utilities.

**→ See [linting.md § Shell Script Linting](./linting.md) for complete workflow**

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

## Decision Guide

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

### Change Type Requirements

**Small bug fix**: Format, build, unit tests, regression test
**Feature addition**: Format, build, tests, documentation
**Refactoring**: Format, build, all tests, static analysis (recommended)
**Memory changes**: Format, build, tests, **static analysis required**, Valgrind
**Security fix**: Format, build, tests, **static analysis required**, regression test, security review

**→ See [linting.md § Pre-Commit Checklist](./linting.md) for complete workflow**

---

## Best Practices

### Development Workflow

**During development**: Format regularly, build with warnings, run tests frequently, fix immediately
**Before commit**: Pre-commit checklist, self-review, tests pass, docs updated
**Before push**: Rebase on main, verify CI locally, full test suite for critical changes

### Quality Mindset

**Do**: Trust tools, fix root causes, add tests for bugs, small focused commits
**Don't**: Disable warnings, commit failing tests, mix formatting and logic, skip checks

### Continuous Improvement

Learn from failures: Why didn't tests catch it? Why didn't local checks catch it? Add regression tests. Document gotchas.

---

## CI Quality Enforcement

### What CI Checks

Every PR runs: formatting, build (multiple configs), warnings check, unit tests, spec tests, regression tests, platform builds (Linux/macOS/Windows/embedded).

### Why It Matters

**Benefits**: Catches issues pre-merge, maintains quality bar, prevents regressions, enables safe refactoring
**Failures indicate**: Didn't meet standards, local checks skipped, needs revision

**→ See [linting.md § Reproducing CI Failures](./linting.md) for detailed troubleshooting**

---

## Troubleshooting

**Format check fails**: Auto-fix with `clang-format-14 -i file.c`
**Build warnings**: Read message, fix root cause, rebuild
**Static analysis warnings**: Determine true/false positive, fix or document

**→ See [linting.md § Troubleshooting](./linting.md) for detailed solutions**

---

## Related Documentation

- **[linting.md](linting.md)** - Complete pre-commit checklist and operational guide
- **[building.md](building.md)** - Build configuration and options
- **[testing.md](testing.md)** - Testing strategy and test types
- **[debugging.md](debugging.md)** - Debugging with GDB and Valgrind
- **[AGENTS.md](../AGENTS.md)** - Execution patterns
