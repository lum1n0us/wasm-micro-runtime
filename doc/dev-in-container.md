# Development in Devcontainer

## Overview

The WAMR project uses Docker-based development containers (devcontainers) to provide a consistent, reproducible build environment across all development machines. This eliminates "works on my machine" problems and ensures all developers and CI systems use identical toolchain versions.

### Why Use Devcontainers?

- **Consistent Toolchains**: Everyone uses the same versions of compilers, build tools, and libraries
- **Zero Setup**: New contributors can start building immediately without manual dependency installation
- **Isolated Environment**: Project dependencies don't contaminate your host system
- **CI/CD Parity**: Local builds match CI builds exactly

### Pre-installed Tools

The WAMR devcontainer includes all necessary development tools:

- **Build Tools**: CMake, Ninja, Make, ccache
- **Compilers**: GCC (with multilib), Clang, LLVM
- **WebAssembly Tools**: 
  - WASI SDK 25.0 (at `/opt/wasi-sdk`)
  - WABT 1.0.37 (WebAssembly Binary Toolkit at `/opt/wabt`)
- **Debugging**: GDB, Valgrind
- **Code Quality**: clang-format-14
- **Languages**: Python 3 (with pip and venv), OCaml (with opam)
- **Utilities**: Git, Vim, Wget, Curl, Zip/Unzip

### Configuration Files

The devcontainer configuration lives in `.devcontainer/`:

- `devcontainer.json` - VS Code devcontainer configuration
- `Dockerfile` - Container image definition with all tools
- `finalize.sh` - Post-creation setup script
- `.container-name` - Cached container name (auto-generated)

---

## For AI Agents

**CRITICAL REQUIREMENT**: All build, test, debug, and code quality commands MUST run inside the devcontainer. Never run build commands directly on the host system.

### Using the Wrapper Script

The `scripts/in-container.sh` wrapper script is your primary interface to the devcontainer. It handles all container lifecycle management automatically.

**Basic Pattern:**
```bash
scripts/in-container.sh "<command>"
```

The script will:
1. Detect if a devcontainer exists (3 detection methods)
2. Start the container if not running
3. Execute your command inside the container
4. Return the command's exit code

### Automatic Detection and Startup

The script automatically detects existing containers using three methods:

1. **Cached Name**: Reads `.devcontainer/.container-name` if it exists
2. **Name Pattern**: Finds containers matching `wamr.*dev` (case-insensitive)
3. **Mount Inspection**: Finds containers with `/workspaces/ai-thoughts` mounted

If no container is found, it will start one using:
- `devcontainer up` command (if devcontainer CLI is installed)
- `docker-compose` (fallback method)
- Reports an error if neither method is available

### Available Options

```bash
scripts/in-container.sh [OPTIONS] <command>

--help              Show comprehensive help message
--version           Display script version and detection methods
--status            Show container status without running commands
--verbose           Enable detailed logging for debugging
--quiet             Suppress info messages, show only output and errors
--no-start          Fail if container not running (for CI)
--container-name    Manually specify container name (skip detection)
```

### AI Agent Examples

**Building the project:**
```bash
# Configure and build
scripts/in-container.sh "cmake -B build -DWAMR_BUILD_INTERP=1 && cmake --build build -j$(nproc)"

# Clean rebuild
scripts/in-container.sh "rm -rf build && cmake -B build && cmake --build build"
```

**Running tests:**
```bash
# Run all tests
scripts/in-container.sh "cd build && ctest --output-on-failure"

# Run specific test
scripts/in-container.sh "cd build && ctest -R test_wasm_runtime --verbose"

# Run Python test suite
scripts/in-container.sh "python3 tests/wamr-test-suites/test_wamr.py"
```

**Code quality checks:**
```bash
# Format check (dry run)
scripts/in-container.sh "clang-format-14 --dry-run --Werror core/iwasm/**/*.c"

# Format fix
scripts/in-container.sh "clang-format-14 -i core/iwasm/**/*.c"

# Check formatting of changed files
scripts/in-container.sh "git diff --name-only | grep '\.c$' | xargs clang-format-14 --dry-run --Werror"
```

**Debugging:**
```bash
# Run under GDB
scripts/in-container.sh "gdb --args ./build/iwasm test.wasm"

# Run under Valgrind
scripts/in-container.sh "valgrind --leak-check=full ./build/iwasm test.wasm"
```

**Inspecting the environment:**
```bash
# Check installed tools
scripts/in-container.sh "which cmake gcc clang wasm-opt wasm2wat"

# Verify versions
scripts/in-container.sh "cmake --version && gcc --version && clang --version"

# Check workspace
scripts/in-container.sh "pwd && ls -la"
```

**CI/CD usage:**
```bash
# Ensure container is pre-started (fail fast if not)
scripts/in-container.sh --no-start "cmake --build build"

# Quiet mode for clean logs
scripts/in-container.sh --quiet "ctest --output-on-failure" > test-results.txt
```

### Error Handling

The script properly propagates exit codes, so you can chain commands and detect failures:

```bash
# This works correctly
if scripts/in-container.sh "make test"; then
    echo "Tests passed"
else
    echo "Tests failed"
    exit 1
fi

# Chain with && and ||
scripts/in-container.sh "cmake -B build" && \
scripts/in-container.sh "cmake --build build" && \
scripts/in-container.sh "cd build && ctest"
```

### Container Status Monitoring

```bash
# Check if container is running
scripts/in-container.sh --status

# Debug detection issues
scripts/in-container.sh --verbose --status

# Example output:
# Container: wamr-dev_devcontainer-1
# Status: Running ✓
# Details:
#   Image: wamr-dev:latest
#   Created: 2026-04-03T10:23:45
#   Workspace: /home/user/wamr -> /workspaces/ai-thoughts
```

---

## For Human Developers

### VS Code Integration

The recommended way for human developers to use the devcontainer is through VS Code's built-in support.

**First-time setup:**

1. **Install Prerequisites**:
   - [Docker Desktop](https://docs.docker.com/get-docker/) or Docker Engine
   - [Visual Studio Code](https://code.visualstudio.com/)
   - [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)

2. **Open in Container**:
   - Open the WAMR project folder in VS Code
   - VS Code will detect `.devcontainer/devcontainer.json`
   - Click "Reopen in Container" when prompted
   - Or: Press `F1` → "Dev Containers: Reopen in Container"

3. **Wait for Build**:
   - First time takes 5-10 minutes to build the image
   - Subsequent opens are instant (image is cached)
   - Progress shown in VS Code terminal

4. **Start Developing**:
   - Terminal is automatically inside the container
   - Extensions are automatically installed
   - Ready to build, test, and debug

**VS Code Extensions Included:**
- `dtsvet.vscode-wasm` - WebAssembly support
- `ms-python.python` - Python language support
- `ms-python.vscode-pylance` - Python IntelliSense
- `ms-vscode.cmake-tools` - CMake integration

### Manual Container Management

If you prefer command-line management or don't use VS Code:

**Using devcontainer CLI:**
```bash
# Install CLI (requires Node.js)
npm install -g @devcontainers/cli

# Start container
devcontainer up --workspace-folder .

# Execute command
devcontainer exec --workspace-folder . bash -c "make build"

# Stop container
docker stop <container-name>
```

**Using docker-compose (if configured):**
```bash
# Start container
docker-compose -f .devcontainer/docker-compose.yml up -d

# Enter container
docker exec -it <container-name> bash

# Stop container
docker-compose -f .devcontainer/docker-compose.yml down
```

**Direct Docker commands:**
```bash
# List WAMR containers
docker ps -a | grep wamr

# Start stopped container
docker start <container-name>

# Get interactive shell
docker exec -it <container-name> bash

# Stop container
docker stop <container-name>

# Remove container (will be recreated next time)
docker rm <container-name>
```

### Interactive Development Workflow

Once inside the container (via VS Code or `docker exec`):

```bash
# Configure build
cmake -B build -DWAMR_BUILD_INTERP=1 -DWAMR_BUILD_AOT=1

# Build
cmake --build build -j$(nproc)

# Test
cd build && ctest --output-on-failure

# Install locally (inside container)
cmake --install build --prefix /tmp/wamr-install
```

---

## Workspace Layout

### Path Mapping

| Location | Host Path | Container Path |
|----------|-----------|----------------|
| Project Root | `$PWD` | `/workspaces/ai-thoughts` |
| Build Output | `$PWD/build` | `/workspaces/ai-thoughts/build` |
| Config | `$PWD/.devcontainer` | `/workspaces/ai-thoughts/.devcontainer` |
| WASI SDK | N/A | `/opt/wasi-sdk` |
| WABT Tools | N/A | `/opt/wabt` |

### Volume Mounts

The devcontainer mounts your project directory as a volume:
- **Host**: Your local clone of the WAMR repository
- **Container**: `/workspaces/ai-thoughts` 
- **Sync**: Bidirectional - changes on host appear in container and vice versa

This means:
- Editing files on host is immediately visible in container
- Build outputs created in container appear on host
- Git operations work from either host or container
- No data loss when container is removed

### Working Directory

Commands executed via `scripts/in-container.sh` automatically run in `/workspaces/ai-thoughts` (the project root inside the container).

### User Permissions

- **Container User**: `vscode` (non-root)
- **User ID**: Mapped to match host user (prevents permission issues)
- **File Ownership**: Files created in container are owned by your host user

---

## Troubleshooting

### Container Won't Start

**Symptom**: `scripts/in-container.sh` fails with "Cannot start container"

**Solutions**:
```bash
# 1. Check Docker is running
docker ps

# 2. Check for conflicting containers
docker ps -a | grep wamr
docker rm -f <old-container-name>

# 3. Install devcontainer CLI
npm install -g @devcontainers/cli

# 4. Try manual start via VS Code
# Open project in VS Code → F1 → "Reopen in Container"

# 5. Check Docker daemon
systemctl status docker  # Linux
# or check Docker Desktop status on Mac/Windows

# 6. Clean up and retry
docker system prune -a
# Then restart from VS Code
```

### Permission Errors

**Symptom**: "Permission denied" when building or writing files

**Solutions**:
```bash
# 1. Check file ownership on host
ls -la build/

# 2. Fix ownership (run on host)
sudo chown -R $USER:$USER build/

# 3. Ensure container user matches host
# This should be automatic but verify:
docker exec <container> id
# Should show uid matching your host uid

# 4. For persistent issues, rebuild container
# In VS Code: F1 → "Dev Containers: Rebuild Container"
```

### Container Is Slow

**Symptom**: Builds are slower in container than on host

**Solutions**:
1. **Enable Docker BuildKit** (faster builds):
   ```bash
   export DOCKER_BUILDKIT=1
   ```

2. **Use ccache** (already installed):
   ```bash
   scripts/in-container.sh "cmake -B build -DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
   ```

3. **Adjust Docker resources**:
   - Docker Desktop → Settings → Resources
   - Increase CPU cores and RAM allocation
   - Recommended: 4+ CPUs, 8+ GB RAM

4. **Use named volumes for build cache**:
   - Modify `.devcontainer/devcontainer.json`
   - Add volume mounts for `build/` directory

### "Container Not Found" After System Restart

**Symptom**: Script can't find container after reboot

**Solutions**:
```bash
# 1. Remove stale cache
rm -f .devcontainer/.container-name

# 2. Let script auto-detect or start new container
scripts/in-container.sh "pwd"

# 3. Or manually specify container name
docker ps -a | grep wamr
scripts/in-container.sh --container-name <name> "pwd"
```

### Changes Not Appearing

**Symptom**: File changes on host don't appear in container

**Solutions**:
```bash
# 1. Verify mount is working
scripts/in-container.sh "ls -la /workspaces/ai-thoughts"

# 2. Check Docker volume mounts
docker inspect <container-name> | grep Mounts -A 20

# 3. Restart container
docker restart <container-name>

# 4. If using Docker Desktop on Mac/Windows, check file sharing settings
# Docker Desktop → Settings → Resources → File Sharing
# Ensure your project path is allowed
```

### Build Fails with "Tool Not Found"

**Symptom**: cmake, gcc, or other tool not found

**Solutions**:
```bash
# 1. Verify tool installation
scripts/in-container.sh "which cmake gcc clang"

# 2. Check PATH
scripts/in-container.sh "echo \$PATH"

# 3. Tool might need manual PATH addition
# Check .devcontainer/finalize.sh for setup

# 4. Rebuild container to ensure fresh install
# In VS Code: F1 → "Dev Containers: Rebuild Container"
```

### Out of Disk Space

**Symptom**: "No space left on device" during build

**Solutions**:
```bash
# 1. Clean Docker resources
docker system df  # Check usage
docker system prune -a  # Clean up

# 2. Clean build directory
scripts/in-container.sh "rm -rf build"

# 3. Remove unused containers
docker ps -a
docker rm $(docker ps -aq -f status=exited)

# 4. Remove unused images
docker images
docker rmi $(docker images -q -f dangling=true)
```

---

## Related Documentation

- **[building.md](building.md)** - Complete build instructions for WAMR (to be created)
- **[testing.md](testing.md)** - Test suite documentation and guidelines (to be created)
- **[debugging.md](debugging.md)** - Debugging WAMR with GDB and Valgrind (to be created)
- **[code-quality.md](code-quality.md)** - Code formatting and quality standards (to be created)
- **[devcontainer.md](devcontainer.md)** - Basic devcontainer overview for VS Code users
- **[AGENTS.md](../AGENTS.md)** - Complete AI agent development guide (to be updated)

---

## Reference

### External Documentation

- [Development Containers Specification](https://containers.dev/)
- [VS Code Dev Containers Documentation](https://code.visualstudio.com/docs/devcontainers/containers)
- [Docker Documentation](https://docs.docker.com/)
- [devcontainer CLI Reference](https://github.com/devcontainers/cli)

### Container Configuration

**Base Image**: `mcr.microsoft.com/devcontainers/cpp:debian-12`

**Capabilities**:
- `SYS_PTRACE` enabled (required for GDB debugging)
- `seccomp=unconfined` (allows all system calls for Valgrind)

**User**: `vscode` (non-root, uid mapped to host)

**Workspace**: `/workspaces/ai-thoughts`

### Tool Versions

Exact versions are pinned in `.devcontainer/Dockerfile`:
- WASI SDK: 25.0
- WABT: 1.0.37
- Base OS: Debian 12 (Bookworm)
- clang-format: 14

### Script Internals

The `scripts/in-container.sh` wrapper implements:

1. **Container Detection**:
   - Reads cached name from `.devcontainer/.container-name`
   - Pattern matches `docker ps -a` output for `wamr.*dev`
   - Inspects container mounts for `/workspaces/ai-thoughts`

2. **Container Startup**:
   - Tries `devcontainer up` command first
   - Falls back to `docker-compose up -d`
   - Reports error if neither method available

3. **Command Execution**:
   - Uses `docker exec` with auto-detected TTY flags
   - Sets working directory to `/workspaces/ai-thoughts`
   - Runs as `vscode` user
   - Executes via `bash -c` for shell feature support

4. **Exit Code Propagation**:
   - Captures and returns command's actual exit code
   - Enables proper error handling in scripts

### Script Exit Codes

- `0` - Command succeeded
- `1` - Container failed to start or command failed
- `2` - Invalid options or usage error

---

## Quick Reference Card

**Most Common Commands:**

```bash
# Check status
scripts/in-container.sh --status

# Build project
scripts/in-container.sh "cmake -B build && cmake --build build"

# Run tests
scripts/in-container.sh "cd build && ctest --output-on-failure"

# Format code
scripts/in-container.sh "clang-format-14 -i core/iwasm/**/*.c"

# Get shell
scripts/in-container.sh "bash"

# Debug detection
scripts/in-container.sh --verbose --status
```

**Emergency Recovery:**

```bash
# Complete reset
docker ps -a | grep wamr | awk '{print $1}' | xargs docker rm -f
rm -f .devcontainer/.container-name
# Reopen in VS Code or run: scripts/in-container.sh "pwd"
```

---

**Documentation Version**: 1.0.0  
**Last Updated**: 2026-04-03  
**Maintained By**: WAMR Development Team
