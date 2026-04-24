# Prompt Template: Review an Actual PR in WAMR

## Initial Prompt for New Session

```markdown
You are reviewing a pull request for the WAMR (WebAssembly Micro Runtime) project.

## ⚠️ CRITICAL: Development Environment

This project uses devcontainer for ALL development activities.
ALL build/test/check commands MUST use:

./scripts/in-container.sh "<command>"

## Your Mission

Review this PR: [PASTE PR LINK HERE]

Example format: https://github.com/bytecodealliance/wasm-micro-runtime/pull/XXXX

## Getting Started

1. **First, read the navigation guide:**
   - Read `AGENTS.md` completely
   - Follow "For PR Reviews" section

2. **Understand review criteria:**
   - Read `doc/architecture-overview.md` for architectural principles
   - Understand dependency rules and design principles

## Step-by-Step Review Workflow

### Phase 1: Fetch and Checkout the PR

```bash
# Get PR number from the link (e.g., 3456)
PR_NUMBER=XXXX

# Fetch the PR
git fetch origin pull/$PR_NUMBER/head:pr-$PR_NUMBER

# Checkout the PR branch
git checkout pr-$PR_NUMBER

# Check what changed
git diff main...pr-$PR_NUMBER --stat
```

### Phase 2: Understand the Changes

```bash
# See the commits
git log main..pr-$PR_NUMBER --oneline

# Read the PR description (on GitHub)
# Understand: What problem does this solve? How does it solve it?

# Review the diff
git diff main...pr-$PR_NUMBER
```

### Phase 3: Build the Changes

Follow `doc/building.md`:

```bash
# Clean build with PR changes
./scripts/in-container.sh "rm -rf build-review"

# Configure (adjust options based on what the PR touches)
./scripts/in-container.sh "cmake -B build-review -DWAMR_BUILD_INTERP=1 -DWAMR_BUILD_FAST_INTERP=1 -DWAMR_BUILD_AOT=1 -DCMAKE_BUILD_TYPE=Debug"

# Build
./scripts/in-container.sh "cmake --build build-review -j\$(nproc)" 2>&1 | tee build.log

# Check for new warnings
grep -i warning build.log
```

### Phase 4: Run Tests

Follow `doc/testing.md`:

```bash
# Run unit tests
./scripts/in-container.sh "cd build-review && ctest --output-on-failure"

# Check if PR added new tests
git diff main...pr-$PR_NUMBER --name-only | grep test

# If relevant, run spec tests
./scripts/in-container.sh "cd tests/wamr-test-suites && ./test_wamr.sh"
```

### Phase 5: Code Quality Review

Follow `doc/code-quality.md`:

```bash
# Check formatting on changed files
CHANGED_FILES=$(git diff main...pr-$PR_NUMBER --name-only --diff-filter=ACMR | grep '\\.\\(c\\|h\\)$')

# Format check
for file in $CHANGED_FILES; do
    ./scripts/in-container.sh "clang-format-14 --dry-run --Werror $file" || echo "❌ $file needs formatting"
done

# Check for common issues
./scripts/in-container.sh "grep -n 'TODO\\|FIXME\\|XXX\\|HACK' $CHANGED_FILES" || echo "No TODOs found"
```

### Phase 6: Architecture Review

Read `doc/architecture-overview.md` and check:

**Dependency Rules:**
- [ ] Does the change respect the dependency graph?
- [ ] No upward dependencies (e.g., VMcore depending on iwasm)?
- [ ] Platform-specific code in correct locations?

**Design Principles:**
- [ ] Is the code portable (no platform-specific code in VMcore)?
- [ ] Is memory management correct (no leaks)?
- [ ] Are error paths handled properly?
- [ ] Is the API design clean and consistent?

**Component Boundaries:**
- [ ] If it touches VMcore (core/iwasm/, core/shared), is it truly core functionality?
- [ ] If it touches iwasm (product-mini/platforms), is it platform-specific?
- [ ] If it touches wamrc (wamr-compiler/), is it AOT-related?

### Phase 7: Functional Review

- [ ] Does the change actually solve the stated problem?
- [ ] Are there edge cases not covered?
- [ ] Is error handling adequate?
- [ ] Are the changes minimal (no unnecessary refactoring)?
- [ ] Is the commit message clear and follows convention?

### Phase 8: Write Review Comments

Structure your review:

**Summary:**
- What does this PR do?
- Overall assessment (Approve / Request Changes / Comment)

**Architectural Concerns (if any):**
- Dependency issues
- Design principle violations
- Component boundary issues

**Code Quality Issues (if any):**
- Formatting problems
- Missing tests
- Unclear code
- Potential bugs

**Positive Feedback:**
- What was done well
- Good design choices

**Questions:**
- Anything unclear that needs clarification

## Container Wrapper Tips

```bash
# Run multiple checks efficiently
./scripts/in-container.sh "cmake --build build-review && cd build-review && ctest"

# Debug a test failure
./scripts/in-container.sh "cd build-review && ctest -R failing_test -V"

# Quick status check
./scripts/in-container.sh --status
```

## 📝 IMPORTANT: Track Your Feedback

As you review, please document:

### What Worked Well
- Was AGENTS.md navigation helpful for reviews?
- Were architecture docs sufficient to judge the changes?
- Did the container wrapper make testing easy?
- What documentation was most valuable?

### What Could Be Better
- What questions couldn't you answer from docs?
- What architectural knowledge was missing?
- What made the review workflow difficult?
- What documentation gaps did you find?

### Specific Issues
- Any problems with the container wrapper?
- Any missing tools or commands?
- Any unclear review criteria?

**Save feedback to:** `.superpowers/feedback/task-2-pr-review-feedback.md`

Or bring it back to the "Make WAMR be friendly to AI" session.

## Review Checklist

Use this before finalizing your review:

- [ ] PR description read and understood
- [ ] All commits reviewed
- [ ] Code builds without new warnings
- [ ] Tests pass
- [ ] No formatting issues
- [ ] Architectural principles respected
- [ ] Security implications considered
- [ ] Performance implications considered
- [ ] Documentation updated (if needed)
- [ ] Tests added for new functionality
- [ ] Review feedback is constructive and specific

## Ready?

Start by fetching the PR and reading AGENTS.md → "For PR Reviews" section.

Remember: This validates our review workflow and documentation. Your feedback helps improve the process for everyone.

Good luck! 🎯
```

## Tips for the Human Supervisor

- PR reviews are subjective - guide the AI if it's too strict or too lenient
- Encourage looking at both code and architecture
- Remind to check tests (PRs without tests are a red flag)
- The container wrapper makes it easy to actually build and test PRs
- Real review is better than just reading code

## Success Criteria

- [ ] PR is thoroughly reviewed (code + architecture + tests)
- [ ] Review comments are constructive and specific
- [ ] Build and tests were actually run
- [ ] Feedback is documented
