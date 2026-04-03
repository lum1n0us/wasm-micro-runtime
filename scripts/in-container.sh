#!/bin/bash
# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Smart wrapper script to execute commands inside WAMR devcontainer
# Usage: ./scripts/in-container.sh "<command>"

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CONTAINER_NAME_FILE="${PROJECT_ROOT}/.devcontainer/.container-name"

# Color output helpers
info() { echo -e "\033[0;36m[INFO]\033[0m $*"; }
error() { echo -e "\033[0;31m[ERROR]\033[0m $*" >&2; }
success() { echo -e "\033[0;32m[SUCCESS]\033[0m $*"; }

# Detect running or existing container
# Returns: container name if found, empty string otherwise
detect_container() {
    # Method 1: Read from saved file
    if [ -f "${CONTAINER_NAME_FILE}" ]; then
        local saved_name=$(cat "${CONTAINER_NAME_FILE}")
        # Verify container still exists
        if docker ps -a --format '{{.Names}}' | grep -q "^${saved_name}$"; then
            echo "${saved_name}"
            return 0
        fi
        # Stale file, remove it
        rm -f "${CONTAINER_NAME_FILE}"
    fi

    # Method 2: Find container by name pattern (WAMR-Dev or wamr-dev)
    local container_name=$(docker ps -a --format '{{.Names}}' | grep -i "wamr.*dev" | head -n1)
    if [ -n "${container_name}" ]; then
        echo "${container_name}" > "${CONTAINER_NAME_FILE}"
        echo "${container_name}"
        return 0
    fi

    # Method 3: Find container with project path mounted
    local project_basename=$(basename "${PROJECT_ROOT}")
    container_name=$(docker ps -a --format '{{.Names}}' --filter "volume=/workspaces/${project_basename}" | head -n1)
    if [ -n "${container_name}" ]; then
        echo "${container_name}" > "${CONTAINER_NAME_FILE}"
        echo "${container_name}"
        return 0
    fi

    # Not found
    return 1
}

# TODO: Add main logic
