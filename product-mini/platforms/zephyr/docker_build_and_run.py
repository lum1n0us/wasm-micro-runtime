#!/usr/bin/env python3
#
# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""Build the Zephyr development image and build/run a WAMR sample inside a
container, against the local checkout."""

import argparse
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
WAMR_ROOT = HERE.parents[2]
LOG_DIR = HERE / "build" / "logs"
IMAGE = "wamr-zephyr"
MODULE_DIR = "/root/zephyrproject/modules/wasm-micro-runtime"
ZEPHYR_PLATFORM_DIR = f"{MODULE_DIR}/product-mini/platforms/zephyr"
TIMEOUT_SECONDS = 30

# The samples report a failure by printing a line carrying this marker
ERROR_MARKER = "ERROR:"

# board identifier and WAMR build target per simulator. The target is derived
# from the board by the Zephyr module, but samples that build the runtime
# themselves (user-mode) still need it on the command line, just like in CI.
BOARDS = {
    "native_sim": ("native_sim", "X86_32"),
    "qemu_arc": ("qemu_arc/qemu_arc_hs", "ARC"),
}

EPILOG = f"""\
output:
  The Docker image build, the CMake configuration and the compilation only show
  progress on the console; their full output is written to:

    build/logs/docker-build.log     the Docker image build
    build/logs/<sample>-<sim>.log   one per sample/simulator combination

  The west build directory, including zephyr.exe / zephyr.elf, is
  build/<sample>-<sim>/. Everything under build/ is created by the container
  and therefore owned by root; -p always is passed so stale trees are rebuilt.

  The output of the sample itself goes to both the console and the log. A run
  counts as successful when the process exits with an expected status and the
  output carries no "ERROR:" line -- by convention the samples report every
  failure that way, since a Zephyr application cannot return a status to the
  host. Emulators do not stop on their own, so the run is killed after
  {TIMEOUT_SECONDS}s, which is an expected status.

  On failure the log path and the tail of that log are printed.
"""


def tail(log_path, lines=15):
    content = log_path.read_text(errors="replace").splitlines()
    return "\n".join(f"  | {line}" for line in content[-lines:])


def report(succeeded, step, log_path):
    print("    ok" if succeeded else "    FAILED")
    if not succeeded:
        print(f"    log: {log_path}")
    return succeeded


def run_logged(argv, log_path, step):
    """Run argv, sending its output to log_path only."""
    print(f"--> {step} ...", flush=True)
    with log_path.open("a") as log:
        log.write(f"\n$ {' '.join(argv)}\n")
        log.flush()
        completed = subprocess.run(argv, stdout=log, stderr=subprocess.STDOUT)

    return report(completed.returncode == 0, step, log_path)


def run_streamed(argv, log_path, step, ok_returncodes):
    """Run argv, echoing its output to both the console and log_path. Fails on
    an unexpected exit status or on an ERROR: line reported by the sample."""
    print(f"--> {step} ...", flush=True)
    reported_errors = []
    with log_path.open("a") as log:
        log.write(f"\n$ {' '.join(argv)}\n")
        process = subprocess.Popen(
            argv, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True
        )
        for line in process.stdout:
            print(f"  | {line}", end="", flush=True)
            log.write(line)
            if ERROR_MARKER in line:
                reported_errors.append(line.strip())
        returncode = process.wait()

    for error in reported_errors:
        print(f"    reported: {error}")

    succeeded = returncode in ok_returncodes and not reported_errors
    return report(succeeded, step, log_path)


def build_image():
    log_path = LOG_DIR / "docker-build.log"
    log_path.unlink(missing_ok=True)
    return run_logged(
        ["docker", "build", "-t", IMAGE, str(HERE)],
        log_path,
        f"building Docker image {IMAGE} (this takes a while)",
    )


def image_exists():
    return (
        subprocess.run(
            ["docker", "image", "inspect", IMAGE],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        ).returncode
        == 0
    )


def run_sample(sample, simulator):
    """Build and run one sample on one simulator. Return True on success."""
    board, target = BOARDS[simulator]
    log_path = LOG_DIR / f"{sample}-{simulator}.log"
    log_path.unlink(missing_ok=True)

    def docker_run(command):
        return [
            "docker",
            "run",
            "--rm",
            "-v",
            # native path on the host side, posix path on the container side
            f"{WAMR_ROOT}:{MODULE_DIR}",
            "-w",
            f"{ZEPHYR_PLATFORM_DIR}/{sample}",
            IMAGE,
            "bash",
            "-euo",
            "pipefail",
            "-c",
            command,
        ]

    # keep every build tree, and the logs, under this directory
    build_dir = f"{ZEPHYR_PLATFORM_DIR}/build/{sample}-{simulator}"

    built = run_logged(
        docker_run(
            f"west build . -b {board} -p always -d {build_dir}"
            f" -- -DEXTRA_ZEPHYR_MODULES={MODULE_DIR}"
            f" -DWAMR_BUILD_TARGET={target}"
        ),
        log_path,
        f"{sample} on {simulator} ({board}): build",
    )
    if not built:
        return False

    # The emulator keeps running once the sample is done, so it is killed by
    # `timeout`, which reports 124.
    return run_streamed(
        docker_run(f"timeout {TIMEOUT_SECONDS}s west build -d {build_dir} -t run"),
        log_path,
        f"{sample} on {simulator} ({board}): run",
        ok_returncodes={0, 124},
    )


def main():
    parser = argparse.ArgumentParser(
        description=__doc__,
        epilog=EPILOG,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "sample",
        nargs="?",
        default="simple",
        help="sample directory name (default: simple)",
    )
    parser.add_argument(
        "--build",
        action="store_true",
        help=f"build the {IMAGE} Docker image and exit",
    )
    parser.add_argument(
        "--sim",
        choices=sorted(BOARDS),
        action="append",
        dest="simulators",
        help="simulator to run on, repeatable (default: native_sim)",
    )
    args = parser.parse_args()

    LOG_DIR.mkdir(parents=True, exist_ok=True)

    if args.build:
        return 0 if build_image() else 1

    # accept "simple", "./simple" and "simple/" alike
    sample = Path(args.sample).name
    if not (HERE / sample / "CMakeLists.txt").is_file():
        parser.error(f"unknown sample: {args.sample}")

    if not image_exists() and not build_image():
        return 1

    for simulator in args.simulators or ["native_sim"]:
        if not run_sample(sample, simulator):
            return 1

    print("all done")
    return 0


if __name__ == "__main__":
    sys.exit(main())
