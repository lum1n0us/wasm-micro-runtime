# Prompt Template: Fix a Real Bug in WAMR

## Initial Prompt for New Session

```markdown
You are helping fix a bug in the WAMR (WebAssembly Micro Runtime) project.

## ⚠️ CRITICAL: Development Environment

This project uses devcontainer for ALL development activities.
ALL commands MUST use the wrapper script:

./scripts/in-container.sh "<command>"

Never run cmake, make, gcc, gdb, etc. directly on the host!

## Your Mission

Fix this bug: [PASTE ISSUE LINK OR DESCRIPTION HERE]

## Getting Started

1. **First, read the navigation guide:**
   - Read `AGENTS.md` completely
   - Follow "For Bug Fixes" section

2. **Understand the codebase:**
   - Read `doc/architecture-overview.md` to understand component relationships
   - Identify which component the bug is in (VMcore, iwasm, wamrc)

3. **Set up your environment:**
   - Read `doc/dev-in-container.md` to understand the container
   - Check container status: `./scripts/in-container.sh --status`

## Step-by-Step Workflow

### Phase 1: Understand the Bug
- Read the issue description carefully
- Identify reproduction steps
- Determine expected vs actual behavior
- Locate relevant code sections

### Phase 2: Build with Debug Symbols
Follow `doc/building.md`:

```bash
# Debug build
./scripts/in-container.sh "cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DWAMR_BUILD_INTERP=1 -DWAMR_BUILD_FAST_INTERP=1 -DWAMR_BUILD_DUMP_CALL_STACK=1 -DWAMR_BUILD_LIBC_WASI=1"

# Build
./scripts/in-container.sh "cmake --build build-debug -j\$(nproc)"
```

### Phase 3: Reproduce and Debug
Follow `doc/debugging.md`:

```bash
# If you have a test case
./scripts/in-container.sh "gdb --args ./build-debug/product-mini/platforms/linux/iwasm test.wasm"

# Set breakpoints, examine variables, trace execution
# Document your findings
```

### Phase 4: Implement Fix
- Make minimal changes to fix the bug
- Follow coding conventions from the codebase
- Add comments explaining the fix

### Phase 5: Test the Fix
Follow `doc/testing.md`:

```bash
# Run existing tests
./scripts/in-container.sh "cd build-debug && ctest --output-on-failure"

# Add a regression test if appropriate
# Test with your reproduction case
```

### Phase 6: Code Quality Check
Follow `doc/code-quality.md`:

```bash
# Format your changes
./scripts/in-container.sh "clang-format-14 -i <your-modified-files>"

# Check for warnings
./scripts/in-container.sh "cmake --build build-debug 2>&1 | grep -i warning"
```

### Phase 7: Prepare for Review
- Write clear commit message explaining the fix
- Document what was wrong and how you fixed it
- Note any testing you did

## Container Wrapper Features to Use

```bash
# Check what container is running
./scripts/in-container.sh --status

# Get version info
./scripts/in-container.sh --version

# Verbose mode for debugging wrapper issues
./scripts/in-container.sh --verbose "pwd"

# Quiet mode for clean output
./scripts/in-container.sh --quiet "git status" > status.txt

# Get help anytime
./scripts/in-container.sh --help
```

## 📝 IMPORTANT: Track Your Feedback

As you work, please document:

### What Worked Well
- Which documentation was most helpful?
- Which container wrapper features did you use?
- What made the workflow smooth?

### What Could Be Better
- What documentation was missing or unclear?
- What questions couldn't you answer from the docs?
- What container wrapper issues did you encounter?
- What would make this workflow easier?

### Specific Issues
- Any errors with the wrapper script?
- Any missing tools in the container?
- Any gaps in the documentation?

**Save feedback to:** `.superpowers/feedback/task-1-bugfix-feedback.md`

Or bring it back to the "Make WAMR be friendly to AI" session for discussion.

## Ready?

Let's fix this bug! Start by reading AGENTS.md and understanding the issue.

Remember: This is a real-world validation of our AI-friendly infrastructure. Your feedback will help improve the system for all future developers (human and AI).

Good luck! 🚀
```

## Tips for the Human Supervisor

- Let the AI agent work autonomously but check in periodically
- If the agent seems stuck, ask "What documentation would help right now?"
- Encourage the agent to read docs rather than guess
- Remind to use the container wrapper if it tries direct commands
- The goal is to validate the infrastructure, not just fix the bug

## Success Criteria

- [ ] Bug is fixed
- [ ] Fix is tested
- [ ] Code is formatted
- [ ] Commit is ready
- [ ] Feedback is documented
