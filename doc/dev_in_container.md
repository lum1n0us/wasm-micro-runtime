# Development in Devcontainer

This guide covers WAMR's devcontainer technical details, VS Code integration, and container troubleshooting.


---

## Overview

The WAMR project uses Docker-based development containers (devcontainers) to provide a consistent, reproducible build environment with all required toolchains.

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

```bash
# Check Docker is running
docker ps

# Remove conflicting containers
docker ps -a | grep devcontainer
docker rm -f <old-container-name>

# Or use VS Code: F1 → "Reopen in Container"

# Clean up and retry
docker system prune -a
```

### Permission Errors

"Permission denied" when building or writing files:

```bash
# Fix ownership (run on host)
sudo chown -R $USER:$USER build/

# Verify container user matches host
docker exec <container> id

# Rebuild container if needed
# VS Code: F1 → "Dev Containers: Rebuild Container"
```

### Container Is Slow

Builds are slower in container than on host:

1. **Enable Docker BuildKit**: `export DOCKER_BUILDKIT=1`
2. **Use ccache**: `cmake -B build -DCMAKE_CXX_COMPILER_LAUNCHER=ccache`
3. **Increase Docker resources**: Docker Desktop → Settings → Resources (4+ CPUs, 8+ GB RAM)
4. **Use named volumes**: Modify `.devcontainer/devcontainer.json` to cache `build/` directory

### Container Not Found After Restart

```bash
# Restart via VS Code: F1 → "Reopen in Container"
# Or manually: docker ps -a | grep devcontainer && docker start <container-name>
```

### Changes Not Appearing

File changes on host don't appear in container:

```bash
# Restart container
docker restart <container-name>

# Docker Desktop (Mac/Windows): Check file sharing
# Settings → Resources → File Sharing → Ensure project path allowed
```

### Build Fails with "Tool Not Found"

cmake, gcc, or other tool not found:

```bash
# Rebuild container: F1 → "Dev Containers: Rebuild Container"
# Or check .devcontainer/finalize.sh for tool installation
```

### Out of Disk Space

"No space left on device" during build:

```bash
# Clean Docker resources
docker system df  # Check usage
docker system prune -a  # Clean up

# Remove unused containers and images
docker rm $(docker ps -aq -f status=exited)
docker rmi $(docker images -q -f dangling=true)
```

## Related Documentation

- **[linting.md](linting.md)** - Code formatting and quality standards

## Reference

### Container Configuration

- **Base Image**: `mcr.microsoft.com/devcontainers/cpp:debian-12`
- **User**: `vscode` (non-root, uid mapped to host)
- **Workspace**: `/workspaces/ai-thoughts`
- **Capabilities**: `SYS_PTRACE` enabled, `seccomp=unconfined`

### Tool Versions

Pinned in `.devcontainer/Dockerfile`:
- WASI SDK: 25.0
- WABT: 1.0.37
- Base OS: Debian 12 (Bookworm)
- clang-format: 14

### External Documentation

- [Development Containers Specification](https://containers.dev/)
- [VS Code Dev Containers](https://code.visualstudio.com/docs/devcontainers/containers)
- [devcontainer CLI](https://github.com/devcontainers/cli)
