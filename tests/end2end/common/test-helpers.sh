#!/bin/bash
# Shared utilities for E2E test scripts

# Color output (disabled if not a TTY)
if [ -t 1 ]; then
    GREEN='\033[0;32m'
    RED='\033[0;31m'
    YELLOW='\033[1;33m'
    BLUE='\033[0;34m'
    NC='\033[0m'
else
    GREEN=''; RED=''; YELLOW=''; BLUE=''; NC=''
fi

log_step() {
    echo -e "${BLUE}[STEP]${NC} $*"
}

log_success() {
    echo -e "${GREEN}[PASS]${NC} $*"
}

log_error() {
    echo -e "${RED}[FAIL]${NC} $*" >&2
}

log_warning() {
    echo -e "${YELLOW}[WARN]${NC} $*"
}

check_tool() {
    local tool_path=$1
    local tool_name=$2

    if [ ! -x "${tool_path}" ]; then
        log_error "${tool_name} not found or not executable: ${tool_path}"
        exit 1
    fi
}

validate_bool() {
    local value=$1
    local name=$2

    if [ "${value}" != "true" ] && [ "${value}" != "false" ]; then
        log_error "Invalid ${name}: ${value} (must be 'true' or 'false')"
        exit 1
    fi
}

validate_expect() {
    local value=$1
    local name=$2

    if [ "${value}" != "success" ] && [ "${value}" != "fail" ]; then
        log_error "Invalid ${name}: ${value} (must be 'success' or 'fail')"
        exit 1
    fi
}
