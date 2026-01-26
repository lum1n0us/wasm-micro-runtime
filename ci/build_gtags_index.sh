#!/bin/bash
# build_gtags_index.sh - WAMR Project Gtags Index Generator
# 
# Usage: ./build_gtags_index.sh <repo_root>
# 
# Features:
# - Generate gtags index for WAMR source code
# - Exclude test directories and build artifacts
# - Generate simple reports (console + file)
#
# Author: OpenCode

set -euo pipefail

# ============================================================================
# Configuration Parameters
# ============================================================================

# List of directories to scan (relative to repo root)
TARGET_DIRS=("core/iwasm" "core/shared" "product-mini/platforms/linux" "wamr-compiler")

# Supported source file extensions
SOURCE_EXTENSIONS=("*.c" "*.h" "*.cc" "*.s" "*.S")

# Output directory name (will be created in repo root)
INDEX_DIR_NAME="gtags-indexes"

# Cache validity period (seconds), default 24 hours
CACHE_TTL=86400

# Statistics variables
declare -A STATS

# ============================================================================
# Utility Functions
# ============================================================================

show_usage() {
    echo "Usage: $0 <repo_root>"
    echo
    echo "Arguments:"
    echo "  repo_root    Path to the WAMR repository root directory"
    echo
    echo "Options:"
    echo "  -h, --help   Show this help message"
    echo "  --clean      Clean index and cache"
    echo
    echo "Example:"
    echo "  $0 /path/to/wasm-micro-runtime"
    echo "  $0 . # if running from repo root"
    echo
    exit 1
}

log_info() {
    echo "ℹ️  $*" >&2
}

log_success() {
    echo "✅ $*" >&2
}

log_error() {
    echo "❌ $*" >&2
}

# ============================================================================
# Main Functions
# ============================================================================

check_gtags_available() {
    if ! command -v gtags >/dev/null 2>&1; then
        log_error "gtags tool is not installed"
        echo
        echo "Installation instructions:"
        echo "  Ubuntu/Debian: sudo apt install global"
        echo "  macOS:         brew install global"
        echo "  CentOS/RHEL:   sudo yum install global"
        return 1
    fi
    
    if ! command -v global >/dev/null 2>&1; then
        log_error "global tool is not installed"
        return 1
    fi
    
    return 0
}

validate_repo_root() {
    local repo_root="$1"
    
    if [ ! -d "$repo_root" ]; then
        log_error "Repository root directory does not exist: $repo_root"
        return 1
    fi
    
    # Check if it looks like WAMR repo
    if [ ! -d "$repo_root/core" ]; then
        log_error "Directory does not appear to be WAMR repository (missing core/ directory): $repo_root"
        return 1
    fi
    
    return 0
}

discover_and_count_files() {
    local repo_root="$1"
    local total_files=0
    
    log_info "Scanning source files..."
    
    for dir in "${TARGET_DIRS[@]}"; do
        local full_path="$repo_root/$dir"
        if [ ! -d "$full_path" ]; then
            continue
        fi
        
        local count=$(find "$full_path" -type f \
            \( -name "*.c" -o -name "*.h" -o -name "*.cc" -o -name "*.s" -o -name "*.S" \) \
            ! -path "*/build/*" \
            ! -path "*/CMakeFiles/*" \
            ! -path "*/.git/*" \
            ! -path "*/test/*" \
            ! -path "*/tests/*" \
            ! -path "*/stress-test/*" \
            ! -name "*.o" ! -name "*.so" ! -name "*.a" ! -name "*.d" \
            ! -type l \
            2>/dev/null | wc -l)
        
        total_files=$((total_files + count))
    done
    
    echo "$total_files"
}

generate_file_list() {
    local repo_root="$1"
    local file_list="$2"
    
    > "$file_list"  # Clear file
    
    for dir in "${TARGET_DIRS[@]}"; do
        local full_path="$repo_root/$dir"
        if [ ! -d "$full_path" ]; then
            continue
        fi
        
        find "$full_path" -type f \
            \( -name "*.c" -o -name "*.h" -o -name "*.cc" -o -name "*.s" -o -name "*.S" \) \
            ! -path "*/build/*" \
            ! -path "*/CMakeFiles/*" \
            ! -path "*/.git/*" \
            ! -path "*/test/*" \
            ! -path "*/tests/*" \
            ! -path "*/stress-test/*" \
            ! -name "*.o" ! -name "*.so" ! -name "*.a" ! -name "*.d" \
            ! -type l \
            2>/dev/null >> "$file_list"
    done
    
    sort -u "$file_list" -o "$file_list"
}

run_gtags_indexing() {
    local repo_root="$1"
    local file_list="$2"
    local index_dir="$3"
    
    local file_count=$(wc -l < "$file_list")
    log_info "Indexing $file_count files..."
    
    # Clean old index files
    rm -f "$index_dir/GTAGS" "$index_dir/GRTAGS" "$index_dir/GPATH"
    
    # Run gtags
    local start_time=$(date +%s)
    
    if cd "$repo_root" && gtags -f "$file_list" "$index_dir"; then
        local end_time=$(date +%s)
        local duration=$((end_time - start_time))
        
        STATS[indexing_time]="$duration"
        STATS[indexing_success]="yes"
        return 0
    else
        STATS[indexing_success]="no"
        return 1
    fi
}

collect_symbol_statistics() {
    local index_dir="$1"
    
    if [ -f "$index_dir/GTAGS" ]; then
        cd "$index_dir"
        local total_symbols=$(global -c 2>/dev/null | wc -l || echo "0")
        STATS[total_symbols]="$total_symbols"
        cd - > /dev/null
    else
        STATS[total_symbols]="0"
    fi
}

generate_console_report() {
    local repo_root="$1"
    local index_dir="$2"
    local file_count="$3"
    
    echo
    echo "========================================"
    echo "WAMR Gtags Index Report"
    echo "========================================"
    
    # Status
    if [ "${STATS[indexing_success]:-no}" = "yes" ]; then
        echo "Status: ✅ SUCCESS"
    else
        echo "Status: ❌ FAILED"
    fi
    
    # Configuration
    echo "Target Directories: ${TARGET_DIRS[*]}"
    echo "Output Directory: $index_dir"
    
    # Counts
    echo "File Count: $file_count"
    echo "Symbol Count: ${STATS[total_symbols]:-0}"
    
    # Performance
    echo "Processing Time: ${STATS[indexing_time]:-0} seconds"
    
    echo "========================================"
}

generate_file_report() {
    local repo_root="$1"
    local index_dir="$2" 
    local file_count="$3"
    local report_file="$4"
    
    cat > "$report_file" << EOF
WAMR Gtags Index Report
Generated: $(date)

Status: $([ "${STATS[indexing_success]:-no}" = "yes" ] && echo "SUCCESS" || echo "FAILED")
Target Directories: ${TARGET_DIRS[*]}
Output Directory: $index_dir
File Count: $file_count
Symbol Count: ${STATS[total_symbols]:-0}
Processing Time: ${STATS[indexing_time]:-0} seconds
EOF
}

main() {
    local repo_root="$1"
    local start_time=$(date +%s)
    
    # Validate inputs
    if ! validate_repo_root "$repo_root"; then
        exit 1
    fi
    
    # Setup paths
    local index_dir="$repo_root/$INDEX_DIR_NAME"
    local file_list="$index_dir/files.list"
    local report_file="$index_dir/report.txt"
    
    # Create directories
    mkdir -p "$index_dir"
    
    # Check dependencies
    if ! check_gtags_available; then
        exit 1
    fi
    
    # Count files
    local file_count
    file_count=$(discover_and_count_files "$repo_root")
    
    if [ "$file_count" -eq 0 ]; then
        log_error "No source files found"
        exit 1
    fi
    
    # Generate file list
    generate_file_list "$repo_root" "$file_list"
    
    # Run indexing
    if run_gtags_indexing "$repo_root" "$file_list" "$index_dir"; then
        log_success "Indexing completed successfully"
    else
        log_error "Indexing failed"
    fi
    
    # Collect statistics
    collect_symbol_statistics "$index_dir"
    
    # Calculate total time
    local end_time=$(date +%s)
    local total_time=$((end_time - start_time))
    STATS[total_time]="$total_time"
    
    # Generate reports
    generate_console_report "$repo_root" "$index_dir" "$file_count"
    generate_file_report "$repo_root" "$index_dir" "$file_count" "$report_file"
    
    echo "Report saved to: $report_file"
    
    if [ "${STATS[indexing_success]:-no}" = "yes" ]; then
        exit 0
    else
        exit 1
    fi
}

# ============================================================================
# Script Entry Point
# ============================================================================

# Handle command line arguments
if [ $# -eq 0 ]; then
    echo "Error: Repository root path is required"
    echo
    show_usage
fi

case "${1:-}" in
    -h|--help)
        show_usage
        ;;
    --clean)
        if [ $# -lt 2 ]; then
            echo "Error: Repository root path is required with --clean"
            show_usage
        fi
        repo_root="$2"
        if validate_repo_root "$repo_root"; then
            echo "Cleaning index and cache..."
            rm -rf "$repo_root/$INDEX_DIR_NAME"
            echo "Cleaning completed"
            exit 0
        else
            exit 1
        fi
        ;;
    -*)
        echo "Error: Unknown option: $1"
        show_usage
        ;;
    *)
        # Run main function with repo root
        main "$1"
        ;;
esac