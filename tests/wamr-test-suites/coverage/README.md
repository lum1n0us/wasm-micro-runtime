# WAMR Code Coverage

Parameterized code coverage measurement for WAMR, based on **GCC `--coverage`
(gcov data) + gcovr** (line / function / branch). The former lcov/genhtml
pipeline (`collect_coverage.sh`) has been replaced.

Everything coverage-related lives in this directory
(`tests/wamr-test-suites/coverage/`): the runner, the presets-free feature-set
helpers, the compatibility filter and the gcovr collector.

## Scope

Only the WAMR core sources are counted:

- **counted**: `core/iwasm`, `core/shared`
- **excluded**: `core/deps`, `tests/`, `samples/`, `product-mini/`,
  `wamr-compiler/` (its own sources), `test-tools/`

Regression tests (`tests/regression/ba-issues`) are **not** part of the
coverage scope; see [Regression tests](#regression-tests).

## Toolchain

- gcc/gcov: the system toolchain (`build-essential` on Debian/Ubuntu).
- gcovr 6.0: `python3 -m gcovr` must work for the interpreter that runs the
  collector (`pip install gcovr==6.0`).
- LLVM 18.1.8: built by `build-scripts/build_llvm.py` and linked at
  `core/deps/llvm/build` — required by the unit tests (`tests/unit` builds
  AOT/compilation sub-suites against `LLVM_DIR`). `--llvm-dir` is optional and
  already defaults to that bundled build
  (`core/deps/llvm/build/lib/cmake/llvm`); pass it only for a custom LLVM.

## Scripts

| Script | Purpose |
|---|---|
| `coverage/run_coverage.py` | Parameterized entry: build + run spec/unit for one or more report objects, collect gcovr reports, merge reports. |
| `coverage/run_classic_fset.py` | The canned classic-interp feature-set report object: that mode + a fixed feature set + unit. |
| `coverage/run_full.py` | Full run: every spec variant plus the unit suites of the supported modes, merged into `_merged/`. |
| `coverage/collect_coverage_gcovr.py` | gcovr collector: one or more build dirs → HTML + JSON + txt report (scope-filtered). Replaces `collect_coverage.sh`. |
| `tests/regression/ba-issues/build_run.py` | Build + run the BA-issue regression tests (merged from `build_wamr.sh` + `run.py`); `--mode` filters cases and auto-derives runtimes, `--coverage` enables gcov data. Quality gate, not part of the coverage scope. |

## Pipeline: orchestrator and collector

The toolchain is layered. `run_coverage.py` is the **orchestrator**: it decides
what runs and which build directories count. `collect_coverage_gcovr.py` is the
**collector**: it knows nothing but "a list of build dirs plus one output dir"
and turns that into gcovr reports. The two never import each other — the
orchestrator shells out.

| Layer | `run_coverage.py` (orchestrator) | `collect_coverage_gcovr.py` (collector) |
|---|---|---|
| Knows about | report objects (`mode` × spec options × feature set), fingerprints, the `_work/` dirs, unit-target compatibility | a list of build dirs and one output dir |
| Does | runs the spec suite via `test_wamr.sh`, configures/builds/runs the unit suites, filters the unit targets with `compile_commands.json`, invokes the collector, writes `fingerprint.txt` and `unmatched.txt`, implements `--merge` | runs gcovr three times (`--html-details`, `--json`, `--txt`), pins the statistics scope to `core/iwasm` + `core/shared`, and applies the gcov compatibility switches |
| Knows nothing about | gcov/gcovr mechanics | running modes, feature sets, report objects, fingerprints |

Call graph (`test_wamr.sh` invokes the collector directly as well — it has two
call sites, for its standalone and unit/regression flows):

```
run_coverage.py  (orchestrator)
 ├── bash test_wamr.sh -s spec -b -t <mode> -C          # run the spec suite
 │     ├── builds iwasm in place -> product-mini/platforms/<platform>/build
 │     └── -C: python3 collect_coverage_gcovr.py --out <test-suites>/coverage-report <build dir>
 ├── cmake + ctest on tests/unit (per running mode)      # run the unit suites
 └── python3 collect_coverage_gcovr.py --out <report> <spec copy> <unit target dirs...>

test_wamr.sh -C   (used standalone, or via -s unit / -s regression)
 └── python3 collect_coverage_gcovr.py --out <test-suites>/coverage-report <build dirs...>
```

### Why there are two collection passes

A single `run_coverage.py` report triggers gcovr **twice** over the same spec
`.gcda` data, on purpose:

1. **`test_wamr.sh -C`** collects the product build dir on its own into
   `tests/wamr-test-suites/coverage-report/`. That report belongs to
   `test_wamr.sh`'s `-C` contract (the same path serves `-s unit` and
   `-s regression`) and covers the spec build only.
2. **`run_coverage.py`** then collects again, in one gcovr invocation, from the
   spec build **plus** every unit target directory that is compatible with the
   report's feature set F — that merged result is the actual report.

Pass 2 needs its own copy of the spec data because `test_wamr.sh` builds
`product-mini/platforms/<platform>/build` in place and `run_spec()` wipes it
before every run, so the `.gcda` of a finished spec variant is gone as soon as
the next one starts. Right after each spec run, `run_spec()` therefore copies
that directory's `.gcno`/`.gcda` into
`<out>/_work/<report>/spec-coverage-<mode>/`, which is what pass 2 collects
from (and what makes a later `--merge` able to see every spec variant).

## Report objects and fingerprints

A **report object** is one or more `(running mode, spec options, feature set)`
combinations. The report **fingerprint** is the normalized serialization of
its combinations:

```
fingerprint = running-modes + spec options + feature set
```

The same combination always yields the same fingerprint, so reports are
comparable across runs. The fingerprint is written to `fingerprint.txt` in
each report directory and shown in the HTML header.

Reports support merging:

- **same-report multi-test merge**: with `--unit`, the `.gcda` data of the
  spec and unit runs is merged into one report;
- **cross-report merge**: repeated `--merge <report>` merges previously
  generated reports into `_merged/`. Each report's `_work/<name>/` directory
  keeps the build dirs its data was collected from (the unit build dirs, and
  for the spec layer a copy of the `.gcno`/`.gcda` of `test_wamr.sh`'s
  in-place product build, which is what lets every spec variant survive into
  the merge), so the merge re-collects the union of those directories.

## Usage

Run from anywhere in the repository (the repository root is auto-detected).

```bash
# single report: classic-interp + a curated feature set + spec + unit
python3 tests/wamr-test-suites/coverage/run_coverage.py \
    --report classic-fset --mode classic-interp \
    --feature "-DWAMR_BUILD_LIBC_BUILTIN=1 -DWAMR_BUILD_SHARED_HEAP=1 \
               -DWAMR_BUILD_GLOBAL_HEAP_POOL=1 -DWAMR_BUILD_SPEC_TEST=1 \
               -DWAMR_BUILD_BULK_MEMORY=1 -DWAMR_BUILD_REF_TYPES=1" \
    --unit --out build/coverage

# spec variant: test_wamr.sh -s spec -b -t classic-interp -C -G
python3 tests/wamr-test-suites/coverage/run_coverage.py \
    --report gc --mode classic-interp --spec "-G" --out build/coverage

# no feature constraint: every unit suite keeps its own defaults
python3 tests/wamr-test-suites/coverage/run_coverage.py \
    --report ci --mode classic-interp --unit --full-test \
    --out build/coverage

# the canned classic-interp feature-set report, and the full matrix
python3 tests/wamr-test-suites/coverage/run_classic_fset.py --out build/coverage
python3 tests/wamr-test-suites/coverage/run_full.py --out build/coverage

# merge two previously generated reports
python3 tests/wamr-test-suites/coverage/run_coverage.py \
    --merge classic-fset --merge gc --out build/coverage
```

`run_classic_fset.py` and `run_full.py` are fixed pipelines: they take only
`--out` and `--llvm-dir`, and always run everything they describe (the one
canned report object; every spec variant + every unit suite + the merge).

`--spec` only carries the extra `test_wamr.sh` switches — `-s spec` (spec
suite) and `-b` (use the wabt binary release instead of compiling wabt) are
always passed. The switches used by `run_full.py`:

| Variant | `--spec` |
|---|---|
| default | (none) |
| gc | `-G` |
| exception-handling | `-e` |
| extended-const | `-N` |
| memory64 | `-W` |
| multi-memory | `-E` |
| threads | `-p` |

## Feature set F

`--feature` takes the report's feature set as plain cmake switches:

```bash
--feature "-DWAMR_BUILD_GC=1 -DWAMR_BUILD_SPEC_TEST=1 -DWAMR_BUILD_BULK_MEMORY=1"
```

`expand_features()` (`coverage/coverage_features.py`) completes it before use:
the listed features become 1, every other feature known to the build (the
`ALL_FEATURES` checklist) becomes 0, cmake's implied features are added until a
fixpoint (GC → REF_TYPES, JIT → INTERP, ...), and the running-mode features
follow `--mode`. Unknown feature names are rejected — a typo would otherwise
silently weaken the checks. Omitting `--feature` is the one exception: no
feature is constrained, only the mode features are fixed, and each unit suite
keeps the values its own `CMakeLists.txt` declares.

F decides which unit targets belong to the report: every target in
`compile_commands.json` is checked against it (rule 1: F=1 features must be
enabled; rule 2: F=0 features must not be), the compatible ones are collected
and the rest are listed in `unmatched.txt`.

F is **not** injected into any configure — writing `-DWAMR_BUILD_GC=1` here
does **not** turn GC on for a build. The unit suites hard-code their own
`WAMR_BUILD_*` switches (CMake directory scope), so unit coverage is the union
of each sub-suite's own configuration, and the spec layer is configured by
`test_wamr.sh` itself (running-mode flags plus the `--spec` switches). F is
therefore a statement about *what the report covers*, and the unit targets that
do not match it are excluded from the report's denominator rather than
reconfigured.

## Collecting with test_wamr.sh (`-C`)

The `test_wamr.sh -C` flow uses the gcovr collector:

```bash
cd tests/wamr-test-suites
./test_wamr.sh -s spec -b -C -t classic-interp
```

Reports land under `tests/wamr-test-suites/coverage-report/`
(`index.html`, `coverage.json`, `summary.txt`).

## Regression tests

The BA-issue regression tests are a quality gate, not part of the coverage
report. Run them directly (see `tests/regression/ba-issues/README.md`):

```bash
cd tests/regression/ba-issues

# build + run every active case (runtimes auto-derived from running_config.json)
python3 build_run.py

# only classic-interp cases
python3 build_run.py --mode classic-interp

# build with gcov data (-DCOLLECT_CODE_COVERAGE=1)
python3 build_run.py --coverage
```

`--llvm-dir` is optional there too (defaults to the bundled LLVM build). To
turn their `.gcda` into a report, point the collector at the runtime build
directories:

```bash
python3 tests/wamr-test-suites/coverage/collect_coverage_gcovr.py \
    --out build/regression-coverage \
    tests/regression/ba-issues/build/build-iwasm-*
```

## Unit test new cases: cmocka first

New unit cases should prefer **cmocka** (its stub/mock support fits C
projects better than GoogleTest). See `tests/unit/mem-alloc/` as the
template (`mem_alloc_test.c` + `test_runner.c` + `cmocka::cmocka` + ctest
registration + `-DWAMR_BUILD_TEST=1`). Existing gtest cases are kept as is.

Note: the unit sub-directories hard-code their own `WAMR_BUILD_*` feature
switches (CMake directory scope), so `--feature` does **not** override unit
test configurations — unit coverage is the union of each sub-suite's own
configuration.
