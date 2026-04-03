#!/bin/bash
# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Smart wrapper script to execute commands inside WAMR devcontainer
# Usage: ./scripts/in-container.sh "<command>"

set -e

# Script version
VERSION="1.0.0"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CONTAINER_NAME_FILE="${PROJECT_ROOT}/.devcontainer/.container-name"

# Option flags
OPT_VERBOSE=false
OPT_QUIET=false
OPT_NO_START=false
OPT_CONTAINER_NAME=""

# Color output helpers (all output to stderr to avoid interfering with function return values)
info() {
    if [ "$OPT_QUIET" = false ]; then
        echo -e "\033[0;36m[INFO]\033[0m $*" >&2
    fi
}
error() { echo -e "\033[0;31m[ERROR]\033[0m $*" >&2; }
success() {
    if [ "$OPT_QUIET" = false ]; then
        echo -e "\033[0;32m[SUCCESS]\033[0m $*" >&2
    fi
}
verbose() {
    if [ "$OPT_VERBOSE" = true ]; then
        echo -e "\033[0;35m[VERBOSE]\033[0m $*" >&2
    fi
}

# Detect running or existing container
# Returns: container name if found, empty string otherwise
detect_container() {
    # Method 1: Read from saved file
    if [ -f "${CONTAINER_NAME_FILE}" ]; then
        local saved_name
        saved_name=$(cat "${CONTAINER_NAME_FILE}")
        # Verify container still exists
        if docker ps -a --format '{{.Names}}' | grep -q "^${saved_name}$"; then
            echo "${saved_name}"
            return 0
        fi
        # Stale file, remove it
        rm -f "${CONTAINER_NAME_FILE}"
    fi

    # Method 2: Find container by name pattern (WAMR-Dev or wamr-dev)
    local container_name
    container_name=$(docker ps -a --format '{{.Names}}' | grep -i "wamr.*dev" | head -n1)
    if [ -n "${container_name}" ]; then
        mkdir -p "$(dirname "${CONTAINER_NAME_FILE}")"
        echo "${container_name}" > "${CONTAINER_NAME_FILE}"
        echo "${container_name}"
        return 0
    fi

    # Method 3: Find container with project path mounted
    local project_basename
    project_basename=$(basename "${PROJECT_ROOT}")
    container_name=$(docker ps -a --format '{{.Names}}' | while read -r name; do
        if docker inspect "$name" 2>/dev/null | grep -q "/workspaces/${project_basename}"; then
            echo "$name"
            break
        fi
    done | head -n1)
    if [ -n "${container_name}" ]; then
        mkdir -p "$(dirname "${CONTAINER_NAME_FILE}")"
        echo "${container_name}" > "${CONTAINER_NAME_FILE}"
        echo "${container_name}"
        return 0
    fi

    # Not found
    return 1
}

# Start devcontainer
# Returns: 0 on success, 1 on failure
start_container() {
    info "Starting devcontainer..."

    # Check if devcontainer CLI is available
    if command -v devcontainer &> /dev/null; then
        info "Using devcontainer CLI..."
        if devcontainer up --workspace-folder "${PROJECT_ROOT}" >&2; then
            sleep 3  # Wait for container to be ready
            return 0
        else
            error "Failed to start container with devcontainer CLI"
            return 1
        fi
    fi

    # Fallback: Check for docker-compose
    if [ -f "${PROJECT_ROOT}/.devcontainer/docker-compose.yml" ]; then
        info "Using docker-compose..."
        if docker-compose -f "${PROJECT_ROOT}/.devcontainer/docker-compose.yml" up -d >&2; then
            sleep 3
            return 0
        else
            error "Failed to start container with docker-compose"
            return 1
        fi
    fi

    # No method available
    error "Cannot start container: devcontainer CLI not found and no docker-compose.yml"
    error "Please install devcontainer CLI: npm install -g @devcontainers/cli"
    error "Or open the project in VS Code and 'Reopen in Container'"
    return 1
}

# Ensure container is running
# Returns: container name on success, empty on failure
ensure_container() {
    local container_name
    container_name=$(detect_container)

    # If no container found, try to start one
    if [ -z "${container_name}" ]; then
        if start_container; then
            container_name=$(detect_container)
        else
            return 1
        fi
    fi

    # Check if container is running
    if ! docker ps --format '{{.Names}}' | grep -q "^${container_name}$"; then
        info "Container '${container_name}' exists but not running. Starting..."
        if docker start "${container_name}" > /dev/null; then
            sleep 2
            # Verify container is actually running
            if ! docker ps --format '{{.Names}}' | grep -q "^${container_name}$"; then
                error "Container '${container_name}' started but not running"
                return 1
            fi
        else
            error "Failed to start container '${container_name}'"
            return 1
        fi
    fi

    echo "${container_name}"
}

# Execute command in container
# Args: $1 = container name, $2+ = command to execute
# Note: Commands are executed via bash -c to support shell features (pipes, redirection, etc.)
#       This means shell metacharacters will be interpreted - only pass trusted input
exec_in_container() {
    local container_name="$1"
    shift

    info "Executing in container '${container_name}': $*"

    # Determine workspace path
    local project_basename
    project_basename=$(basename "${PROJECT_ROOT}")
    local workspace_path="/workspaces/${project_basename}"

    # Execute command
    # Detect if we need TTY flags (only when stdin is a TTY)
    local tty_flags=""
    if [ -t 0 ]; then
        tty_flags="--interactive --tty"
    fi

    docker exec \
        ${tty_flags} \
        --workdir "${workspace_path}" \
        --user vscode \
        "${container_name}" \
        bash -c "$*"
}

# Show help message
show_help() {
    cat << 'EOF'
Usage: scripts/in-container.sh [OPTIONS] <command>

Execute commands inside WAMR's devcontainer from your host machine.

DESCRIPTION
    This script automatically detects, starts, and manages the WAMR
    devcontainer, then executes your command inside it. You don't need
    to manually start VS Code or manage container lifecycle.

    The container provides a consistent build environment with all
    required toolchains: CMake, GCC, Clang, WASI-SDK, WABT, etc.

CONTAINER DETECTION
    The script uses three methods to find your devcontainer:
    1. Cached name from .devcontainer/.container-name
    2. Docker container name pattern matching (wamr.*dev)
    3. Mount path inspection (/workspaces/ai-thoughts)

    If no container is found, it will automatically start one using:
    - devcontainer CLI (if available), or
    - docker-compose (fallback)

OPTIONS
    -h, --help
        Show this help message and exit.

    --version
        Display script version and container information.

    --status
        Show current container status (running/stopped/not found) and exit.
        Does not start or modify containers.

    --container-name <name>
        Manually specify the container name to use.
        Bypasses automatic detection. Container must already exist.

    --no-start
        Fail if container is not running instead of auto-starting it.
        Useful for CI environments where containers should be pre-started.

    --verbose
        Enable verbose output showing detection steps and container info.
        Useful for debugging container detection issues.

    --quiet
        Suppress informational messages, show only command output and errors.
        Exit codes are still properly propagated.

EXAMPLES
    # Basic usage - auto-detect and use container
    scripts/in-container.sh "cmake -B build && cmake --build build"

    # Check container status without running commands
    scripts/in-container.sh --status

    # Use a specific container by name
    scripts/in-container.sh --container-name my_wamr_dev "make test"

    # Fail fast if container is not running (CI mode)
    scripts/in-container.sh --no-start "ctest --output-on-failure"

    # Debug container detection issues
    scripts/in-container.sh --verbose "pwd"

    # Quiet mode for clean output in scripts
    scripts/in-container.sh --quiet "git status" > status.txt

    # Run tests
    scripts/in-container.sh "cd build && ctest --output-on-failure"

    # Check available tools
    scripts/in-container.sh "which cmake gcc clang wasm-opt"

    # Interactive shell (when run from terminal)
    scripts/in-container.sh "bash"

    # Format code
    scripts/in-container.sh "clang-format-14 -i core/iwasm/**/*.c"

    # Run a Python test script
    scripts/in-container.sh "python3 tests/wamr-test-suites/test_wamr.py"

ENVIRONMENT
    - Working directory: /workspaces/ai-thoughts
    - User: vscode (non-root)
    - Shell: bash

NOTES
    - Commands are executed via 'bash -c', so shell features work (pipes, &&, etc.)
    - Exit codes are properly propagated to the host
    - Works in both interactive terminals and CI/CD pipelines
    - Container state is automatically managed (start/stop/restart)
    - Options must come before the command argument

EXIT CODES
    0    Command succeeded
    1    Container failed to start or command failed
    2    Invalid options or usage error

SEE ALSO
    - .devcontainer/devcontainer.json (container configuration)
    - .devcontainer/Dockerfile (container image definition)
    - AGENTS.md (development guide for AI agents)
EOF
}

# Show version information
show_version() {
    echo "in-container.sh version ${VERSION}"
    echo ""
    echo "Container detection methods:"
    echo "  1. Cached name (.devcontainer/.container-name)"
    echo "  2. Name pattern matching (wamr.*dev)"
    echo "  3. Mount path inspection (/workspaces/ai-thoughts)"
    echo ""
    echo "Container startup methods:"
    echo "  - devcontainer CLI (primary)"
    echo "  - docker-compose (fallback)"
}

# Show container status
show_status() {
    local container_name
    container_name=$(detect_container)

    if [ -z "${container_name}" ]; then
        echo "Status: No container found"
        echo ""
        echo "Suggestions:"
        echo "  - Run a command to auto-start: scripts/in-container.sh \"pwd\""
        echo "  - Open in VS Code: Reopen in Container"
        echo "  - Manual start: devcontainer up --workspace-folder ."
        return 1
    fi

    echo "Container: ${container_name}"
    echo ""

    # Check if running
    if docker ps --format '{{.Names}}' | grep -q "^${container_name}$"; then
        echo "Status: Running ✓"

        # Get container info
        local container_id
        container_id=$(docker ps --format '{{.ID}}' --filter "name=^${container_name}$" | head -n1)

        echo ""
        echo "Details:"
        docker inspect "${container_id}" --format '  Image: {{.Config.Image}}'
        docker inspect "${container_id}" --format '  Created: {{.Created}}'
        docker inspect "${container_id}" --format '  Uptime: {{.State.Status}} (since {{.State.StartedAt}})'

        # Show workspace mount
        local workspace_mount
        workspace_mount=$(docker inspect "${container_id}" --format '{{range .Mounts}}{{if eq .Destination "/workspaces/ai-thoughts"}}{{.Source}}{{end}}{{end}}')
        if [ -n "${workspace_mount}" ]; then
            echo "  Workspace: ${workspace_mount} -> /workspaces/ai-thoughts"
        fi
    else
        echo "Status: Stopped ✗"
        echo ""
        echo "To start: scripts/in-container.sh \"pwd\""
        echo "Or: docker start ${container_name}"
        return 1
    fi
}

# Main entry point
main() {
    # Parse options
    while [ $# -gt 0 ]; do
        case "$1" in
            -h|--help|help)
                show_help
                exit 0
                ;;
            --version)
                show_version
                exit 0
                ;;
            --status)
                show_status
                exit $?
                ;;
            --verbose)
                OPT_VERBOSE=true
                shift
                ;;
            --quiet)
                OPT_QUIET=true
                shift
                ;;
            --no-start)
                OPT_NO_START=true
                shift
                ;;
            --container-name)
                if [ -z "$2" ] || [[ "$2" == -* ]]; then
                    error "Option --container-name requires an argument"
                    exit 2
                fi
                OPT_CONTAINER_NAME="$2"
                shift 2
                ;;
            -*)
                error "Unknown option: $1"
                echo "" >&2
                echo "Run 'scripts/in-container.sh --help' for usage information." >&2
                exit 2
                ;;
            *)
                # First non-option argument is the command
                break
                ;;
        esac
    done

    # Check if command provided
    if [ $# -eq 0 ]; then
        error "No command provided"
        echo "" >&2
        show_help
        exit 1
    fi

    # Determine container to use
    local container_name

    if [ -n "${OPT_CONTAINER_NAME}" ]; then
        # Use manually specified container
        verbose "Using manually specified container: ${OPT_CONTAINER_NAME}"
        container_name="${OPT_CONTAINER_NAME}"

        # Verify it exists
        if ! docker ps -a --format '{{.Names}}' | grep -q "^${container_name}$"; then
            error "Container '${container_name}' not found"
            exit 1
        fi
    else
        # Auto-detect or start container
        verbose "Auto-detecting container..."
        container_name=$(detect_container)

        if [ -z "${container_name}" ]; then
            if [ "$OPT_NO_START" = true ]; then
                error "No container found and --no-start specified"
                exit 1
            fi

            verbose "No container found, starting new one..."
            if ! start_container; then
                error "Failed to start container"
                exit 1
            fi

            container_name=$(detect_container)
            if [ -z "${container_name}" ]; then
                error "Container started but detection failed"
                exit 1
            fi
        else
            verbose "Found container: ${container_name}"
        fi
    fi

    # Ensure container is running
    if ! docker ps --format '{{.Names}}' | grep -q "^${container_name}$"; then
        if [ "$OPT_NO_START" = true ]; then
            error "Container '${container_name}' is not running and --no-start specified"
            exit 1
        fi

        info "Container '${container_name}' exists but not running. Starting..."
        if ! docker start "${container_name}" > /dev/null; then
            error "Failed to start container '${container_name}'"
            exit 1
        fi
        sleep 2

        # Verify it's running
        if ! docker ps --format '{{.Names}}' | grep -q "^${container_name}$"; then
            error "Container '${container_name}' started but not running"
            exit 1
        fi
    fi

    success "Using container: ${container_name}"
    verbose "Executing command: $*"

    # Execute command and propagate exit code
    exec_in_container "${container_name}" "$@"
    exit $?
}

# Run main
main "$@"
