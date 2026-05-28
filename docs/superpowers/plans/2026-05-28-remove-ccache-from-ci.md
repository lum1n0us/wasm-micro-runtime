# Remove ccache Caching from CI Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove unused ccache caching from CI to save GitHub Actions cache storage

**Architecture:** Single-file edit to remove 3 ccache cache steps and modify 3 package installation commands in the LLVM build workflow

**Tech Stack:** GitHub Actions YAML workflow

---

## File Structure

**Files to Modify:**
- `.github/workflows/build_llvm_libraries.yml` - Remove ccache caching and installation

**No New Files Created**

---

## Task 1: Create Feature Branch

**Files:**
- None (git operation)

- [ ] **Step 1: Create and switch to feature branch**

```bash
git checkout -b fix/remove-ccache-from-ci
```

Expected output:
```
Switched to a new branch 'fix/remove-ccache-from-ci'
```

- [ ] **Step 2: Verify branch creation**

```bash
git branch --show-current
```

Expected output:
```
fix/remove-ccache-from-ci
```

---

## Task 2: Remove Ubuntu ccache Caching

**Files:**
- Modify: `.github/workflows/build_llvm_libraries.yml:92-102`

- [ ] **Step 1: Read the current Ubuntu ccache section**

```bash
sed -n '92,102p' .github/workflows/build_llvm_libraries.yml
```

Expected to show:
```yaml
      - uses: actions/cache@v5
        with:
          path: ~/.cache/ccache
          key: 0-ccache-${{ inputs.os }}-${{ steps.get_last_commit.outputs.last_commit }}
          restore-keys: |
            0-ccache-${{ inputs.os }}
        if: steps.retrieve_llvm_libs.outputs.cache-hit != 'true' && inputs.os == 'ubuntu-22.04'

      # Don't install dependencies if the cache is hit or running in docker container
      - run: sudo apt install -y ccache ninja-build
        if: steps.retrieve_llvm_libs.outputs.cache-hit != 'true' && startsWith(inputs.os, 'ubuntu') && inputs.container_image == ''
```

- [ ] **Step 2: Remove Ubuntu ccache cache step (lines 92-98)**

Delete lines 92-98 (the entire `actions/cache@v5` block for Ubuntu ccache).

Use the Edit tool:

```yaml
      - uses: actions/cache@v5
        with:
          path: ~/.cache/ccache
          key: 0-ccache-${{ inputs.os }}-${{ steps.get_last_commit.outputs.last_commit }}
          restore-keys: |
            0-ccache-${{ inputs.os }}
        if: steps.retrieve_llvm_libs.outputs.cache-hit != 'true' && inputs.os == 'ubuntu-22.04'

      # Don't install dependencies if the cache is hit or running in docker container
```

Replace with:

```yaml
      # Don't install dependencies if the cache is hit or running in docker container
```

- [ ] **Step 3: Update Ubuntu installation command and add comment**

Find:
```yaml
      # Don't install dependencies if the cache is hit or running in docker container
      - run: sudo apt install -y ccache ninja-build
        if: steps.retrieve_llvm_libs.outputs.cache-hit != 'true' && startsWith(inputs.os, 'ubuntu') && inputs.container_image == ''
```

Replace with:
```yaml
      # Don't install dependencies if the cache is hit or running in docker container
      # Note: ccache is not used in CI to save cache storage.
      # Local developers can still use --use-ccache flag with build_llvm.py
      - run: sudo apt install -y ninja-build
        if: steps.retrieve_llvm_libs.outputs.cache-hit != 'true' && startsWith(inputs.os, 'ubuntu') && inputs.container_image == ''
```

- [ ] **Step 4: Verify Ubuntu section changes**

```bash
sed -n '92,105p' .github/workflows/build_llvm_libraries.yml
```

Expected to show the updated section without ccache cache step and with updated install command.

---

## Task 3: Remove macOS ccache Caching

**Files:**
- Modify: `.github/workflows/build_llvm_libraries.yml:104-113`

- [ ] **Step 1: Read the current macOS ccache section**

```bash
sed -n '104,113p' .github/workflows/build_llvm_libraries.yml
```

Expected to show macOS ccache cache step and brew install command.

- [ ] **Step 2: Remove macOS ccache cache step**

Find:
```yaml
      - uses: actions/cache@v5
        with:
          path: ~/Library/Caches/ccache
          key: 0-ccache-${{ inputs.os }}-${{ steps.get_last_commit.outputs.last_commit }}
          restore-keys: |
            0-ccache-${{ inputs.os }}
        if: steps.retrieve_llvm_libs.outputs.cache-hit != 'true' && startsWith(inputs.os, 'macos')

      - run: brew install ccache ninja
```

Replace with:
```yaml
      - run: brew install ninja
```

- [ ] **Step 3: Verify macOS section changes**

```bash
sed -n '104,108p' .github/workflows/build_llvm_libraries.yml
```

Expected to show only the brew install ninja command without ccache.

---

## Task 4: Remove Windows ccache Caching

**Files:**
- Modify: `.github/workflows/build_llvm_libraries.yml:115-125`

- [ ] **Step 1: Read the current Windows ccache section**

```bash
sed -n '115,125p' .github/workflows/build_llvm_libraries.yml
```

Expected to show Windows ccache cache step and choco install command.

- [ ] **Step 2: Remove Windows ccache cache step**

Find:
```yaml
      - uses: actions/cache@v5
        with:
          path: ~/.cache/ccache
          key: 0-ccache-${{ inputs.os }}-${{ steps.get_last_commit.outputs.last_commit }}
          restore-keys: |
            0-ccache-${{ inputs.os }}
        if: steps.retrieve_llvm_libs.outputs.cache-hit != 'true' && inputs.os == 'windows-2022'

      # Install tools on Windows
      - run: choco install -y ccache ninja
```

Replace with:
```yaml
      # Install tools on Windows
      - run: choco install -y ninja
```

- [ ] **Step 3: Verify Windows section changes**

```bash
sed -n '115,120p' .github/workflows/build_llvm_libraries.yml
```

Expected to show only the choco install ninja command without ccache.

---

## Task 5: Verify Changes and YAML Syntax

**Files:**
- Verify: `.github/workflows/build_llvm_libraries.yml`

- [ ] **Step 1: Check that ccache is completely removed**

```bash
grep -n "ccache" .github/workflows/build_llvm_libraries.yml
```

Expected output: Only the comment line mentioning ccache should appear:
```
[line number]:      # Note: ccache is not used in CI to save cache storage.
[line number]:      # Local developers can still use --use-ccache flag with build_llvm.py
```

- [ ] **Step 2: Verify LLVM libraries cache is untouched**

```bash
sed -n '80,90p' .github/workflows/build_llvm_libraries.yml
```

Expected to show the LLVM libraries cache block unchanged:
```yaml
      - name: Cache LLVM libraries
        id: retrieve_llvm_libs
        uses: actions/cache@v5
        with:
          path: |
            ./core/deps/llvm/build/bin
            ./core/deps/llvm/build/include
            ./core/deps/llvm/build/lib
            ./core/deps/llvm/build/libexec
            ./core/deps/llvm/build/share
          key: ${{ steps.create_lib_cache_key.outputs.key}}
```

- [ ] **Step 3: Validate YAML syntax**

```bash
python3 -c "import yaml; yaml.safe_load(open('.github/workflows/build_llvm_libraries.yml'))" && echo "YAML syntax valid"
```

Expected output:
```
YAML syntax valid
```

- [ ] **Step 4: Check git diff summary**

```bash
git diff --stat .github/workflows/build_llvm_libraries.yml
```

Expected to show modifications to the file (deletions and small additions).

- [ ] **Step 5: Review full diff**

```bash
git diff .github/workflows/build_llvm_libraries.yml
```

Verify:
- 3 ccache cache blocks removed (red lines with `actions/cache@v5`)
- `ccache` removed from 3 install commands
- Comment added in Ubuntu section
- All other content preserved

---

## Task 6: Commit Changes

**Files:**
- Commit: `.github/workflows/build_llvm_libraries.yml`

- [ ] **Step 1: Stage the modified file**

```bash
git add .github/workflows/build_llvm_libraries.yml
```

- [ ] **Step 2: Verify staged changes**

```bash
git status
```

Expected output:
```
On branch fix/remove-ccache-from-ci
Changes to be committed:
  (use "git restore --staged <file>..." to unstage)
        modified:   .github/workflows/build_llvm_libraries.yml
```

- [ ] **Step 3: Create commit**

```bash
git commit -m "$(cat <<'EOF'
ci: remove ccache caching to save storage

- Remove ccache cache steps for Ubuntu/macOS/Windows
- Remove ccache from dependency installation
- Keep ninja-build installation
- Add comment explaining ccache is not used in CI
- Local developers can still use --use-ccache flag

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
EOF
)"
```

Expected output:
```
[fix/remove-ccache-from-ci xxxxxxx] ci: remove ccache caching to save storage
 1 file changed, 3 insertions(+), 24 deletions(-)
```

- [ ] **Step 4: Verify commit**

```bash
git log -1 --stat
```

Expected to show the commit with the workflow file modification.

---

## Task 7: Push Branch and Verification Summary

**Files:**
- None (git operation)

- [ ] **Step 1: Push branch to remote**

```bash
git push -u origin fix/remove-ccache-from-ci
```

Expected output:
```
To [repository-url]
 * [new branch]      fix/remove-ccache-from-ci -> fix/remove-ccache-from-ci
Branch 'fix/remove-ccache-from-ci' set up to track remote branch 'fix/remove-ccache-from-ci' from 'origin'.
```

- [ ] **Step 2: Display branch URL for PR creation**

```bash
echo "Branch pushed successfully. Ready for PR creation."
git remote get-url origin
echo "Branch: fix/remove-ccache-from-ci"
```

- [ ] **Step 3: Summary of changes**

Display summary:
```
✅ Removed ccache caching from CI workflow
✅ Updated package installation commands
✅ Added explanatory comment
✅ Preserved LLVM libraries caching
✅ All changes in .github/workflows/build_llvm_libraries.yml
✅ Branch: fix/remove-ccache-from-ci

Next steps:
- Create PR with title: "ci: remove ccache caching to save storage"
- PR description should reference the design doc and explain the storage savings
```

---

## Completion Checklist

After all tasks:
- [ ] ccache cache steps removed from workflow (3 instances)
- [ ] ccache removed from installation commands (3 instances)
- [ ] Explanatory comment added
- [ ] LLVM libraries cache preserved
- [ ] YAML syntax validated
- [ ] Changes committed to fix/remove-ccache-from-ci branch
- [ ] Branch pushed to remote
- [ ] Ready for PR creation

## Success Criteria

1. No `actions/cache@v5` steps with ccache paths remain
2. Installation commands only install `ninja` (no ccache)
3. Comment explains why ccache is not used in CI
4. LLVM libraries cache block is unchanged
5. Workflow file passes YAML validation
6. Git history shows clean commit on feature branch
