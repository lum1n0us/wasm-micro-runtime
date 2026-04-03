#!/bin/bash
# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Smart wrapper script to execute commands inside WAMR devcontainer
# Usage: ./scripts/in-container.sh "<command>"

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CONTAINER_NAME_FILE="${PROJECT_ROOT}/.devcontainer/.container-name"

# Color output helpers (all output to stderr to avoid interfering with function return values)
info() { echo -e "\033[0;36m[INFO]\033[0m $*" >&2; }
error() { echo -e "\033[0;31m[ERROR]\033[0m $*" >&2; }
success() { echo -e "\033[0;32m[SUCCESS]\033[0m $*" >&2; }

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
Usage: scripts/in-container.sh <command>

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

EXAMPLES
    # Build WAMR with CMake
    scripts/in-container.sh "cmake -B build && cmake --build build"

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

OPTIONS
    --help, -h, help    Show this help message

EXIT CODES
    0    Command succeeded
    1    Container failed to start or command failed

SEE ALSO
    - .devcontainer/devcontainer.json (container configuration)
    - .devcontainer/Dockerfile (container image definition)
    - AGENTS.md (development guide for AI agents)
EOF
}

# Main entry point
main() {
    # Check for help flag
    if [ $# -eq 0 ] || [ "$1" = "--help" ] || [ "$1" = "-h" ] || [ "$1" = "help" ]; then
        if [ $# -eq 0 ]; then
            error "No command provided"
            echo "" >&2
        fi
        show_help
        exit $([ $# -eq 0 ] && echo 1 || echo 0)
    fi

    # Ensure container is running
    local container_name
    container_name=$(ensure_container)

    if [ -z "${container_name}" ]; then
        error "Failed to start or detect devcontainer"
        exit 1
    fi

    success "Using container: ${container_name}"

    # Execute command and propagate exit code
    exec_in_container "${container_name}" "$@"
    exit $?
}

# Run main
main "$@"
