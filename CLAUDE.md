# Claude Code Instructions for WAMR

**🚨 CRITICAL: Before doing ANYTHING in this project, read [AGENTS.md](./AGENTS.md) completely.**

AGENTS.md is your primary guide. It contains:
- Complete navigation for all task types (bug fixes, features, PR reviews, tests, refactoring)
- Documentation reading order and when to read what
- Container environment requirements and usage
- Project architecture and workflows

## Critical Rule: Linux Development Must Use Devcontainer

**On Linux systems ONLY**, all build, test, debug, and development activities MUST be performed inside the devcontainer:

```bash
# ✅ CORRECT - On Linux
devcontainer exec --workspace-folder . -- cmake -B build
devcontainer exec --workspace-folder . -- ctest --test-dir build

# ❌ WRONG - On Linux, missing required toolchain
cmake -B build
```

Why: WAMR requires WASI-SDK, WABT, and other specialized toolchains only available in the devcontainer.

**Note**: macOS and Windows developers may have different workflows. See [AGENTS.md](./AGENTS.md) for platform-specific guidance.

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
