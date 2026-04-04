# Claude Code Instructions for WAMR

**🚨 CRITICAL: Before doing ANYTHING in this project, read [AGENTS.md](./AGENTS.md) completely.**

AGENTS.md is your primary guide. It contains:
- Complete navigation for all task types (bug fixes, features, PR reviews, tests, refactoring)
- Documentation reading order and when to read what
- Container environment requirements and usage
- Project architecture and workflows

## Two Hard Rules

### 1. ALL commands MUST use the container wrapper

```bash
# ✅ CORRECT
./scripts/in-container.sh "cmake -B build"
./scripts/in-container.sh "ctest --test-dir build"

# ❌ WRONG - Missing toolchain
cmake -B build
```

Why: WAMR requires WASI-SDK, WABT, and other toolchains only available in the devcontainer.

### 2. Run pre-commit checks before committing

See [doc/linting.md](./doc/linting.md) for the mandatory checklist:
- Code format check
- Unit tests
- Spec tests  
- Regression tests

## What to Read Next

Based on your task, [AGENTS.md](./AGENTS.md) tells you exactly which docs to read and in what order.

**Do not skip AGENTS.md.** It will save you time and prevent mistakes.

---

**Ready?** → Open [AGENTS.md](./AGENTS.md) now.
