# Prompt Template: Add Tests to Undertested Code in WAMR

## Initial Prompt for New Session

```markdown
You are adding tests to improve test coverage in the WAMR (WebAssembly Micro Runtime) project.

## ⚠️ CRITICAL: Development Environment

This project uses devcontainer for ALL development activities.
ALL test commands MUST use:

./scripts/in-container.sh "<command>"

## Your Mission

Add tests for: [PASTE COMPONENT/FUNCTION/FEATURE HERE]

Example targets:
- A specific function in VMcore (e.g., wasm_runtime_malloc)
- An interpreter feature (e.g., multi-value returns)
- A platform abstraction (e.g., file I/O operations)
- An AOT optimization (e.g., constant folding)

## Getting Started

1. **First, read the navigation guide:**
   - Read `AGENTS.md` completely
   - Follow "For Test Writing" section

2. **Understand the testing strategy:**
   - Read `doc/testing.md` for test structure and patterns
   - Browse `tests/unit/` for existing examples
   - Check `tests/wamr-test-suites/` for spec test patterns

3. **Understand the code you're testing:**
   - Read `doc/architecture-overview.md` to understand the component
   - Read the actual source code for the feature
   - Understand inputs, outputs, and edge cases

## Step-by-Step Testing Workflow

### Phase 1: Analyze What Needs Testing

```bash
# Find the code to test
./scripts/in-container.sh "find core/iwasm -name '*.c' | xargs grep -l 'function_name'"

# Read the code
./scripts/in-container.sh "cat path/to/file.c | grep -A 20 'function_name'"

# Check existing tests
./scripts/in-container.sh "find tests -name '*test*.c' | xargs grep -l 'function_name'"
```

**Identify:**
- What does this code do?
- What are the inputs and outputs?
- What are the edge cases?
- What can go wrong?
- What's currently NOT tested?

### Phase 2: Choose Test Type

Based on `doc/testing.md`:

**Unit Tests** (for isolated functions):
- Location: `tests/unit/<component>/`
- Use CTest framework
- Test individual functions

**Integration Tests** (for features):
- Location: `samples/` or new sample
- Test end-to-end workflows
- Use actual Wasm modules

**Spec Tests** (for WebAssembly conformance):
- Location: `tests/wamr-test-suites/spec-test-script/`
- Test WebAssembly spec compliance

### Phase 3: Write Tests

Follow patterns from `doc/testing.md` and existing tests:

**Unit Test Template:**

```c
#include <assert.h>
#include "wasm_runtime.h"

// Test successful case
void test_function_name_success() {
    // Setup
    wasm_runtime_init();
    
    // Execute
    int result = function_to_test(valid_input);
    
    // Verify
    assert(result == expected_value);
    assert(side_effect_happened());
    
    // Cleanup
    wasm_runtime_destroy();
}

// Test error case
void test_function_name_null_input() {
    wasm_runtime_init();
    
    // Should handle NULL gracefully
    int result = function_to_test(NULL);
    
    assert(result == ERROR_CODE);
    
    wasm_runtime_destroy();
}

// Test edge case
void test_function_name_boundary() {
    wasm_runtime_init();
    
    // Test with boundary values
    int result = function_to_test(MAX_VALUE);
    assert(result == expected_max_result);
    
    result = function_to_test(MIN_VALUE);
    assert(result == expected_min_result);
    
    wasm_runtime_destroy();
}

int main() {
    test_function_name_success();
    test_function_name_null_input();
    test_function_name_boundary();
    
    printf("All tests passed!\n");
    return 0;
}
```

### Phase 4: Add Test to Build System

```cmake
# In tests/unit/<component>/CMakeLists.txt or create new one

add_executable(test_my_feature test_my_feature.c)
target_link_libraries(test_my_feature iwasm_static)
add_test(NAME test_my_feature COMMAND test_my_feature)
```

### Phase 5: Build and Run Tests

Follow `doc/building.md` and `doc/testing.md`:

```bash
# Build with tests
./scripts/in-container.sh "cmake -B build-test -DWAMR_BUILD_INTERP=1 -DWAMR_BUILD_FAST_INTERP=1"
./scripts/in-container.sh "cmake --build build-test -j\$(nproc)"

# Run your new test
./scripts/in-container.sh "cd build-test && ctest -R test_my_feature -V"

# Run all tests to ensure nothing broke
./scripts/in-container.sh "cd build-test && ctest --output-on-failure"
```

### Phase 6: Test Edge Cases

Make sure you test:
- [ ] Happy path (normal input, expected output)
- [ ] NULL/invalid inputs
- [ ] Boundary values (max, min, zero)
- [ ] Memory allocation failures (if applicable)
- [ ] Concurrent access (if relevant)
- [ ] Error conditions
- [ ] Resource cleanup

### Phase 7: Code Quality

Follow `doc/code-quality.md`:

```bash
# Format your test code
./scripts/in-container.sh "clang-format-14 -i tests/unit/my-component/test_my_feature.c"

# Check for warnings
./scripts/in-container.sh "cmake --build build-test 2>&1 | grep -i warning"

# Run under Valgrind to check for leaks
./scripts/in-container.sh "valgrind --leak-check=full ./build-test/tests/unit/my-component/test_my_feature"
```

### Phase 8: Document Your Tests

In your test file, add comments:

```c
/**
 * Test suite for my_feature
 * 
 * Coverage:
 * - test_my_feature_success: Tests normal operation
 * - test_my_feature_null_input: Tests NULL handling
 * - test_my_feature_boundary: Tests edge cases
 * 
 * Not covered (future work):
 * - Concurrent access scenarios
 * - Integration with other components
 */
```

### Phase 9: Prepare Commit

```bash
# Check what you're committing
git status
git diff

# Commit with clear message
git add tests/unit/my-component/test_my_feature.c
git commit -m "test(my-component): add tests for my_feature

- Test normal operation
- Test error handling (NULL, invalid inputs)
- Test boundary conditions
- All tests pass under Valgrind

Increases test coverage for core/iwasm/common/my_feature.c"
```

## Test Best Practices (from doc/testing.md)

1. **One assertion per test** - Each test should verify one specific behavior
2. **Test both success and failure** - Cover happy path and error cases
3. **Clean up resources** - Always free memory, unload modules
4. **Use meaningful names** - Test name should describe what's tested
5. **Keep tests independent** - Tests should not depend on each other
6. **Fast tests** - Unit tests should run in milliseconds
7. **Deterministic** - Tests should always produce the same result

## Container Wrapper Tips

```bash
# Run tests in verbose mode
./scripts/in-container.sh "cd build-test && ctest -V"

# Run only your new tests
./scripts/in-container.sh "cd build-test && ctest -R my_feature"

# Run with memory checking
./scripts/in-container.sh "valgrind --leak-check=full ./build-test/path/to/test"

# Check test infrastructure
./scripts/in-container.sh "ls -l tests/unit/"
```

## 📝 IMPORTANT: Track Your Feedback

As you write tests, please document:

### What Worked Well
- Was doc/testing.md sufficient?
- Were existing test examples helpful?
- Did the container wrapper make testing smooth?
- What made the workflow easy?

### What Could Be Better
- What testing patterns were missing?
- What questions couldn't you answer?
- What test infrastructure issues did you encounter?
- What would make test writing easier?

### Specific Gaps
- Missing test utilities?
- Unclear test patterns?
- Build system issues?
- Container tool problems?

**Save feedback to:** `.superpowers/feedback/task-3-add-tests-feedback.md`

Or bring it back to the "Make WAMR be friendly to AI" session.

## Testing Checklist

Before you're done:

- [ ] Tests cover normal operation
- [ ] Tests cover error cases
- [ ] Tests cover edge cases/boundaries
- [ ] Tests are independent (can run in any order)
- [ ] Tests clean up resources properly
- [ ] Tests run quickly (< 1 second each)
- [ ] Tests pass under Valgrind (no leaks)
- [ ] Code is formatted with clang-format-14
- [ ] Test names are descriptive
- [ ] Commit message is clear

## Examples to Study

Look at existing tests:

```bash
# Browse unit tests
./scripts/in-container.sh "ls -R tests/unit/"

# Read a sample test
./scripts/in-container.sh "cat tests/unit/gc-test/gc_test.c | head -50"

# See how tests are built
./scripts/in-container.sh "cat tests/unit/gc-test/CMakeLists.txt"
```

## Ready?

Start by reading AGENTS.md → "For Test Writing", then analyze the code you're testing.

Remember: This validates our testing infrastructure and documentation. Your feedback will improve the test-writing experience for everyone.

Good luck! 🧪
```

## Tips for the Human Supervisor

- Good tests are specific and focused
- Encourage testing error paths, not just happy paths
- Tests should be fast and deterministic
- Memory leaks in tests are bugs too (use Valgrind)
- Better to have many small tests than one giant test
- Test code quality matters too (formatting, clarity)

## Success Criteria

- [ ] Tests are written and passing
- [ ] Tests cover success and failure cases
- [ ] Tests are properly integrated into build system
- [ ] No memory leaks (Valgrind clean)
- [ ] Code is formatted
- [ ] Commit is ready
- [ ] Feedback is documented
