# in-container.sh Test Results

## Test 1: No Container
**Scenario:** No WAMR container exists
- ✅ Script detects no container
- ✅ Script starts new container using devcontainer CLI
- ✅ Command executes successfully
- ✅ Correct workspace path: `/workspaces/ai-thoughts`
- ✅ Correct user: `vscode`

**Output:**
```
[INFO] Starting devcontainer...
[INFO] Using devcontainer CLI...
[devcontainer CLI output...]
[SUCCESS] Using container: optimistic_mcnulty
[INFO] Executing in container 'optimistic_mcnulty': echo 'Test: Container auto-started' && pwd && whoami
Test: Container auto-started
/workspaces/ai-thoughts
vscode
```

## Test 2: Stopped Container
**Scenario:** WAMR container exists but is stopped
- ✅ Script detects stopped container
- ✅ Script restarts container with `docker start`
- ✅ Verification ensures container is running before executing command
- ✅ Command executes successfully

**Output:**
```
[INFO] Container 'optimistic_mcnulty' exists but not running. Starting...
[SUCCESS] Using container: optimistic_mcnulty
[INFO] Executing in container 'optimistic_mcnulty': echo 'Container restarted' && uptime | head -1
Container restarted
 13:23:14 up 3 days,  4:21,  0 user,  load average: 1.09, 0.77, 0.39
```

## Test 3: Running Container
**Scenario:** WAMR container is already running
- ✅ Script detects running container immediately
- ✅ No unnecessary restart or startup messages
- ✅ Command executes immediately
- ✅ Minimal overhead

**Output:**
```
[SUCCESS] Using container: optimistic_mcnulty
[INFO] Executing in container 'optimistic_mcnulty': echo 'Using running container' && hostname
Using running container
74131c04e0a9
```

## Additional Fixes Applied During Testing

### Issue 1: TTY Detection
**Problem:** Script failed with "input device is not a TTY" error when run in non-interactive contexts
**Solution:** Added TTY detection using `[ -t 0 ]` to conditionally apply `--interactive --tty` flags
**Commit:** 1bb99b1e0

### Issue 2: Output Pollution
**Problem:** Helper function outputs (info, success) interfered with function return values captured via command substitution
**Solution:** Redirected all helper outputs to stderr, redirected devcontainer/docker-compose outputs to stderr
**Commit:** 74f5898a6

## Summary

All tests passed on 2026-04-03. The script correctly handles:
- ✅ Auto-starting containers when none exist
- ✅ Restarting stopped containers
- ✅ Using running containers efficiently
- ✅ Non-interactive execution (CI/CD compatible)
- ✅ Proper output handling without pollution

## Test Environment

- Platform: Linux 5.15.0-173-generic
- Docker: Available
- devcontainer CLI: v0.80.3 (Node.js v22.20.0)
- Project: wasm-micro-runtime/.worktree/ai-thoughts
- Container base: mcr.microsoft.com/devcontainers/cpp:debian-12
