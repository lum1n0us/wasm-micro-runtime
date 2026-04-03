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

# TODO: Add main logic
echo "Script skeleton created"
