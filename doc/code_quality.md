# Code Quality Standards

This guide defines WAMR's code quality standards. For the execution checklist, see [linting.md](./linting.md).

**Boundary with linting.md**:

- **code_quality.md** (this doc): Standards (WHAT)
- **linting.md**: Execution checklist (HOW)


---

## Code Formatting

- **Tool**: `clang-format-12 --style file`
- **Config**: `.clang-format` at repository root
- **Style**: K&R with Mozilla customizations
- **When required**: Before every commit, PR, merge to main
- **CI enforcement**: Build fails if files don't match format

**→ See [linting.md § Code Formatting](./linting.md) for execution**

---

## Compiler Warnings

**Flags**: `-Wall -Wextra -Werror`

**When required**: Always (local builds, CI/CD, pre-commit)
**CI enforcement**: Build fails on any warning

**→ See [linting.md § Compiler Warnings](./linting.md) for common fixes**

---

## Static Analysis

**Tools**:

- **clang-tidy**: C/C++ practices, readability

**When required**:

- Memory management changes
- Security fixes
- Large refactors

**When recommended**:

- New features
- Complex code paths

**Skip**: Documentation, simple config changes

**→ See [linting.md § Static Analysis](./linting.md) for workflows**

---

## Python Code Quality

- **Standard**: PEP 8
- **Tool**: `pylint`
- **Checks**: Style, unused imports, missing docstrings, potential bugs

**When required**: Before committing to `ci/`, `tests/`, or new utilities

**→ See [linting.md § Python Linting](./linting.md) for workflow**

---

## Shell Script Quality

- **Tool**: `shellcheck`

**Checks**: Unquoted variables, missing error handling, unreachable code, portability

**Best practices**:

1. Quote variables: `"$var"` not `$var`
2. Use strict mode: `set -euo pipefail`
3. Check exit codes: `command || exit 1`

**When required**: Before committing build scripts, test scripts, new utilities

**→ See [linting.md § Shell Script Linting](./linting.md) for workflow**

---

## Code Review Standards

**Automatic checks** (CI-enforced): Formatting, warnings, tests pass

**Manual review focus**:

- Correctness: Logic handles edge cases, errors checked, resources cleaned
- Testing: New tests added, error paths covered, regression tests for bugs
- Code quality: No duplication, appropriate abstractions
- Documentation: Public APIs documented, complex logic explained

**Common issues**:

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

| Situation      | Tool               | Required    |
| -------------- | ------------------ | ----------- |
| Every commit   | clang-format       | Yes         |
| Every build    | -Wall -Werror      | Yes         |
| Python changes | pylint             | Yes         |
| Shell changes  | shellcheck         | Yes         |
| Memory changes | scan-build         | Yes         |
| Large refactor | scan-build + tests | Recommended |
| Security fixes | All tools          | Yes         |
| Documentation  | None               | N/A         |

### Change Type Requirements

| Change Type    | Requirements                                                   |
| -------------- | -------------------------------------------------------------- |
| Small bug fix  | Format, build, unit tests, regression test                     |
| Feature        | Format, build, tests, documentation                            |
| Refactoring    | Format, build, all tests, static analysis (recommended)        |
| Memory changes | Format, build, tests, static analysis, Valgrind                |
| Security fix   | Format, build, tests, static analysis, regression test, review |

**→ See [linting.md § Pre-Commit Checklist](./linting.md) for complete workflow**

---

## CI Quality Enforcement

**Every PR runs**:

- Formatting
- Build (multiple configs)
- Warnings check
- Unit tests
- Spec tests
- Regression tests
- Platform builds (Linux/macOS/Windows/embedded)

**→ See [linting.md § Reproducing CI Failures](./linting.md) for troubleshooting**

---

## Related Documentation

- **[linting.md](linting.md)** - Complete pre-commit checklist
- **[building.md](building.md)** - Build configuration
- **[testing.md](testing.md)** - Testing strategy
- **[debugging.md](debugging.md)** - Debugging with GDB and Valgrind
