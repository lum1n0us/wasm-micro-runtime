# Code Quality and Formatting Guide

## Overview

This guide covers code quality standards, formatting rules, and static analysis tools used in the WAMR project. Following these practices ensures consistent, maintainable, and high-quality code across the codebase.

### Quality Standards

WAMR maintains strict code quality standards:
- **Formatting**: Consistent code style enforced by clang-format-14
- **Linting**: Static analysis for C/C++, Python, and shell scripts
- **Compiler Warnings**: Strict warning levels (-Wall, -Wextra, -Werror)
- **Static Analysis**: Additional checks via clang static analyzer
- **Pre-commit Checks**: Automated quality gates before commits
- **CI Enforcement**: Quality checks in continuous integration pipeline

---

## Prerequisites

All code quality tools are pre-installed in the devcontainer environment. Before using these tools, ensure you're working within the container:

- **For AI Agents**: Use `scripts/in-container.sh` wrapper for ALL commands
- **For Humans**: Open project in VS Code devcontainer or use `docker exec`

See **[dev-in-container.md](dev-in-container.md)** for complete container setup and usage instructions.

---

## For AI Agents

**CRITICAL REQUIREMENT**: All code quality commands MUST be executed inside the devcontainer using the `scripts/in-container.sh` wrapper script.

### Basic Pattern

```bash
scripts/in-container.sh "<quality-command>"
```

### Common Quality Workflows

**Format check before commit:**
```bash
# Check formatting (dry run, non-destructive)
scripts/in-container.sh "find core/iwasm -name '*.c' -o -name '*.h' | xargs clang-format-14 --dry-run --Werror"
```

**Format staged changes:**
```bash
# Format only files staged for commit
scripts/in-container.sh "git diff --cached --name-only --diff-filter=ACM | grep -E '\.(c|h|cpp|hpp)$' | xargs -r clang-format-14 -i"
```

**Full project format:**
```bash
# Format all C/C++ files (use with caution)
scripts/in-container.sh "find core product-mini samples -type f \( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.hpp' \) -exec clang-format-14 -i {} +"
```

**Python linting:**
```bash
# Lint Python scripts
scripts/in-container.sh "pylint ci/coding_guidelines_check.py ci/validate_lldb.py"
```

**Shell script checks:**
```bash
# Check shell scripts for issues
scripts/in-container.sh "shellcheck scripts/in-container.sh .devcontainer/finalize.sh"
```

---

## Code Formatting (clang-format)

WAMR uses **clang-format-14** for consistent C/C++ code formatting. The configuration is defined in `.clang-format` at the project root.

### Formatting Configuration

The project uses a Mozilla-based style with customizations:
- **Indentation**: 4 spaces (no tabs)
- **Line Length**: 80 characters maximum
- **Braces**: Custom placement (functions on new line, control statements on same line)
- **Pointer Alignment**: Right-aligned (`char *ptr` not `char* ptr`)
- **Base Style**: Mozilla

Configuration file: `.clang-format`

### Check Formatting (Dry Run)

Before making changes, check if files conform to the style:

```bash
# Check single file
scripts/in-container.sh "clang-format-14 --dry-run --Werror core/iwasm/common/wasm_runtime_common.c"

# Check multiple files
scripts/in-container.sh "clang-format-14 --dry-run --Werror core/iwasm/common/*.c"

# Check all C files in a directory recursively
scripts/in-container.sh "find core/iwasm/common -name '*.c' | xargs clang-format-14 --dry-run --Werror"
```

**Exit codes:**
- `0` - All files properly formatted
- `1` - Formatting issues found (if `--Werror` flag used)

### Auto-Format Code

Apply formatting fixes automatically:

```bash
# Format single file
scripts/in-container.sh "clang-format-14 -i core/iwasm/common/wasm_runtime_common.c"

# Format multiple files
scripts/in-container.sh "clang-format-14 -i core/iwasm/common/*.c"

# Format all C/C++ files in directory
scripts/in-container.sh "find core/iwasm/interpreter -type f \( -name '*.c' -o -name '*.h' \) -exec clang-format-14 -i {} +"
```

**Important**: The `-i` flag modifies files in-place. Always commit or backup before bulk formatting.

### Format Staged Git Changes

Format only files you're about to commit:

```bash
# Format staged C/C++ files
scripts/in-container.sh "git diff --cached --name-only --diff-filter=ACM | grep -E '\.(c|h|cpp|hpp)$' | xargs -r clang-format-14 -i"

# Then re-stage the formatted files
scripts/in-container.sh "git add -u"
```

### Format Modified Files

Format only files changed in your working directory:

```bash
# Format all modified C/C++ files
scripts/in-container.sh "git diff --name-only --diff-filter=ACM | grep -E '\.(c|h|cpp|hpp)$' | xargs -r clang-format-14 -i"
```

### Format Changes in Pull Request

Format files changed since branching from main:

```bash
# Show files that would be formatted
scripts/in-container.sh "git diff --name-only main...HEAD | grep -E '\.(c|h|cpp|hpp)$'"

# Format all changed files
scripts/in-container.sh "git diff --name-only main...HEAD | grep -E '\.(c|h|cpp|hpp)$' | xargs -r clang-format-14 -i"
```

### Bulk Formatting Patterns

**Format entire component:**
```bash
# Format interpreter module
scripts/in-container.sh "find core/iwasm/interpreter -type f \( -name '*.c' -o -name '*.h' \) -exec clang-format-14 -i {} +"

# Format AOT module
scripts/in-container.sh "find core/iwasm/aot -type f \( -name '*.c' -o -name '*.h' \) -exec clang-format-14 -i {} +"
```

**Format with exclusions:**
```bash
# Format core but skip third-party code
scripts/in-container.sh "find core/iwasm -type f \( -name '*.c' -o -name '*.h' \) ! -path '*/deps/*' -exec clang-format-14 -i {} +"
```

### Formatting Best Practices

1. **Format before committing**: Always run format checks before creating commits
2. **Format incrementally**: Format only files you modify, not the entire codebase
3. **Separate formatting commits**: Keep formatting changes separate from functional changes
4. **Check diff carefully**: Review formatting changes to ensure no unintended modifications
5. **Use dry-run first**: Always test with `--dry-run --Werror` before applying `-i`

---

## Static Analysis

### Compiler Warnings

WAMR uses strict compiler warning flags to catch potential issues:

```bash
# Build with all warnings enabled
scripts/in-container.sh "cmake -B build -DCMAKE_C_FLAGS='-Wall -Wextra -Werror' && cmake --build build"

# Build specific component with strict warnings
scripts/in-container.sh "cd core/iwasm/common && gcc -Wall -Wextra -Werror -c *.c"
```

**Key warning flags:**
- `-Wall` - Enable all standard warnings
- `-Wextra` - Enable additional warnings beyond -Wall
- `-Werror` - Treat warnings as errors (build fails on warnings)

### Clang Static Analyzer

The Clang static analyzer performs deeper analysis than compiler warnings:

```bash
# Analyze single file
scripts/in-container.sh "clang --analyze -Xanalyzer -analyzer-output=text core/iwasm/common/wasm_runtime_common.c"

# Analyze entire project with scan-build
scripts/in-container.sh "scan-build cmake -B build && scan-build cmake --build build"

# Generate HTML report
scripts/in-container.sh "scan-build -o scan-reports cmake --build build"
```

**Common issues detected:**
- Memory leaks
- Use-after-free
- Null pointer dereferences
- Uninitialized variables
- Dead code

### Integration with Build System

Add static analysis to CMake configuration:

```bash
# Enable static analysis in build
scripts/in-container.sh "cmake -B build -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_FLAGS='-Wall -Wextra -Werror'"
```

---

## Python Code Quality

### Linting with pylint

WAMR includes Python scripts for testing and tooling. Lint these with pylint:

```bash
# Lint single script
scripts/in-container.sh "pylint ci/coding_guidelines_check.py"

# Lint all Python files in directory
scripts/in-container.sh "find ci -name '*.py' -exec pylint {} +"

# Lint with specific configuration
scripts/in-container.sh "pylint --max-line-length=100 --disable=C0111 ci/*.py"
```

**Common pylint checks:**
- Code style violations
- Unused imports and variables
- Missing docstrings
- Naming conventions
- Potential bugs

### Formatting with black

Format Python code with black (if available):

```bash
# Check formatting
scripts/in-container.sh "black --check ci/"

# Apply formatting
scripts/in-container.sh "black ci/"

# Format with line length limit
scripts/in-container.sh "black --line-length=100 ci/"
```

### Python Best Practices

1. **PEP 8 compliance**: Follow Python style guidelines
2. **Type hints**: Use type annotations where beneficial
3. **Docstrings**: Document functions and modules
4. **Error handling**: Proper exception handling and logging
5. **Testing**: Include unit tests for Python utilities

---

## Shell Script Quality

### ShellCheck

ShellCheck analyzes shell scripts for common issues:

```bash
# Check single script
scripts/in-container.sh "shellcheck scripts/in-container.sh"

# Check all shell scripts
scripts/in-container.sh "find . -name '*.sh' -type f -exec shellcheck {} +"

# Check with specific severity level
scripts/in-container.sh "shellcheck --severity=warning scripts/in-container.sh"

# Exclude specific warnings
scripts/in-container.sh "shellcheck --exclude=SC2086,SC2046 scripts/in-container.sh"
```

**Common shellcheck warnings:**
- Unquoted variables (SC2086)
- Missing quotes (SC2046)
- Useless use of cat (SC2002)
- Unreachable code (SC2317)

### Shell Script Best Practices

1. **Quote variables**: Always quote `"$variable"` to handle spaces
2. **Use shellcheck**: Run shellcheck on all scripts before commit
3. **Set strict mode**: Use `set -euo pipefail` for safer scripts
4. **Consistent style**: Follow project conventions for indentation and naming
5. **Error handling**: Check exit codes and provide meaningful error messages

---

## Pre-commit Checks

### Manual Pre-commit Workflow

Before committing, run these checks:

```bash
# 1. Format check
scripts/in-container.sh "git diff --cached --name-only | grep -E '\.(c|h)$' | xargs -r clang-format-14 --dry-run --Werror"

# 2. If formatting issues, fix them
scripts/in-container.sh "git diff --cached --name-only | grep -E '\.(c|h)$' | xargs -r clang-format-14 -i"

# 3. Re-stage formatted files
scripts/in-container.sh "git add -u"

# 4. Check for shell script issues
scripts/in-container.sh "git diff --cached --name-only | grep '\.sh$' | xargs -r shellcheck"

# 5. Run quick build test
scripts/in-container.sh "cmake --build build"

# 6. Now commit
scripts/in-container.sh "git commit"
```

### Automated Pre-commit Hooks

Install git hooks to automate checks (optional):

```bash
# Create pre-commit hook
cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash
# Run format check on staged files
exec scripts/in-container.sh "git diff --cached --name-only | grep -E '\.(c|h)$' | xargs -r clang-format-14 --dry-run --Werror"
EOF

chmod +x .git/hooks/pre-commit
```

---

## CI Quality Gates

### GitHub Actions Integration

WAMR's CI pipeline enforces quality checks on all pull requests:

1. **Format verification**: Checks all C/C++ files conform to clang-format rules
2. **Build with warnings**: Builds with `-Wall -Wextra -Werror`
3. **Static analysis**: Runs clang static analyzer on changed files
4. **Linting**: Checks Python and shell scripts
5. **Test execution**: Runs full test suite

### CI Quality Script

The CI uses automated quality checks:

```bash
# Run same checks as CI locally
scripts/in-container.sh "python3 ci/coding_guidelines_check.py"
```

### Pre-CI Validation

Before pushing, validate your changes will pass CI:

```bash
# Check formatting
scripts/in-container.sh "find core product-mini -name '*.c' -o -name '*.h' | xargs clang-format-14 --dry-run --Werror"

# Build with strict warnings
scripts/in-container.sh "cmake -B build -DCMAKE_C_FLAGS='-Wall -Wextra -Werror' && cmake --build build"

# Run tests
scripts/in-container.sh "cd build && ctest --output-on-failure"
```

---

## Best Practices

### Code Style Guidelines

1. **Follow .clang-format**: Trust the automated formatting
2. **80-character lines**: Keep lines under 80 characters when possible
3. **Consistent indentation**: Use 4 spaces (never tabs)
4. **Meaningful names**: Use descriptive variable and function names
5. **Comments**: Document complex logic and non-obvious behavior

### Quality Workflow

1. **Before making changes**:
   - Pull latest code
   - Build successfully
   - Run existing tests

2. **While developing**:
   - Write clean, readable code
   - Add tests for new functionality
   - Document public APIs

3. **Before committing**:
   - Format code with clang-format-14
   - Run static analysis
   - Build with strict warnings
   - Run affected tests

4. **Before pushing**:
   - Rebase on latest main
   - Run full test suite
   - Verify CI will pass

### Code Review Checklist

When reviewing code, check for:

- [ ] Code formatted with clang-format-14
- [ ] No compiler warnings with -Wall -Wextra -Werror
- [ ] No static analyzer warnings
- [ ] Shell scripts pass shellcheck
- [ ] Python scripts pass pylint
- [ ] Tests added for new functionality
- [ ] Documentation updated if needed
- [ ] Commit messages follow conventions

---

## Troubleshooting

### Formatting Issues

**Issue**: clang-format-14 not found

```bash
# Verify clang-format-14 is available
scripts/in-container.sh "which clang-format-14"

# Check version
scripts/in-container.sh "clang-format-14 --version"

# If missing, rebuild container
# In VS Code: F1 → "Dev Containers: Rebuild Container"
```

**Issue**: Formatting produces unexpected changes

```bash
# Review changes before committing
scripts/in-container.sh "git diff"

# Verify .clang-format config is correct
cat .clang-format

# Test on single file first
scripts/in-container.sh "clang-format-14 -i test_file.c"
```

**Issue**: Files keep showing formatting issues after fixing

```bash
# Ensure you're formatting with correct version
scripts/in-container.sh "clang-format-14 --version"
# Should show: clang-format version 14.x.x

# Clear any cached state
scripts/in-container.sh "git clean -fdx"

# Re-apply formatting
scripts/in-container.sh "git diff --name-only | grep -E '\.(c|h)$' | xargs -r clang-format-14 -i"
```

### Static Analysis Issues

**Issue**: Too many analyzer warnings

```bash
# Focus on high-severity issues first
scripts/in-container.sh "scan-build -enable-checker security cmake --build build"

# Suppress false positives (carefully)
# Add comments in code: // NOLINT or /* NOLINTNEXTLINE */
```

**Issue**: Analyzer takes too long

```bash
# Analyze only changed files
scripts/in-container.sh "git diff --name-only main...HEAD | grep '\.c$' | xargs clang --analyze"

# Use parallel analysis
scripts/in-container.sh "scan-build -j$(nproc) cmake --build build"
```

### Python/Shell Linting Issues

**Issue**: pylint or shellcheck not found

```bash
# Verify tools are installed
scripts/in-container.sh "which pylint shellcheck"

# Install if missing (should not be needed in devcontainer)
scripts/in-container.sh "apt-get update && apt-get install -y pylint shellcheck"
```

**Issue**: Too many linting warnings

```bash
# Fix issues incrementally
scripts/in-container.sh "pylint --disable=all --enable=E ci/coding_guidelines_check.py"

# Gradually enable more checks
scripts/in-container.sh "pylint --disable=all --enable=E,W ci/"
```

### Performance Issues

**Issue**: Quality checks take too long

```bash
# Check only modified files
scripts/in-container.sh "git diff --name-only | grep -E '\.(c|h)$' | xargs -r clang-format-14 --dry-run"

# Use parallel processing
scripts/in-container.sh "find core -name '*.c' | xargs -P$(nproc) -n1 clang-format-14 --dry-run"

# Skip third-party code
scripts/in-container.sh "find . -name '*.c' ! -path '*/deps/*' ! -path '*/third-party/*' -exec clang-format-14 -i {} +"
```

---

## Quick Reference

### Essential Commands

```bash
# Format check (dry run)
scripts/in-container.sh "clang-format-14 --dry-run --Werror <file>"

# Format file
scripts/in-container.sh "clang-format-14 -i <file>"

# Format staged changes
scripts/in-container.sh "git diff --cached --name-only | grep -E '\.(c|h)$' | xargs -r clang-format-14 -i"

# Check shell script
scripts/in-container.sh "shellcheck <script.sh>"

# Lint Python
scripts/in-container.sh "pylint <script.py>"

# Build with strict warnings
scripts/in-container.sh "cmake -B build -DCMAKE_C_FLAGS='-Wall -Wextra -Werror' && cmake --build build"
```

### File Patterns

```bash
# All C/C++ files
\( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.hpp' \)

# Exclude third-party
! -path '*/deps/*' ! -path '*/third-party/*'

# Only modified files
git diff --name-only --diff-filter=ACM

# Only staged files
git diff --cached --name-only --diff-filter=ACM
```

---

## Related Documentation

- **[dev-in-container.md](dev-in-container.md)** - Container setup and usage
- **[building.md](building.md)** - Build instructions and CMake configuration
- **[testing.md](testing.md)** - Test suite documentation (to be created)
- **[debugging.md](debugging.md)** - Debugging tools and techniques (to be created)
- **[AGENTS.md](../AGENTS.md)** - AI agent development guidelines

---

## External Resources

- [clang-format Documentation](https://clang.llvm.org/docs/ClangFormat.html)
- [Clang Static Analyzer](https://clang-analyzer.llvm.org/)
- [ShellCheck Wiki](https://github.com/koalaman/shellcheck/wiki)
- [Pylint Documentation](https://pylint.pycqa.org/)
- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)

---

**Documentation Version**: 1.0.0  
**Last Updated**: 2026-04-03  
**Maintained By**: WAMR Development Team
