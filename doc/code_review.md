# Code Review Guide

This guide defines code review principles and practices for WAMR development.

---

## Review Principles

### 0. Match requirements

- Ensure the change meets the stated requirements
- Or the change fixes the stated issue

### 1. Minimize Changes

- Keep changes focused and surgical
- Avoid unnecessary refactoring in bug fixes
- Don't mix formatting changes with functional changes

### 2. Logic & Correctness

**Manual review focus**:

- Correctness: Logic handles edge cases, errors checked, resources cleaned
- Boundary conditions: null inputs, empty data, overflow cases
- Error paths: All error conditions handled properly
- Resource cleanup: Memory, file handles, sockets freed in all paths

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

- Update relevant doc/\*.md files for API changes
- Add or update samples for significant features
- Adjust existing samples if behavior changes

---

## Review Checklist

### 0. Requirements Match

- [ ] Change meets stated requirements or fixes stated issue
- [ ] No scope creep beyond original intent

### 1. Minimal Changes

- [ ] Changes focused and surgical
- [ ] No unnecessary refactoring mixed with bug fixes
- [ ] Formatting changes separated from functional changes
- [ ] One logical change per PR

### 2. Logic & Correctness

- [ ] Logic handles all edge cases
- [ ] Boundary conditions checked (null, empty, overflow)
- [ ] Error conditions properly handled
- [ ] Resources cleaned up in all code paths
- [ ] No obvious bugs or logic errors

### 3. Readability & Maintainability

- [ ] Clear variable and function names
- [ ] Complex logic has explanatory comments
- [ ] No unnecessary code duplication
- [ ] Appropriate level of abstraction (not over-engineered)
- [ ] Consistent with existing codebase style

### 4. Performance & Efficiency

- [ ] Algorithm complexity acceptable for use case
- [ ] No unnecessary nested loops or redundant operations
- [ ] Hot paths optimized appropriately
- [ ] Memory allocations justified
- [ ] Memory usage reasonable (no leaks or excessive usage)
- [ ] Binary size impact acceptable (especially for embedded)

### 5. Security

- [ ] Input validation at system boundaries
- [ ] No buffer overflows possible
- [ ] Return values checked for risky operations
- [ ] Error messages don't leak sensitive information
- [ ] Appropriate log levels used
- [ ] No logging of secrets or credentials
- [ ] Resource limits validated (buffer sizes, integer overflow)

### 6. Consistency & Collaboration

**Pattern consistency**:
- [ ] Uses established frameworks and patterns
- [ ] Matches existing code structure
- [ ] Naming consistent with codebase
- [ ] Follows project architecture principles

**Testing**:
- [ ] Unit tests added for new functionality
- [ ] Error paths covered in tests
- [ ] Regression test included (for bug fixes)
- [ ] Integration tests updated (for large features)

**Documentation**:
- [ ] Public APIs documented
- [ ] Complex logic explained in comments
- [ ] Relevant doc/*.md files updated (for API changes)
- [ ] Samples added/updated (for significant features)

---

## Related Documentation

- **[linting.md](linting.md)** - Pre-commit checklist
- **[code_quality.md](code_quality.md)** - Quality standards
- **[testing.md](testing.md)** - Testing requirements
