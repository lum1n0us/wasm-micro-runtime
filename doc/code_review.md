# Code Review Guide

This guide defines code review principles and practices for WAMR development.

---

## Review Principles

### 0. Match Requirements

- The change meets the stated requirements
- The change fixes the stated issue

### 1. Minimize Changes

- Keep changes focused and surgical
- Avoid unnecessary refactoring in bug fixes
- Do not mix formatting changes with functional changes

### 2. Logic & Correctness

**Manual review focus**:

- Logic handles edge cases
- Errors are checked
- Resources are cleaned up
- Boundary conditions: null inputs, empty data, overflow cases
- Error paths: all error conditions are handled properly
- Resource cleanup: memory, file handles, sockets are freed in all code paths

### 3. Readability & Maintainability

- Code uses clear names that are self-documenting
- Complex logic has explanatory comments
- No unnecessary duplication
- Appropriate abstractions (not over-engineered)
- Public APIs are documented
- Code is consistent with existing codebase style

### 4. Performance & Efficiency

**Algorithm complexity**:

- Time complexity is appropriate for the use case
- No unnecessary nested loops or redundant operations

**CPU load**:

- Hot paths are optimized
- No blocking operations in critical sections

**Memory consumption**:

- Memory allocations are justified
- No memory leaks or excessive usage
- Stack vs heap usage is appropriate

**Binary size**:

- Build artifact size impact is considered
- Especially important for embedded targets
- Avoid linking unnecessary dependencies

### 5. Security

**Error handling**:

- Input validation at system boundaries
- Return values are checked for risky operations
- Error messages do not leak sensitive information

**Logging**:

- Appropriate log levels are used
- Security-sensitive operations are logged
- No logging of secrets or credentials

**Resource limits**:

- Buffer sizes are validated
- Resource exhaustion is prevented
- Integer overflow is checked where needed

### 6. Consistency & Collaboration

**Follow existing patterns**:

- Use established frameworks and patterns
- Match existing code structure
- Use consistent naming conventions
- Follow project architecture principles

**Testing requirements**:

- Unit tests for new functionality
- Error paths are covered
- Regression tests for bug fixes
- Integration tests for large features

**Documentation updates**:

- Update relevant doc/*.md files for API changes
- Add or update samples for significant features
- Adjust existing samples if behavior changes

---

## Review Checklist

### 0. Requirements Match

- [ ] Change meets stated requirements or fixes stated issue
- [ ] No scope creep beyond original intent

### 1. Minimal Changes

- [ ] Changes are focused and surgical
- [ ] No unnecessary refactoring mixed with bug fixes
- [ ] Formatting changes are separated from functional changes
- [ ] One logical change per PR

### 2. Logic & Correctness

- [ ] Logic handles all edge cases
- [ ] Boundary conditions are checked (null, empty, overflow)
- [ ] Error conditions are properly handled
- [ ] Resources are cleaned up in all code paths
- [ ] No obvious bugs or logic errors

### 3. Readability & Maintainability

- [ ] Variable and function names are clear
- [ ] Complex logic has explanatory comments
- [ ] No unnecessary code duplication
- [ ] Appropriate level of abstraction (not over-engineered)
- [ ] Consistent with existing codebase style

### 4. Performance & Efficiency

- [ ] Algorithm complexity is acceptable for use case
- [ ] No unnecessary nested loops or redundant operations
- [ ] Hot paths are optimized appropriately
- [ ] Memory allocations are justified
- [ ] Memory usage is reasonable (no leaks or excessive usage)
- [ ] Binary size impact is acceptable (especially for embedded targets)

### 5. Security

- [ ] Input validation at system boundaries
- [ ] No buffer overflows possible
- [ ] Return values are checked for risky operations
- [ ] Error messages do not leak sensitive information
- [ ] Appropriate log levels are used
- [ ] No logging of secrets or credentials
- [ ] Resource limits are validated (buffer sizes, integer overflow)

### 6. Consistency & Collaboration

**Pattern consistency**:
- [ ] Uses established frameworks and patterns
- [ ] Matches existing code structure
- [ ] Naming is consistent with codebase
- [ ] Follows project architecture principles

**Testing**:
- [ ] Unit tests are added for new functionality
- [ ] Error paths are covered in tests
- [ ] Regression test is included (for bug fixes)
- [ ] Integration tests are updated (for large features)

**Documentation**:
- [ ] Public APIs are documented
- [ ] Complex logic is explained in comments
- [ ] Relevant doc/*.md files are updated (for API changes)
- [ ] Samples are added or updated (for significant features)

---

## Related Documentation

- **[linting.md](linting.md)** - Pre-commit checklist
- **[code_quality.md](code_quality.md)** - Quality standards
- **[testing.md](testing.md)** - Testing requirements
