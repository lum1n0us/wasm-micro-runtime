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
TIMEOUT_SECONDS = 30

# board identifier per simulator
BOARDS = {
    "native_sim": "native_sim",
    "qemu_arc": "qemu_arc/qemu_arc_hs",
}

# strings expected in the output of every sample
EXPECTED = ["Hello world!", "elapsed"]

EPILOG = f"""\
output:
  The console only shows progress and the final result. The full output of
  `docker build`, the CMake configuration, the compilation and the emulator run
  is written to:

    build/logs/docker-build.log     the Docker image build
    build/logs/<sample>-<sim>.log   one per sample/simulator combination

  On failure the log path and the tail of that log are printed.
"""


def tail(log_path, lines=15):
    content = log_path.read_text(errors="replace").splitlines()
    return "\n".join(f"  | {line}" for line in content[-lines:])


def run_logged(argv, log_path, step):
    """Run argv, appending its output to log_path. Return True on success."""
    print(f"--> {step} ... ", end="", flush=True)
    with log_path.open("a") as log:
        log.write(f"\n$ {' '.join(argv)}\n")
        log.flush()
        completed = subprocess.run(argv, stdout=log, stderr=subprocess.STDOUT)

    if completed.returncode == 0:
        print("ok")
        return True

    print("FAILED")
    print(f"    log: {log_path}")
    print(tail(log_path))
    return False


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
    board = BOARDS[simulator]
    log_path = LOG_DIR / f"{sample}-{simulator}.log"
    log_path.unlink(missing_ok=True)

    def in_container(command, step):
        return run_logged(
            [
                "docker",
                "run",
                "--rm",
                "-v",
                # native path on the host side, posix path on the container side
                f"{WAMR_ROOT}:{MODULE_DIR}",
                "-w",
                f"{MODULE_DIR}/product-mini/platforms/zephyr/{sample}",
                IMAGE,
                "bash",
                "-o",
                "errexit",
                "-o",
                "nounset",
                "-o",
                "errtrace",
                "-o",
                "pipefail",
                "-c",
                command,
            ],
            log_path,
            step,
        )

    expected = " ".join(f"'{string}'" for string in EXPECTED)
    return in_container(
        f"west build . -b {board} -p always -- -DEXTRA_ZEPHYR_MODULES={MODULE_DIR}",
        f"{sample} on {simulator} ({board}): build",
    ) and in_container(
        f"{MODULE_DIR}/.github/scripts/run_qemu_and_verify.sh"
        f" $PWD/build {TIMEOUT_SECONDS} {expected}",
        f"{sample} on {simulator} ({board}): run and verify",
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
