# Development in Devcontainer

## Overview

The WAMR project uses Docker-based development containers (devcontainers) to provide a consistent, reproducible build environment. This document covers technical details about the devcontainer setup.

> **For command execution requirements and workflows, see [AGENTS.md](../AGENTS.md).**

### Pre-installed Tools

The WAMR devcontainer includes all necessary development tools:

- **Build Tools**: CMake, Ninja, Make, ccache
- **Compilers**: GCC (with multilib), Clang, LLVM
- **WebAssembly Tools**: WASI SDK 25.0, WABT 1.0.37
- **Debugging**: GDB, Valgrind
- **Code Quality**: clang-format-14
- **Languages**: Python 3, OCaml


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

### User Permissions

- **Container User**: `vscode` (non-root)
- **User ID**: Mapped to match host user (prevents permission issues)
- **File Ownership**: Files created in container are owned by your host user

## Troubleshooting

### Container Won't Start

**Solutions**:
```bash
# Check Docker is running
docker ps

# Check for conflicting containers
docker ps -a | grep devcontainer
docker rm -f <old-container-name>

# Try manual start via VS Code
# Open project in VS Code → F1 → "Reopen in Container"

# Check Docker daemon
systemctl status docker  # Linux
# or check Docker Desktop status on Mac/Windows

# Clean up and retry
docker system prune -a
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
   cmake -B build -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
   ```

3. **Adjust Docker resources**:
   - Docker Desktop → Settings → Resources
   - Increase CPU cores and RAM allocation
   - Recommended: 4+ CPUs, 8+ GB RAM

4. **Use named volumes for build cache**:
   - Modify `.devcontainer/devcontainer.json`
   - Add volume mounts for `build/` directory

### "Container Not Found" After System Restart

**Solutions**:
```bash
# Restart via VS Code
# F1 → "Dev Containers: Reopen in Container"

# Or check container status
docker ps -a | grep devcontainer
docker start <container-name>
```

### Changes Not Appearing

**Symptom**: File changes on host don't appear in container

**Solutions**:
```bash
# Check Docker volume mounts
docker inspect <container-name> | grep Mounts -A 20

# Restart container
docker restart <container-name>

# If using Docker Desktop on Mac/Windows, check file sharing settings
# Docker Desktop → Settings → Resources → File Sharing
# Ensure your project path is allowed
```

### Build Fails with "Tool Not Found"

**Symptom**: cmake, gcc, or other tool not found

**Solutions**:
```bash
# Rebuild container to ensure fresh install
# In VS Code: F1 → "Dev Containers: Rebuild Container"

# Or check .devcontainer/finalize.sh for tool installation
```

### Out of Disk Space

**Symptom**: "No space left on device" during build

**Solutions**:
```bash
# Clean Docker resources
docker system df  # Check usage
docker system prune -a  # Clean up

# Remove unused containers
docker ps -a
docker rm $(docker ps -aq -f status=exited)

# Remove unused images
docker images
docker rmi $(docker images -q -f dangling=true)
```

## Related Documentation

- **[building.md](building.md)** - Complete build instructions for WAMR (to be created)
- **[testing.md](testing.md)** - Test suite documentation and guidelines (to be created)
- **[debugging.md](debugging.md)** - Debugging WAMR with GDB and Valgrind (to be created)
- **[code-quality.md](code-quality.md)** - Code formatting and quality standards (to be created)
- **[devcontainer.md](devcontainer.md)** - Basic devcontainer overview for VS Code users
- **[AGENTS.md](../AGENTS.md)** - Complete AI agent development guide (to be updated)

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


**Documentation Version**: 1.0.0  
**Last Updated**: 2026-04-03  
**Maintained By**: WAMR Development Team
