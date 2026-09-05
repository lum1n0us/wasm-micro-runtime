# WAMR Code Coverage

Parameterized code coverage measurement for WAMR, based on **GCC `--coverage`
(gcov data) + gcovr** (line / function / branch). The former lcov/genhtml
pipeline (`collect_coverage.sh`) has been replaced.

## Scope

Only the WAMR core sources are counted:

- **counted**: `core/iwasm`, `core/shared`
- **excluded**: `core/deps`, `tests/`, `samples/`, `product-mini/`,
  `wamr-compiler/` (its own sources), `test-tools/`

## Toolchain

- gcc/gcov: provided by `build-essential` in the devcontainer.
- gcovr: pinned in `.devcontainer/requirements.txt`
  (`pip install gcovr==6.0`).
- Prebuilt LLVM 18.1.8: compiled at devcontainer image build time by
  `build-scripts/build_llvm.py` into `/opt/llvm` and linked at
  `core/deps/llvm/build` (see `.devcontainer/setup-llvm.sh`) — required by
  the unit tests (`tests/unit` builds AOT/compilation sub-suites against
  `LLVM_DIR`). Pass `--llvm-dir core/deps/llvm/build/lib/cmake/llvm` when an
  explicit `LLVM_DIR` is needed.

## Scripts

| Script | Purpose |
|---|---|
| `tests/wamr-test-suites/run_coverage.py` | Parameterized entry: build + run spec/unit/regression for one or more report objects, collect gcovr reports, merge reports. |
| `tests/wamr-test-suites/spec-test-script/collect_gcovr.py` | gcovr collector: one or more build dirs → HTML + JSON + txt report (scope-filtered). Replaces `collect_coverage.sh`. |
| `tests/wamr-test-suites/coverage_analysis.py` | Analyze `coverage.json` → `coverage-gaps.md` (data-driven test backlog with suggested level). |
| `tests/regression/ba-issues/build_run_regression.py` | Build + run BA-issue regression tests (merged from `build_wamr.sh` + `run.py`); `--mode` filters cases and auto-derives runtimes; `--coverage` enables gcov data. |

## Report objects and fingerprints

A **report object** is one or more `(running mode, spec options, compilation
flags)` combinations. The report **fingerprint** is the normalized
serialization of its combinations:

```
fingerprint = running-modes + spec(s) + compilation flags
```

The same combination always yields the same fingerprint, so reports are
comparable across runs. The fingerprint is written to `fingerprint.txt` in
each report directory and shown in the HTML header.

Reports support merging:

- **same-report multi-test merge**: with `--unit` and/or `--regression`, the
  `.gcda` data of spec/unit/regression runs is merged into one report;
- **cross-report merge**: `--merge <report1> <report2> ...` merges several
  previously generated reports into `_merged/`.

## Usage

Run inside the devcontainer (repository root is auto-detected).

```bash
# single report: classic-interp + default spec + unit
python3 tests/wamr-test-suites/run_coverage.py \
    --report baseline --mode classic-interp --spec "-s spec -b" --unit \
    --llvm-dir core/deps/llvm/build/lib/cmake/llvm \
    --out build/coverage

# two independent reports in one run (different fingerprints)
python3 tests/wamr-test-suites/run_coverage.py \
    --report baseline --mode classic-interp --spec "-s spec -b" --unit \
    --report gc --mode classic-interp --spec "-s spec -G -b" \
    --llvm-dir core/deps/llvm/build/lib/cmake/llvm \
    --out build/coverage

# with regression tests merged in (per-mode filtering)
python3 tests/wamr-test-suites/run_coverage.py \
    --report baseline --mode classic-interp --spec "-s spec -b" \
    --unit --regression \
    --llvm-dir core/deps/llvm/build/lib/cmake/llvm \
    --out build/coverage

# merge two previously generated reports
python3 tests/wamr-test-suites/run_coverage.py \
    --merge baseline gc --out build/coverage
```

Additional `--cmake` flags parameterize the build:

```bash
python3 tests/wamr-test-suites/run_coverage.py \
    --report custom --mode classic-interp --spec "-s spec -b" \
    --cmake "-DWAMR_BUILD_LIBC_BUILTIN=1 -DWAMR_BUILD_SHARED_HEAP=1" \
    --unit --out build/coverage
```

## Collecting with test_wamr.sh (`-C`)

The `test_wamr.sh -C` flow now uses the gcovr collector:

```bash
cd tests/wamr-test-suites
./test_wamr.sh -s spec -b -C -t classic-interp
```

Reports land under `tests/wamr-test-suites/coverage-report/`
(`index.html`, `coverage.json`, `summary.txt`).

## Analyzing gaps

```bash
python3 tests/wamr-test-suites/coverage_analysis.py \
    --json build/coverage/baseline_*/coverage.json --threshold 60 \
    --out build/coverage/coverage-gaps.md
```

Each gap entry carries a suggested level (spec wast / unit cmocka /
regression) and a scenario, derived from the file path decision tree
(see the plan §3.6).

## Regression tests

```bash
cd tests/regression/ba-issues

# build + run all (runtimes auto-derived from running_config.json)
python3 build_run_regression.py

# only classic-interp cases (runtimes for that mode built automatically)
python3 build_run_regression.py --mode classic-interp

# build with gcov data for coverage collection
python3 build_run_regression.py --coverage

# CI-style split phases
python3 build_run_regression.py --build-only
python3 build_run_regression.py --run-only
```

## Unit test new cases: cmocka first

New unit cases should prefer **cmocka** (its stub/mock support fits C
projects better than GoogleTest). See `tests/unit/mem-alloc/` as the
template (`mem_alloc_test.c` + `test_runner.c` + `cmocka::cmocka` + ctest
registration + `-DWAMR_BUILD_TEST=1`). Existing gtest cases are kept as is.

Note: the unit sub-directories hard-code their own `WAMR_BUILD_*` feature
switches (CMake directory scope), so `--cmake` flags do **not** override unit
test configurations — unit coverage is the union of each sub-suite's own
configuration.
