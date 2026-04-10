# Generic Devcontainer Command Executor

**Date:** 2026-04-10  
**Status:** Approved  
**Target:** Create a universal script for executing commands in VS Code devcontainers

## Overview

Refactor the WAMR-specific `scripts/in-container.sh` into a generic, reusable devcontainer command executor that works across any project using standard VS Code devcontainer configurations.

## Goals

1. **Universal compatibility** - Work with any project using VS Code devcontainer standard conventions
2. **Zero configuration** - Auto-detect settings from `devcontainer.json` with graceful fallbacks
3. **Easy deployment** - Maintain source in `~/bin/in-container/`, deploy via symlinks to project `.devcontainer/` directories
4. **Preserve quality** - Keep all battle-tested features from the original WAMR script

## Non-Goals

- Support for non-devcontainer Docker setups (docker-compose only, custom users, non-standard paths)
- Configuration file overrides (`.in-container.config`) - auto-detection only
- Multiple container runtime support (Podman, etc.) - Docker only
- Docker-compose fallback for starting containers - devcontainer CLI only

## Architecture

### Deployment Model

**Source location:** `~/bin/in-container/in-container.sh`

**Usage pattern:**
```bash
# One-time setup per project
cd /path/to/project/.devcontainer/
ln -s ~/bin/in-container/in-container.sh in-container.sh

# Execute commands
cd /path/to/project
./.devcontainer/in-container.sh "cmake -B build"
```

**Benefits:**
- Single source of truth for all projects
- Update once, all projects benefit via symlinks
- No file duplication across projects

### Configuration Auto-Detection

The script automatically extracts configuration from `devcontainer.json` with graceful degradation:

#### 1. Container Name Pattern
- **Source:** `name` field in devcontainer.json
- **Fallback:** Project folder name if `name` missing
- **Processing:** Convert to case-insensitive fuzzy match pattern
- **Example:** `"name": "MyProject-Dev"` → matches containers like `myproject-dev`, `MyProject-Dev-*`

#### 2. Remote User
- **Source:** `remoteUser` field in devcontainer.json
- **Fallback:** `vscode` (standard devcontainer default)
- **Usage:** User context for executing commands inside container

#### 3. Workspace Path
- **Always derived:** `/workspaces/<project-folder-name>`
- **Rationale:** Standard devcontainer convention, no need to parse
- **Example:** Project in `/home/user/myproject` → workspace is `/workspaces/myproject`

#### Parsing Strategy

**Tool priority:**
1. `jq` - If available, most robust JSON parsing
2. `python3 -c` - If jq unavailable, use python JSON module
3. `grep`/`sed` - Last resort, simple regex extraction

**Error handling:**
- If `devcontainer.json` missing or unparseable: Use all fallback values
- Print verbose warnings only in `--verbose` mode
- Never fail - always continue with inferred defaults

### Container Detection

**Two-stage detection:**

#### Stage 1: Cached Name
- Read `.devcontainer/.container-name`
- Verify container still exists in Docker
- If stale, delete cache and proceed to Stage 2

#### Stage 2: Name Pattern Matching
- List all Docker containers (`docker ps -a`)
- Match against the name pattern from devcontainer.json
- Case-insensitive fuzzy matching
- Save matched name to cache for future speed

**Removed:** Mount path inspection method (Stage 3 from original script) - not needed with simplified detection

### Container Lifecycle

**Starting containers:**
- **Only method:** `devcontainer up --workspace-folder <project-root>`
- **Requirements:** devcontainer CLI must be installed (`npm install -g @devcontainers/cli`)
- **No fallback:** If CLI unavailable, fail with clear installation instructions

**Starting stopped containers:**
- If container exists but stopped: `docker start <container-name>`
- Wait 2 seconds for startup
- Verify container is running before proceeding

### Command Execution

**Preserved features from original script:**
- TTY detection: Only attach TTY flags if stdin is a terminal
- Working directory: Always set to `/workspaces/<project-name>`
- User context: Execute as the parsed `remoteUser`
- Exit code propagation: Script exits with same code as containerized command
- Shell execution: Use `bash -c` to support pipes, redirects, etc.

**Execution pattern:**
```bash
docker exec \
  [--interactive --tty]  # Only if stdin is TTY
  --workdir /workspaces/<project-name> \
  --user <remote-user> \
  <container-name> \
  bash -c "<command>"
```

## Interface

### Command-Line Options

**Retained:**
- `-h, --help` - Show help message
- `--version` - Display version and feature info
- `--status` - Show container status without running commands
- `--verbose` - Enable detailed diagnostic output (shows parsed config)
- `--quiet` - Suppress informational messages
- `--container-name <name>` - Manually specify container name (bypass detection)

**Removed:**
- `--no-start` - Deleted; script always auto-starts containers when needed

### Help Message Updates

- Remove WAMR-specific terminology and examples
- Add generic devcontainer description
- Include symlink deployment instructions
- Update examples to use common development commands (not WAMR-specific)

### Verbose Output

When `--verbose` is enabled, display:
```
[VERBOSE] Script location: .devcontainer/in-container.sh
[VERBOSE] Project root: /home/user/myproject
[VERBOSE] Project name: myproject
[VERBOSE] Parsed config:
[VERBOSE]   Container name pattern: myproject.*dev
[VERBOSE]   Workspace path: /workspaces/myproject
[VERBOSE]   Remote user: vscode
[VERBOSE] devcontainer.json: found and parsed successfully
```

## Error Handling

### Script Location Validation

**Check:** Script must reside in a `.devcontainer/` directory

**Error message:**
```
ERROR: This script must be placed in or symlinked to .devcontainer/ directory

Current location: /home/user/bin/in-container.sh

Setup instructions:
  1. cd /path/to/your/project/.devcontainer/
  2. ln -s ~/bin/in-container/in-container.sh in-container.sh
  3. cd ..
  4. ./.devcontainer/in-container.sh "your-command"
```

### devcontainer.json Parsing Failure

**Behavior:** Warn (verbose only) and use fallbacks
**No exit:** Continue with inferred defaults

### devcontainer CLI Missing

**Error message:**
```
ERROR: devcontainer CLI not found

The devcontainer CLI is required to start containers.

Install: npm install -g @devcontainers/cli

Alternatively, start your container manually:
  - Open project in VS Code
  - Click "Reopen in Container"
  - Then run this script again
```

### Container Detection Failure

**After detection fails:**
- Attempt to start container via devcontainer CLI
- If start fails, show clear error with troubleshooting steps

### Command Execution Failure

**Behavior:** Propagate exit code, no additional error messages
**Rationale:** Let the original command's output speak for itself

### Edge Cases

- **Empty command:** Show help and exit with code 1
- **Special characters in container name:** Properly escape for pattern matching
- **Spaces in project folder name:** Correctly quote paths
- **Container exists but stopped:** Auto-start and verify running state

## Implementation Details

### Core Functions

#### New Functions

**`check_script_location()`**
- Validate script is in `.devcontainer/` directory
- Use `${BASH_SOURCE[0]}` to determine actual script path (follows symlinks)
- Exit with error and instructions if not in correct location

**`get_project_root()`**
- Return absolute path to project root (parent of `.devcontainer/`)
- Use `dirname` on the script's directory path

**`get_project_name()`**
- Extract folder name from project root path
- Use `basename` on project root

**`parse_devcontainer_json()`**
- Read `<project-root>/.devcontainer/devcontainer.json`
- Extract `name` and `remoteUser` fields
- Set global variables: `CONTAINER_NAME_PATTERN`, `CONTAINER_USER`, `WORKSPACE_PATH`
- Tool selection logic: try jq → python → grep/sed
- Handle missing file and parsing errors gracefully

#### Modified Functions

**`detect_container()`**
- Use dynamic `CONTAINER_NAME_PATTERN` instead of hardcoded "wamr.*dev"
- Update cache file location to use project root variable
- Remove Stage 3 (mount path inspection)

**`start_container()`**
- Remove docker-compose fallback logic
- Only use: `devcontainer up --workspace-folder "${PROJECT_ROOT}"`
- Better error message for missing CLI

**`ensure_container()`**
- Remove `OPT_NO_START` related logic
- Simplify flow: always start if not running

**`exec_in_container()`**
- Use dynamic `WORKSPACE_PATH` and `CONTAINER_USER` variables
- Keep all TTY and exit code handling unchanged

**`show_help()`**
- Replace WAMR-specific descriptions with generic text
- Update examples to generic commands
- Add deployment/symlink instructions

**`show_status()`**
- Use dynamic workspace path in mount display
- Keep all other status reporting unchanged

**`main()`**
- Call `check_script_location()` at startup
- Call `parse_devcontainer_json()` before option parsing
- Remove `--no-start` option parsing
- Keep all other option handling

### Execution Flow

```
main() starts
  ↓
check_script_location()
  - Verify script in .devcontainer/
  - Exit if not
  ↓
get_project_root()
  - Determine project root path
  ↓
get_project_name()
  - Extract project folder name
  ↓
parse_devcontainer_json()
  - Read and parse config
  - Set CONTAINER_NAME_PATTERN
  - Set CONTAINER_USER
  - Set WORKSPACE_PATH
  ↓
Parse command-line options
  - Handle --help, --status, --verbose, etc.
  ↓
Validate command argument provided
  ↓
detect_container()
  - Check cache → name pattern match
  ↓
Container found?
  ├─ No → start_container()
  │         - Use devcontainer CLI
  │         - Re-run detect_container()
  ↓
ensure_container()
  - Start if stopped
  ↓
exec_in_container()
  - Execute command with proper context
  - Propagate exit code
```

## Testing Considerations

### Test Scenarios

1. **Different devcontainer.json formats**
   - With all fields present
   - Missing `name` field
   - Missing `remoteUser` field
   - Missing both fields
   - Malformed JSON

2. **Container states**
   - No container exists (fresh start)
   - Container exists and running
   - Container exists but stopped
   - Multiple matching containers (use first)

3. **Error conditions**
   - devcontainer CLI not installed
   - Script not in .devcontainer/ directory
   - devcontainer.json missing entirely
   - Docker daemon not running

4. **Edge cases**
   - Project folder names with spaces
   - Project folder names with special characters
   - Symlink vs direct file placement
   - Running from subdirectories (should fail with clear message)

## Version and Maintenance

**Version:** `2.0.0`
- Major version bump to indicate breaking change (generalization)
- Original WAMR-specific version was implied 1.0.0

**Compatibility requirements:**
- VS Code devcontainer specification compliance
- devcontainer CLI installed (`@devcontainers/cli`)
- Docker daemon running
- Bash 4.0+ (for associative arrays if needed)

**Documentation:**
- Create `~/bin/in-container/README.md` with:
  - Setup instructions (symlink creation)
  - Deployment to new projects
  - Troubleshooting guide
  - FAQ section

## Migration from WAMR Script

**For WAMR project:**
- Keep original `scripts/in-container.sh` unchanged (as requested)
- Can optionally test generic version in parallel if desired

**For other projects:**
- No migration needed - fresh deployment via symlink
- Works out of the box with standard devcontainer setups

## Success Criteria

1. ✅ Works with any standard VS Code devcontainer project
2. ✅ Zero manual configuration required
3. ✅ All original WAMR script features preserved
4. ✅ Clear error messages for all failure modes
5. ✅ Easy deployment via symlinks
6. ✅ Single source maintained in `~/bin/in-container/`
