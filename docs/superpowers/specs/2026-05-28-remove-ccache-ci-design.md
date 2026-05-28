# Design: Remove ccache Caching from CI

**Date:** 2026-05-28  
**Status:** Approved  
**Author:** Claude Code

## Overview

Remove ccache caching mechanism from LLVM build CI workflows to save GitHub Actions cache storage. The ccache functionality is not being used in CI builds (no `--use-ccache` flag is passed), making the caching steps wasteful.

## Goals

1. Remove unused ccache caching from CI to save cache storage
2. Preserve LLVM libraries caching (the primary cache that saves 2-4 hours)
3. Keep ccache support available for local development via `build_llvm.py --use-ccache`
4. Maintain all existing LLVM build functionality

## Background

### Current State

The `build_llvm_libraries.yml` workflow currently:
- Caches LLVM libraries (essential, saves 2-4 hours)
- Caches ccache directories for 3 platforms (Ubuntu/macOS/Windows)
- Installs ccache on all 3 platforms
- **Does NOT** pass `--use-ccache` flag to `build_llvm.py`

### Problem

The ccache caching steps consume GitHub Actions cache storage but provide no benefit because:
1. The `build_llvm.py` script only uses ccache when `--use-ccache` flag is passed
2. CI workflows never pass this flag
3. The cached ccache directories remain empty or unused

### Why Not Remove Completely?

We keep the `--use-ccache` support in `build_llvm.py` because:
- Local developers can use it for faster incremental builds
- The flag and logic cause no harm in the codebase
- Removing it would require more invasive changes to the build script

## Scope

### In Scope

- **File to modify:** `.github/workflows/build_llvm_libraries.yml`
- Remove 3 ccache cache steps (Ubuntu/macOS/Windows)
- Remove ccache from package installation commands
- Add explanatory comment
- Keep ninja-build installation

### Out of Scope

- LLVM libraries caching (must remain unchanged)
- `build_llvm.py` script modifications
- Other workflow files
- Removal of ccache support from build script

## Design

### Detailed Changes

All changes are in `.github/workflows/build_llvm_libraries.yml`.

#### 1. Ubuntu Platform (lines 92-102)

**Remove entirely:**
```yaml
- uses: actions/cache@v5
  with:
    path: ~/.cache/ccache
    key: 0-ccache-${{ inputs.os }}-${{ steps.get_last_commit.outputs.last_commit }}
    restore-keys: |
      0-ccache-${{ inputs.os }}
  if: steps.retrieve_llvm_libs.outputs.cache-hit != 'true' && inputs.os == 'ubuntu-22.04'
```

**Modify installation step:**

Before:
```yaml
# Don't install dependencies if the cache is hit or running in docker container
- run: sudo apt install -y ccache ninja-build
  if: steps.retrieve_llvm_libs.outputs.cache-hit != 'true' && startsWith(inputs.os, 'ubuntu') && inputs.container_image == ''
```

After:
```yaml
# Don't install dependencies if the cache is hit or running in docker container
# Note: ccache is not used in CI to save cache storage.
# Local developers can still use --use-ccache flag with build_llvm.py
- run: sudo apt install -y ninja-build
  if: steps.retrieve_llvm_libs.outputs.cache-hit != 'true' && startsWith(inputs.os, 'ubuntu') && inputs.container_image == ''
```

#### 2. macOS Platform (lines 104-113)

**Remove entirely:**
```yaml
- uses: actions/cache@v5
  with:
    path: ~/Library/Caches/ccache
    key: 0-ccache-${{ inputs.os }}-${{ steps.get_last_commit.outputs.last_commit }}
    restore-keys: |
      0-ccache-${{ inputs.os }}
  if: steps.retrieve_llvm_libs.outputs.cache-hit != 'true' && startsWith(inputs.os, 'macos')
```

**Modify installation step:**

Before:
```yaml
- run: brew install ccache ninja
  if: steps.retrieve_llvm_libs.outputs.cache-hit != 'true' && startsWith(inputs.os, 'macos')
```

After:
```yaml
- run: brew install ninja
  if: steps.retrieve_llvm_libs.outputs.cache-hit != 'true' && startsWith(inputs.os, 'macos')
```

#### 3. Windows Platform (lines 115-125)

**Remove entirely:**
```yaml
- uses: actions/cache@v5
  with:
    path: ~/.cache/ccache
    key: 0-ccache-${{ inputs.os }}-${{ steps.get_last_commit.outputs.last_commit }}
    restore-keys: |
      0-ccache-${{ inputs.os }}
  if: steps.retrieve_llvm_libs.outputs.cache-hit != 'true' && inputs.os == 'windows-2022'
```

**Modify installation step:**

Before:
```yaml
# Install tools on Windows
- run: choco install -y ccache ninja
  if: steps.retrieve_llvm_libs.outputs.cache-hit != 'true' && inputs.os == 'windows-2022'
```

After:
```yaml
# Install tools on Windows
- run: choco install -y ninja
  if: steps.retrieve_llvm_libs.outputs.cache-hit != 'true' && inputs.os == 'windows-2022'
```

### Comment Strategy

Add explanatory comment only once (in Ubuntu section) to:
- Avoid redundant comments
- Place it where readers will see it first
- Keep maintenance burden low

The comment explains:
1. Why ccache is removed from CI
2. How local developers can still use it

### Summary of Changes

**Deletions:**
- 3 `actions/cache@v5` steps (21 lines total)
- `ccache` from 3 package installation commands

**Additions:**
- 2-line explanatory comment

**Preserved:**
- All LLVM libraries caching logic
- All conditional logic (`if` statements)
- Ninja-build installation
- Build step and all other workflow logic

## Implementation Plan

### Step 1: Create Branch
```bash
git checkout -b fix/remove-ccache-from-ci
```

### Step 2: Apply Changes
Edit `.github/workflows/build_llvm_libraries.yml`:
- Delete lines 92-98 (Ubuntu ccache cache)
- Modify line 101 (Ubuntu install: remove ccache, add comment)
- Delete lines 104-110 (macOS ccache cache)
- Modify line 112 (macOS install: remove ccache)
- Delete lines 115-121 (Windows ccache cache)
- Modify line 124 (Windows install: remove ccache)

### Step 3: Commit
```bash
git add .github/workflows/build_llvm_libraries.yml
git commit -m "ci: remove ccache caching to save storage

- Remove ccache cache steps for Ubuntu/macOS/Windows
- Remove ccache from dependency installation
- Keep ninja-build installation
- Add comment explaining ccache is not used in CI
- Local developers can still use --use-ccache flag"
```

### Step 4: Push and Create PR
```bash
git push -u origin fix/remove-ccache-from-ci
gh pr create --title "ci: remove ccache caching to save storage" --body "..."
```

## Validation

### Static Checks
- ✅ YAML syntax is valid
- ✅ Indentation and formatting consistent
- ✅ All `if` conditions preserved
- ✅ LLVM libraries cache untouched

### Runtime Checks
- ✅ Workflow triggers successfully
- ✅ LLVM builds complete successfully
- ✅ LLVM libraries cache still works
- ✅ No ccache cache entries created

### Expected Behavior

**No change in:**
- Build time (ccache wasn't being used)
- Build success rate
- LLVM libraries caching
- Dependent workflows

**Change in:**
- GitHub Actions cache storage (reduced)
- No more `0-ccache-*` cache entries

## Risk Assessment

**Risk Level:** Low

### Why Low Risk?

1. **Small scope:** Single file, deleting unused functionality
2. **No functional dependency:** ccache wasn't being used
3. **Easy rollback:** Simple `git revert` if needed
4. **Strong isolation:** Doesn't affect LLVM libraries cache or other workflows

### Potential Issues

None identified. The ccache functionality was already disabled (no `--use-ccache` flag).

## Success Criteria

1. ✅ No ccache cache steps in `build_llvm_libraries.yml`
2. ✅ Installation commands only include ninja, not ccache
3. ✅ LLVM build pipeline works normally
4. ✅ LLVM libraries cache works normally
5. ✅ GitHub Actions cache storage reduced (no ccache entries)
6. ✅ Explanatory comment present

## Future Considerations

### If ccache Needed in Future

To re-enable ccache in CI:
1. Add back the cache steps
2. Add `--use-ccache` flag to build_llvm.py invocation
3. Update the comment to explain why it's enabled

### Local Development

Developers can always use:
```bash
python3 build_llvm.py --use-ccache --arch x86_64
```

This continues to work as before.

## References

- Related analysis: `docs/superpowers/ci-cache-usage-analysis.md`
- Build script: `build-scripts/build_llvm.py` (lines 51, 71-72, 274-276)
- Workflow file: `.github/workflows/build_llvm_libraries.yml`
