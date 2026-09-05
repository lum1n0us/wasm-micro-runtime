#!/usr/bin/env python3

#
# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#

"""Analyze a gcovr JSON coverage report and produce a data-driven list of
coverage gaps with a recommended test level (spec wast / unit cmocka /
regression) per the decision tree in the coverage plan (§3.6).

Reads coverage.json (produced by collect_gcovr.py) and writes
coverage-gaps.md:

  * files with low line coverage (below --threshold, default 60%)
  * uncovered functions per such file
  * a suggested test level + scenario for each gap, derived from the file's
    path (decision tree):
      - core/iwasm/{interpreter,aot,loader,compilation}/  -> spec wast
        (loader error paths -> malformed)
      - core/iwasm/common/, core/shared/{utils,mem-alloc,platform}/ -> unit cmocka
      - otherwise                                            -> regression

Usage:
  python3 coverage_analysis.py --json <coverage.json> [--threshold 60] \
      [--out coverage-gaps.md]
"""

import argparse
import json
import os

DECISION_TREE = [
    # (regex on file path, recommended level, scenario template)
    (r"core/iwasm/(interpreter|aot|compilation)/",
     "spec wast",
     "指令/执行路径：找官方 spec 套件同指令 .wast 作模板，构造最小用例（目标指令 + 边界操作数）→ runtest.py 验证"),
    (r"core/iwasm/loader/",
     "spec wast / malformed",
     "加载校验路径：正常路径用 spec wast；错误校验分支用 malformed 用例（tests/malformed 模式）"),
    (r"core/iwasm/common/",
     "unit cmocka",
     "内部 API/错误处理：读函数签名，构造正常路径 + 每个错误分支（NULL 参数、非法枚举、越界、分配失败用 will_return/__wrap_* 注入）"),
    (r"core/shared/utils/",
     "unit cmocka",
     "数据结构（bh_* 容器）：边界条件 + 错误分支，参考 tests/unit/mem-alloc 模板"),
    (r"core/shared/mem-alloc/",
     "unit cmocka",
     "分配器：正常分配 + 对齐 + OOM 注入（参考 tests/unit/mem-alloc 现有用例扩展）"),
    (r"core/shared/platform/",
     "unit cmocka",
     "平台抽象：正常 + 错误路径，stub/mock 平台调用"),
]


def suggest(file_path):
    """Apply the decision tree; return (level, scenario)."""
    for pattern, level, scenario in DECISION_TREE:
        import re
        if re.search(pattern, file_path):
            return level, scenario
    return "regression", "历史 bug 修复路径：按 tests/regression/ba-issues README 加 issue 用例"


def analyze(data, threshold):
    gaps = []  # (file, line_pct, uncovered_funcs)
    for file_entry in data.get("files", []):
        file_path = file_entry.get("file", "")
        # Scope: only core/iwasm and core/shared (core/deps excluded).
        if not (file_path.startswith("core/iwasm/")
                or file_path.startswith("core/shared/")):
            continue
        lines = file_entry.get("lines", [])
        if isinstance(lines, dict):  # gcovr < 6.0
            lines_total = lines.get("count", 0)
            lines_hit = lines.get("hit", 0)
        else:  # gcovr >= 6.0: list of line entries
            lines_total = len(lines)
            lines_hit = sum(1 for ln in lines if ln.get("count", 0) > 0)
        if lines_total == 0:
            continue
        pct = 100.0 * lines_hit / lines_total
        if pct >= threshold:
            continue
        uncovered = []
        for fn in file_entry.get("functions", []):
            if isinstance(fn, dict):
                # gcovr >= 6.0: execution_count == 0 means never executed
                if fn.get("execution_count", 0) == 0:
                    uncovered.append(fn.get("name", "?"))
            elif fn.get("count", 0) == 0:  # gcovr < 6.0
                uncovered.append(fn.get("name", "?"))
        gaps.append((file_path, pct, uncovered))

    gaps.sort(key=lambda g: g[1])
    return gaps


def render_markdown(gaps, threshold):
    lines = [
        "# Coverage Gaps",
        "",
        f"Files with line coverage below {threshold}% (from gcovr JSON).",
        f"Total: {len(gaps)} files.",
        "",
        "| 文件 | 行覆盖 % | 建议级别 | 场景设计 |",
        "|---|---|---|---|",
    ]
    for file_path, pct, uncovered in gaps:
        level, scenario = suggest(file_path)
        fn_list = ", ".join(uncovered[:5]) + ("..." if len(uncovered) > 5 else "")
        lines.append(
            f"| `{file_path}` | {pct:.1f} | {level} | {scenario}"
            + (f"<br>未覆盖函数: {fn_list}" if fn_list else "")
            + " |"
        )
    lines.append("")
    lines.append("## 验证闭环")
    lines.append("每项补完后重跑对应报告对象，diff 前后 coverage.json 确认目标行/分支从'未覆盖'变'已覆盖'。")
    lines.append("")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(
        description="Analyze gcovr JSON and list coverage gaps with suggested test level."
    )
    parser.add_argument("--json", required=True, help="Path to coverage.json")
    parser.add_argument("--threshold", type=float, default=60.0,
                        help="Line coverage threshold (percent); files below are listed.")
    parser.add_argument("--out", default="coverage-gaps.md",
                        help="Output markdown file.")
    args = parser.parse_args()

    with open(args.json) as f:
        data = json.load(f)

    gaps = analyze(data, args.threshold)
    markdown = render_markdown(gaps, args.threshold)

    with open(args.out, "w") as f:
        f.write(markdown)
    print(f"Wrote {len(gaps)} gaps to {args.out}")


if __name__ == "__main__":
    main()
