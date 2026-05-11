# Code Review Guide

This guide defines code review principles and practices for WAMR development.

---

## Review Principles

### 1. Minimize Changes

- Keep changes focused and surgical
- One logical change per PR
- Avoid unnecessary refactoring in bug fixes
- Don't mix formatting changes with functional changes

### 2. Logic & Correctness

**Manual review focus**:

- Correctness: Logic handles edge cases, errors checked, resources cleaned
- Boundary conditions: null inputs, empty data, overflow cases
- Error paths: All error conditions handled properly
- Resource cleanup: Memory, file handles, sockets freed in all paths

**Common issues**:

```c
// ❌ Leak in error path
char *buf = malloc(1024);
if (error) return -1;  // Forgot free()

// ✅ Cleanup all paths
char *buf = malloc(1024);
if (error) { free(buf); return -1; }
```

### 3. Readability & Maintainability

- Code is self-documenting with clear names
- Complex logic has explanatory comments
- No unnecessary duplication
- Appropriate abstractions (not over-engineered)
- Public APIs documented
- Consistent with existing codebase style

### 4. Performance & Efficiency

**Algorithm complexity**:
- Time complexity appropriate for use case
- No unnecessary nested loops or redundant operations

**CPU load**:
- Hot paths optimized
- No blocking operations in critical sections

**Memory consumption**:
- Memory allocations justified
- No memory leaks or excessive usage
- Stack vs heap usage appropriate

**Binary size**:
- Build artifact size impact considered
- Especially important for embedded targets
- Avoid linking unnecessary dependencies

### 5. Security

**Error handling**:
- Input validation at system boundaries
- Return values checked for risky operations
- Error messages don't leak sensitive information

**Logging**:
- Appropriate log levels used
- Security-sensitive operations logged
- No logging of secrets or credentials

**Resource limits**:
- Buffer sizes validated
- Resource exhaustion prevented
- Integer overflow checks where needed

### 6. Consistency & Collaboration

**Follow existing patterns**:
- Use established frameworks and patterns
- Match existing code structure
- Consistent naming conventions
- Follow project architecture principles

**Testing requirements**:
- Unit tests for new functionality
- Error path coverage
- Regression tests for bug fixes
- Integration tests for large features

**Documentation updates**:
- Update relevant doc/*.md files for API changes
- Add or update samples for significant features
- Adjust existing samples if behavior changes

---

## Review Workflow

### For Authors

**Before submitting**:
1. Self-review: Read your own diff
2. Run pre-commit checklist (see [linting.md](./linting.md))
3. Ensure CI passes
4. Write clear PR description explaining the change

**During review**:
- Respond promptly to feedback
- Ask questions if feedback is unclear
- Push fixes as new commits (easier to review)

### For Reviewers

**Automatic checks** (CI-enforced):
- Formatting
- Compiler warnings
- Tests pass

**Review priorities**:
1. Correctness (does it work? does it break anything?)
2. Security (any vulnerabilities?)
3. Testing (adequate coverage?)
4. Maintainability (can others understand/modify it?)
5. Performance (any regressions?)

**Providing feedback**:
- Be specific and constructive
- Distinguish between blocking issues and suggestions
- Explain the reasoning behind feedback
- Acknowledge good solutions

---

## Review Checklist

### Correctness
- [ ] Logic handles all edge cases
- [ ] Error conditions properly checked
- [ ] Resources cleaned up in all paths
- [ ] No obvious bugs or logic errors

### Security
- [ ] Input validation present
- [ ] No buffer overflows possible
- [ ] Error handling doesn't leak information
- [ ] Logging appropriate and safe

### Performance
- [ ] Algorithm complexity acceptable
- [ ] No obvious performance regressions
- [ ] Memory usage reasonable
- [ ] Binary size impact acceptable (if applicable)

### Testing
- [ ] New tests added for functionality
- [ ] Error paths covered
- [ ] Regression test included (for bug fixes)
- [ ] Integration tests updated (for large features)

### Documentation
- [ ] Public APIs documented
- [ ] Complex logic explained
- [ ] Relevant doc/*.md files updated
- [ ] Samples added/updated (if significant feature)

### Consistency
- [ ] Follows existing code patterns
- [ ] Matches project architecture
- [ ] Naming consistent with codebase
- [ ] No unnecessary changes

---

## Common Review Issues

### Memory Management
```c
// ❌ Missing cleanup in error path
void *data = malloc(size);
if (validate(data) < 0)
    return -1;  // Leak!

// ✅ Cleanup all paths
void *data = malloc(size);
if (validate(data) < 0) {
    free(data);
    return -1;
}
```

### Error Handling
```c
// ❌ Unchecked return value
file = fopen(path, "r");
fread(buf, 1, 100, file);  // Crash if fopen failed!

// ✅ Check errors
file = fopen(path, "r");
if (!file) return -1;
if (fread(buf, 1, 100, file) != 100) {
    fclose(file);
    return -1;
}
```

### Resource Cleanup
```c
// ❌ File not closed in error path
FILE *f = fopen(path, "r");
if (process(f) < 0)
    return -1;  // File leak!
fclose(f);

// ✅ Close in all paths
FILE *f = fopen(path, "r");
int ret = process(f);
fclose(f);
if (ret < 0)
    return -1;
```

---

## Related Documentation

- **[linting.md](linting.md)** - Pre-commit checklist
- **[code_quality.md](code_quality.md)** - Quality standards
- **[testing.md](testing.md)** - Testing requirements
