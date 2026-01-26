#!/bin/bash
# build_gtags_index.sh - WAMR Project Code Index Generator
#
# Usage: ./build_gtags_index.sh <repo_root> [--ctags|--gtags]
#
# Features:
# - Generate code index for WAMR source code using ctags (default) or gtags
# - Exclude test directories, build artifacts, and CoAP directory
# - Generate simple reports (console + file)
# - Create symbols.list with function names (ctags) or all symbols (gtags)
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

# Default indexing tool (ctags or gtags)
DEFAULT_TOOL="ctags"

# Statistics variables
declare -A STATS

# Global variables
INDEXING_TOOL=""

# ============================================================================
# Utility Functions
# ============================================================================

show_usage() {
    echo "Usage: $0 <repo_root> [--ctags|--gtags]"
    echo
    echo "Arguments:"
    echo "  repo_root    Path to the WAMR repository root directory"
    echo
    echo "Options:"
    echo "  --ctags      Use Universal ctags for indexing (default)"
    echo "  --gtags      Use GNU Global gtags for indexing"
    echo "  -h, --help   Show this help message"
    echo "  --clean      Clean index and cache"
    echo
    echo "Examples:"
    echo "  $0 .                    # Use ctags (default)"
    echo "  $0 . --ctags            # Explicit ctags"
    echo "  $0 . --gtags            # Use gtags"
    echo "  $0 /path/to/wasm-micro-runtime --ctags"
    echo
    echo "Output:"
    echo "  - ctags: Functions only in symbols.list (one per line)"
    echo "  - gtags: All symbols sample in symbols.list"
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
# Dependency Checking
# ============================================================================

check_ctags_available() {
    if ! command -v ctags >/dev/null 2>&1; then
        log_error "Universal ctags is not installed"
        echo
        echo "Installation instructions:"
        echo "  Ubuntu/Debian: sudo apt install universal-ctags"
        echo "  macOS:         brew install universal-ctags"
        echo "  CentOS/RHEL:   sudo yum install ctags"
        echo
        return 1
    fi

    # Verify it's universal-ctags (not exuberant)
    if ! ctags --version 2>/dev/null | grep -q "Universal Ctags"; then
        log_error "Found ctags but it's not Universal Ctags"
        echo
        echo "Please install universal-ctags specifically:"
        echo "  Ubuntu/Debian: sudo apt install universal-ctags"
        echo "  macOS:         brew install universal-ctags"
        echo
        return 1
    fi

    local ctags_version=$(ctags --version | head -1)
    log_success "Found Universal ctags: $ctags_version"
    return 0
}

check_gtags_available() {
    if ! command -v gtags >/dev/null 2>&1; then
        log_error "gtags tool is not installed"
        echo
        echo "Installation instructions:"
        echo "  Ubuntu/Debian: sudo apt install global"
        echo "  macOS:         brew install global"
        echo "  CentOS/RHEL:   sudo yum install global"
        echo
        return 1
    fi

    if ! command -v global >/dev/null 2>&1; then
        log_error "global tool is not installed"
        echo
        echo "Installation instructions:"
        echo "  Ubuntu/Debian: sudo apt install global"
        echo "  macOS:         brew install global"
        echo
        return 1
    fi

    local gtags_version=$(gtags --version | head -1)
    log_success "Found gtags: $gtags_version"
    return 0
}

# ============================================================================
# Main Functions
# ============================================================================

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
            ! -path "*/coap/*" \
            ! -path "*/lib-socket/*" \
            ! -path "*/freertos/*" \
            ! -path "*/debug/*" \
            ! -path "*/platform/alios/*" \
            ! -path "*/platform/android/*" \
            ! -path "*/platform/cosmopolitan/*" \
            ! -path "*/platform/darwin/*" \
            ! -path "*/platform/ego/*" \
            ! -path "*/platform/esp-idf/*" \
            ! -path "*/platform/freebsd/*" \
            ! -path "*/platform/linux-sgx/*" \
            ! -path "*/platform/nuttx/*" \
            ! -path "*/platform/riot/*" \
            ! -path "*/platform/rt-thread/*" \
            ! -path "*/platform/vxworks/*" \
            ! -path "*/platform/windows/*" \
            ! -path "*/platform/zephyr/*" \
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
            ! -path "*/coap/*" \
            ! -path "*/lib-socket/*" \
            ! -path "*/freertos/*" \
            ! -path "*/debug/*" \
            ! -path "*/platform/alios/*" \
            ! -path "*/platform/android/*" \
            ! -path "*/platform/cosmopolitan/*" \
            ! -path "*/platform/darwin/*" \
            ! -path "*/platform/ego/*" \
            ! -path "*/platform/esp-idf/*" \
            ! -path "*/platform/freebsd/*" \
            ! -path "*/platform/linux-sgx/*" \
            ! -path "*/platform/nuttx/*" \
            ! -path "*/platform/riot/*" \
            ! -path "*/platform/rt-thread/*" \
            ! -path "*/platform/vxworks/*" \
            ! -path "*/platform/windows/*" \
            ! -path "*/platform/zephyr/*" \
            ! -name "*.o" ! -name "*.so" ! -name "*.a" ! -name "*.d" \
            ! -type l \
            2>/dev/null >> "$file_list"
    done

    sort -u "$file_list" -o "$file_list"
}

# ============================================================================
# ctags Implementation
# ============================================================================

run_ctags_indexing() {
    local repo_root="$1"
    local file_list="$2"
    local index_dir="$3"

    local file_count=$(wc -l < "$file_list")
    log_info "Using ctags to index $file_count files..."

    # Clean old index files
    rm -f "$index_dir/tags"

    # Run ctags
    local start_time=$(date +%s)

    if cd "$repo_root" && ctags --languages=C,C++ --kinds-C=fp --kinds-C++=fp -u -L "$file_list" -f "$index_dir/tags"; then
        local end_time=$(date +%s)
        local duration=$((end_time - start_time))

        STATS[indexing_time]="$duration"
        STATS[indexing_success]="yes"
        STATS[tool_used]="ctags (universal-ctags)"
        return 0
    else
        STATS[indexing_success]="no"
        STATS[tool_used]="ctags (universal-ctags)"
        return 1
    fi
}

collect_ctags_statistics() {
    local index_dir="$1"

    if [ -f "$index_dir/tags" ]; then
        # Count functions only - ctags format: name<tab>file<tab>pattern<tab>kind
        local function_count=$(awk -F'\t' '$4 == "f" {count++} END {print count+0}' "$index_dir/tags")
        STATS[total_symbols]="$function_count"
    else
        STATS[total_symbols]="0"
    fi
}

dump_ctags_functions() {
    local index_dir="$1"
    local symbols_file="$2"

    log_info "Dumping functions to symbols.list..."

    if [ -f "$index_dir/tags" ]; then
        # Create header for symbols file (use absolute path)
        local abs_symbols_file="$(cd "$(dirname "$symbols_file")" && pwd)/$(basename "$symbols_file")"

        cat > "$abs_symbols_file" << EOF
# WAMR ctags Function Dump
# Generated: $(date)
# Format: One function name per line
# Tool: Universal ctags
# Total functions: ${STATS[total_symbols]:-0}
#
EOF

        # Extract function names only (one per line) - ctags format: name<tab>file<tab>pattern<tab>kind
        awk -F'\t' '$4 == "f" {print $1}' "$index_dir/tags" >> "$abs_symbols_file"

        log_success "Function dump completed (functions only)"
    else
        log_error "ctags database not found, cannot dump functions"
        touch "$symbols_file"  # Create empty file
    fi
}

# ============================================================================
# gtags Implementation (existing functionality)
# ============================================================================

run_gtags_indexing() {
    local repo_root="$1"
    local file_list="$2"
    local index_dir="$3"

    local file_count=$(wc -l < "$file_list")
    log_info "Using gtags to index $file_count files..."

    # Clean old index files
    rm -f "$index_dir/GTAGS" "$index_dir/GRTAGS" "$index_dir/GPATH"

    # Run gtags
    local start_time=$(date +%s)

    if cd "$repo_root" && gtags -f "$file_list" "$index_dir"; then
        local end_time=$(date +%s)
        local duration=$((end_time - start_time))

        STATS[indexing_time]="$duration"
        STATS[indexing_success]="yes"
        STATS[tool_used]="gtags (GNU Global)"
        return 0
    else
        STATS[indexing_success]="no"
        STATS[tool_used]="gtags (GNU Global)"
        return 1
    fi
}

collect_gtags_statistics() {
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

dump_gtags_symbols() {
    local index_dir="$1"
    local symbols_file="$2"

    log_info "Dumping symbols to symbols.list..."

    if [ -f "$index_dir/GTAGS" ]; then
        # Create header for symbols file (use absolute path)
        local abs_symbols_file="$(cd "$(dirname "$symbols_file")" && pwd)/$(basename "$symbols_file")"

        cat > "$abs_symbols_file" << EOF
# WAMR gtags Symbol Dump
# Generated: $(date)
# Format: All symbol names
# Tool: GNU Global gtags
# Total symbols in database: ${STATS[total_symbols]:-0}
#
EOF

        # Change to index directory for global commands
        cd "$index_dir"

        # Get ALL symbols (no limit)
        global -c >> "$abs_symbols_file" 2>/dev/null || true

        cd - > /dev/null
        log_success "Symbol dump completed (all symbols)"
    else
        log_error "gtags database not found, cannot dump symbols"
        touch "$symbols_file"  # Create empty file
    fi
}

# ============================================================================
# Unified Interface Functions
# ============================================================================

run_indexing() {
    local repo_root="$1"
    local file_list="$2"
    local index_dir="$3"

    if [ "$INDEXING_TOOL" = "ctags" ]; then
        run_ctags_indexing "$repo_root" "$file_list" "$index_dir"
    else
        run_gtags_indexing "$repo_root" "$file_list" "$index_dir"
    fi
}

collect_symbol_statistics() {
    local index_dir="$1"

    if [ "$INDEXING_TOOL" = "ctags" ]; then
        collect_ctags_statistics "$index_dir"
    else
        collect_gtags_statistics "$index_dir"
    fi
}

dump_all_symbols() {
    local index_dir="$1"
    local symbols_file="$2"

    if [ "$INDEXING_TOOL" = "ctags" ]; then
        dump_ctags_functions "$index_dir" "$symbols_file"
    else
        dump_gtags_symbols "$index_dir" "$symbols_file"
    fi
}

check_tool_dependencies() {
    if [ "$INDEXING_TOOL" = "ctags" ]; then
        check_ctags_available
    else
        check_gtags_available
    fi
}

# ============================================================================
# Report Generation
# ============================================================================

generate_console_report() {
    local repo_root="$1"
    local index_dir="$2"
    local file_count="$3"

    echo
    echo "========================================"
    echo "WAMR Code Index Report"
    echo "========================================"

    # Status
    if [ "${STATS[indexing_success]:-no}" = "yes" ]; then
        echo "Status: ✅ SUCCESS"
    else
        echo "Status: ❌ FAILED"
    fi

    # Tool used
    echo "Tool: ${STATS[tool_used]:-unknown}"

    # Configuration
    echo "Target Directories: ${TARGET_DIRS[*]}"
    echo "Output Directory: $index_dir"

    # Counts
    echo "File Count: $file_count"
    if [ "$INDEXING_TOOL" = "ctags" ]; then
        echo "Function Count: ${STATS[total_symbols]:-0}"
    else
        echo "Symbol Count: ${STATS[total_symbols]:-0}"
    fi

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
WAMR Code Index Report
Generated: $(date)

Status: $([ "${STATS[indexing_success]:-no}" = "yes" ] && echo "SUCCESS" || echo "FAILED")
Tool: ${STATS[tool_used]:-unknown}
Target Directories: ${TARGET_DIRS[*]}
Output Directory: $index_dir
File Count: $file_count
$([ "$INDEXING_TOOL" = "ctags" ] && echo "Function Count:" || echo "Symbol Count:") ${STATS[total_symbols]:-0}
Processing Time: ${STATS[indexing_time]:-0} seconds
Exclusions: test/, tests/, stress-test/, coap/, lib-socket/, freertos/, debug/, platform/ subdirs (except common/include/linux)
EOF
}

# ============================================================================
# Main Function
# ============================================================================

main() {
    local repo_root="$1"
    local tool_arg="${2:-}"
    local start_time=$(date +%s)

    # Set indexing tool
    if [ "$tool_arg" = "--gtags" ]; then
        INDEXING_TOOL="gtags"
    elif [ "$tool_arg" = "--ctags" ] || [ -z "$tool_arg" ]; then
        INDEXING_TOOL="ctags"  # Default
    else
        log_error "Invalid tool option: $tool_arg"
        show_usage
    fi

    log_info "Using indexing tool: $INDEXING_TOOL"

    # Validate inputs
    if ! validate_repo_root "$repo_root"; then
        exit 1
    fi

    # Setup paths
    local index_dir="$repo_root/$INDEX_DIR_NAME"
    local file_list="$index_dir/files.list"
    local report_file="$index_dir/report.txt"
    local symbols_file="$index_dir/symbols.list"

    # Create directories
    mkdir -p "$index_dir"

    # Check dependencies
    if ! check_tool_dependencies; then
        exit 1
    fi

    # Count files
    local file_count
    file_count=$(discover_and_count_files "$repo_root")

    if [ "$file_count" -eq 0 ]; then
        log_error "No source files found"
        exit 1
    fi

    log_info "Found $file_count source files (excluding test/ and coap/ directories)"

    # Generate file list
    generate_file_list "$repo_root" "$file_list"

    # Run indexing
    if run_indexing "$repo_root" "$file_list" "$index_dir"; then
        log_success "Indexing completed successfully"
    else
        log_error "Indexing failed"
    fi

    # Collect statistics
    collect_symbol_statistics "$index_dir"

    # Dump symbols if indexing was successful
    if [ "${STATS[indexing_success]:-no}" = "yes" ]; then
        dump_all_symbols "$index_dir" "$symbols_file"
    fi

    # Calculate total time
    local end_time=$(date +%s)
    local total_time=$((end_time - start_time))
    STATS[total_time]="$total_time"

    # Generate reports
    generate_console_report "$repo_root" "$index_dir" "$file_count"
    generate_file_report "$repo_root" "$index_dir" "$file_count" "$report_file"

    echo "Report saved to: $report_file"

    if [ "${STATS[indexing_success]:-no}" = "yes" ]; then
        echo "Symbols list saved to: $symbols_file"
    fi

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
        # Run main function with repo root and optional tool selection
        main "$1" "${2:-}"
        ;;
esac