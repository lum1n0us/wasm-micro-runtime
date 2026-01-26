#!/usr/bin/env python3
"""
ELF Symbol Comparison Tool

This script compares symbols from ELF files with a predefined symbol list.
It extracts symbols using objdump, filters out undefined symbols, and generates
comparison reports.

Usage:
    python compare_symbols.py --elfs file1.so file2.a --predefined symbols.list

Output Files:
    - symbols-in-elf.list: All symbols found in ELF files (intermediate)
    - unmatched-symbols.list: Predefined symbols not found in ELF files
    - match-percentage.txt: Match statistics and percentage

Console Output:
    - Match percentage
"""

import argparse
import subprocess
import sys
import os
import re
from typing import Set, List, Tuple, Optional


class SymbolExtractor:
    """Handles symbol extraction from ELF files using objdump."""

    def __init__(self):
        self.objdump_cmd = "objdump"

    def check_objdump_available(self) -> bool:
        """Check if objdump is available in the system."""
        try:
            subprocess.run(
                [self.objdump_cmd, "--version"], capture_output=True, check=True
            )
            return True
        except (subprocess.CalledProcessError, FileNotFoundError):
            return False

    def extract_symbols_from_elf(self, elf_path: str) -> Set[str]:
        """
        Extract symbols from a single ELF file using objdump.
        Filters out undefined symbols and extracts only symbol names.

        Args:
            elf_path: Path to the ELF file

        Returns:
            Set of symbol names found in the ELF file

        Raises:
            FileNotFoundError: If ELF file doesn't exist
            subprocess.CalledProcessError: If objdump fails
        """
        if not os.path.exists(elf_path):
            raise FileNotFoundError(f"ELF file not found: {elf_path}")

        try:
            # Run objdump -t to get symbol table
            result = subprocess.run(
                [self.objdump_cmd, "-t", elf_path],
                capture_output=True,
                text=True,
                check=True,
            )

            return self._parse_objdump_output(result.stdout, elf_path)

        except subprocess.CalledProcessError as e:
            raise subprocess.CalledProcessError(
                e.returncode, e.cmd, f"objdump failed for {elf_path}: {e.stderr}"
            )

    def _parse_objdump_output(self, objdump_output: str, elf_path: str) -> Set[str]:
        """
        Parse objdump output and extract defined symbol names.

        objdump -t output format:
        SYMBOL TABLE:
        00000000 l    df *ABS*  00000000 file.c
        00001000 g     F .text  00000010 function_name
        00000000      *UND*     00000000 undefined_symbol

        We filter out:
        - Lines that don't contain symbols
        - Undefined symbols (marked with *UND*)
        - File names and other metadata
        """
        symbols = set()

        # Skip header lines until we find SYMBOL TABLE:
        lines = objdump_output.strip().split("\n")
        symbol_section_found = False

        for line in lines:
            line = line.strip()

            # Look for symbol table section
            if "SYMBOL TABLE:" in line:
                symbol_section_found = True
                continue

            if not symbol_section_found:
                continue

            # Skip empty lines and headers
            if not line or line.startswith("SYMBOL TABLE:"):
                continue

            # Parse symbol line
            # Format: address flags section size symbol_name
            # Example: 00001000 g     F .text  00000010 my_function
            parts = line.split()

            if len(parts) < 4:
                continue

            # Check if this is an undefined symbol (*UND*)
            if len(parts) >= 3 and "*UND*" in parts[2]:
                continue

            # Extract symbol name (last part)
            symbol_name = parts[-1]

            # Skip special symbols and metadata
            if (
                symbol_name.startswith(".")
                or symbol_name in ["", "*ABS*", "*UND*"]
                or symbol_name.endswith(".c")
                or symbol_name.endswith(".o")
                or symbol_name.endswith(".cpp")
            ):
                continue

            # Handle symbol versioning (e.g., symbol@@VERSION)
            # Keep the full versioned name for exact matching
            symbol_name = symbol_name.strip()
            if symbol_name:
                symbols.add(symbol_name)

        return symbols

    def extract_symbols_from_multiple_elfs(
        self, elf_paths: List[str]
    ) -> Tuple[Set[str], dict]:
        """
        Extract symbols from multiple ELF files and merge them.

        Args:
            elf_paths: List of ELF file paths

        Returns:
            Tuple of (merged_symbols_set, per_file_stats)
        """
        all_symbols = set()
        per_file_stats = {}

        for elf_path in elf_paths:
            try:
                symbols = self.extract_symbols_from_elf(elf_path)
                all_symbols.update(symbols)
                per_file_stats[elf_path] = {
                    "symbol_count": len(symbols),
                    "status": "success",
                }
                print(f"Extracted {len(symbols)} symbols from {elf_path}")

            except Exception as e:
                per_file_stats[elf_path] = {
                    "symbol_count": 0,
                    "status": "error",
                    "error": str(e),
                }
                print(f"Error processing {elf_path}: {e}", file=sys.stderr)

        return all_symbols, per_file_stats


class SymbolComparator:
    """Handles symbol comparison and report generation."""

    def load_predefined_symbols(self, predefined_path: str) -> Set[str]:
        """
        Load predefined symbols from file (one symbol per line).

        Args:
            predefined_path: Path to predefined symbol list file

        Returns:
            Set of predefined symbol names
        """
        if not os.path.exists(predefined_path):
            raise FileNotFoundError(
                f"Predefined symbol file not found: {predefined_path}"
            )

        symbols = set()
        try:
            with open(predefined_path, "r", encoding="utf-8") as f:
                for line in f:
                    symbol = line.strip()
                    if symbol:  # Skip empty lines
                        symbols.add(symbol)
        except Exception as e:
            raise Exception(f"Error reading predefined symbols: {e}")

        return symbols

    def compare_symbols(
        self, elf_symbols: Set[str], predefined_symbols: Set[str]
    ) -> dict:
        """
        Compare ELF symbols with predefined symbols.

        Args:
            elf_symbols: Set of symbols from ELF files
            predefined_symbols: Set of predefined symbols

        Returns:
            Dictionary with comparison results
        """
        matched_symbols = predefined_symbols.intersection(elf_symbols)
        unmatched_symbols = predefined_symbols.difference(elf_symbols)

        total_predefined = len(predefined_symbols)
        matched_count = len(matched_symbols)

        match_percentage = (
            (matched_count / total_predefined * 100) if total_predefined > 0 else 0
        )

        return {
            "total_predefined": total_predefined,
            "total_elf_symbols": len(elf_symbols),
            "matched_count": matched_count,
            "unmatched_count": len(unmatched_symbols),
            "match_percentage": match_percentage,
            "matched_symbols": matched_symbols,
            "unmatched_symbols": unmatched_symbols,
        }

    def write_output_files(
        self,
        elf_symbols: Set[str],
        comparison_results: dict,
        elf_files_processed: int,
        per_file_stats: dict,
    ):
        """Write all output files."""

        # 1. Write symbols-in-elf.list (intermediate file)
        with open("symbols-in-elf.list", "w", encoding="utf-8") as f:
            for symbol in sorted(elf_symbols):
                f.write(f"{symbol}\n")
        print(f"Written {len(elf_symbols)} symbols to symbols-in-elf.list")

        # 2. Write unmatched-symbols.list
        unmatched_symbols = comparison_results["unmatched_symbols"]
        with open("unmatched-symbols.list", "w", encoding="utf-8") as f:
            for symbol in sorted(unmatched_symbols):
                f.write(f"{symbol}\n")
        print(
            f"Written {len(unmatched_symbols)} unmatched symbols to unmatched-symbols.list"
        )

        # 3. Write match-percentage.txt
        with open("match-percentage.txt", "w", encoding="utf-8") as f:
            f.write(
                f"Match Percentage: {comparison_results['match_percentage']:.2f}%\n"
            )
            f.write(
                f"Total Predefined Symbols: {comparison_results['total_predefined']}\n"
            )
            f.write(f"Matched Symbols: {comparison_results['matched_count']}\n")
            f.write(f"Unmatched Symbols: {comparison_results['unmatched_count']}\n")
            f.write(f"ELF Files Processed: {elf_files_processed}\n")
            f.write(
                f"Total ELF Symbols Found: {comparison_results['total_elf_symbols']}\n"
            )

            # Add per-file statistics
            f.write(f"\nPer-File Statistics:\n")
            for elf_path, stats in per_file_stats.items():
                if stats["status"] == "success":
                    f.write(f"  {elf_path}: {stats['symbol_count']} symbols\n")
                else:
                    f.write(
                        f"  {elf_path}: ERROR - {stats.get('error', 'Unknown error')}\n"
                    )

        print(f"Written match statistics to match-percentage.txt")


def main():
    """Main function to handle command-line interface and orchestrate the comparison."""

    parser = argparse.ArgumentParser(
        description="Compare symbols from ELF files with predefined symbol list",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python compare_symbols.py --elfs libwamr.so --predefined symbols.list
  python compare_symbols.py --elfs lib1.so lib2.a lib3.so --predefined gtags-indexes/symbols.list

Output Files:
  - symbols-in-elf.list: All symbols extracted from ELF files
  - unmatched-symbols.list: Predefined symbols not found in ELF files  
  - match-percentage.txt: Detailed match statistics
        """,
    )

    parser.add_argument(
        "--elfs", nargs="+", required=True, help="One or more ELF file paths to analyze"
    )

    parser.add_argument(
        "--predefined",
        required=True,
        help="Path to predefined symbol list file (one symbol per line)",
    )

    args = parser.parse_args()

    # Initialize components
    extractor = SymbolExtractor()
    comparator = SymbolComparator()

    try:
        # Check if objdump is available
        if not extractor.check_objdump_available():
            print("Error: objdump is not available in the system PATH", file=sys.stderr)
            print(
                "Please install binutils or ensure objdump is accessible",
                file=sys.stderr,
            )
            sys.exit(1)

        print(f"Processing {len(args.elfs)} ELF file(s)...")

        # Extract symbols from ELF files
        elf_symbols, per_file_stats = extractor.extract_symbols_from_multiple_elfs(
            args.elfs
        )

        if not elf_symbols:
            print("Warning: No symbols extracted from ELF files", file=sys.stderr)

        print(f"Total unique symbols extracted from ELF files: {len(elf_symbols)}")

        # Load predefined symbols
        print(f"Loading predefined symbols from {args.predefined}...")
        predefined_symbols = comparator.load_predefined_symbols(args.predefined)
        print(f"Loaded {len(predefined_symbols)} predefined symbols")

        # Compare symbols
        comparison_results = comparator.compare_symbols(elf_symbols, predefined_symbols)

        # Write output files
        successful_files = sum(
            1 for stats in per_file_stats.values() if stats["status"] == "success"
        )

        comparator.write_output_files(
            elf_symbols, comparison_results, successful_files, per_file_stats
        )

        # Print console output (match percentage)
        match_percentage = comparison_results["match_percentage"]
        print(f"\nMatch percentage: {match_percentage:.2f}%")

        # Print summary
        print(f"\nSummary:")
        print(f"  ELF files processed: {successful_files}/{len(args.elfs)}")
        print(f"  Total ELF symbols: {comparison_results['total_elf_symbols']}")
        print(f"  Predefined symbols: {comparison_results['total_predefined']}")
        print(f"  Matched: {comparison_results['matched_count']}")
        print(f"  Unmatched: {comparison_results['unmatched_count']}")

    except KeyboardInterrupt:
        print("\nOperation cancelled by user", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
