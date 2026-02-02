#!/bin/bash

# How to use this script:
#   0. use .devcontainer/coverity/ to setup the Coverity tools environment. This is an enterprise version provided by Intel
#   1. assume you have the right permission to commit to the Coverity server ${COVERITY_SERVER_URL}
#   2. after login to the server, you have created/seen a project named ${PROJECT_NAME}
#   3. you have the auth key file at ${AUTH_KEY}
#   4. Run `python3 ci/build_runtime_comprehensive.py --mode coverity --output-dir <build-for-coverity-scan>` to generate the cov-build output directory
#   5. If everything is ready, run this script:
#        $ ci/prepare_build_for_coverity_scan_intel_bac.sh <build-for-coverity-scan>/cov-int


set -o pipefail
set -o errexit
set -o nounset
set -o errtrace
# DEBUG=1 to enable xtrace
if [ -n "${DEBUG:-}" ]; then
  set -o xtrace
fi

AUTH_KEY=./.devcontainer/coverity/auth-key.txt
COVERITY_SERVER_URL="https://coverity.devtools.intel.com/prod21"
PROJECT_NAME="Wamr-v2"

usage() {
  echo "Usage: $0 [--repo-root <path>] <cov-build-output-directory-path>"
  echo "  --repo-root: path to repository root (default: current directory)"
}

# there should be a directory created by cov-build. This is the input of this script
REPO_ROOT="$(pwd)"
OUTPUT_COV_EMIT=""
while [ $# -gt 0 ]; do
  case "$1" in
    --repo-root)
      if [ $# -lt 2 ]; then
        echo "Error: --repo-root requires a path argument"
        usage
        exit 2
      fi
      REPO_ROOT="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    --)
      shift
      break
      ;;
    -*)
      echo "Error: unknown option: $1"
      usage
      exit 2
      ;;
    *)
      if [ -z "${OUTPUT_COV_EMIT}" ]; then
        OUTPUT_COV_EMIT="$1"
      else
        echo "Error: unexpected argument: $1"
        usage
        exit 2
      fi
      shift
      ;;
  esac
done

if [ -z "${OUTPUT_COV_EMIT}" ]; then
  echo "Error: missing cov-build output directory"
  usage
  exit 2
fi

if [ ! -d "${REPO_ROOT}" ]; then
  echo "Error: repo root directory not found: ${REPO_ROOT}"
  exit 2
fi

REPO_ROOT="$(cd "${REPO_ROOT}" && pwd)"
AUTH_KEY="${REPO_ROOT}/.devcontainer/coverity/auth-key.txt"
BUILD_LOG="${OUTPUT_COV_EMIT}/build-log.txt"

trap 'status=$?; echo "Error: failed with status ${status}"; echo "  line: ${LINENO}"; echo "  cmd: ${BASH_COMMAND}"; echo "  repo_root: ${REPO_ROOT}"; echo "  cov_emit: ${OUTPUT_COV_EMIT}"; echo "  build_log: ${BUILD_LOG}"; echo "  auth_key: ${AUTH_KEY}"; exit ${status}' ERR

if ! command -v cov-analyze >/dev/null 2>&1; then
  echo "Error: cov-analyze not found in PATH"
  exit 127
fi
if ! command -v cov-commit-defects >/dev/null 2>&1; then
  echo "Error: cov-commit-defects not found in PATH"
  exit 127
fi
if [ ! -d "${OUTPUT_COV_EMIT}" ]; then
  echo "Error: cov-build output directory not found: ${OUTPUT_COV_EMIT}"
  exit 2
fi
if [ ! -f "${BUILD_LOG}" ]; then
  echo "Error: build log not found: ${BUILD_LOG}"
  exit 2
fi
if [ ! -f "${AUTH_KEY}" ]; then
  echo "Error: auth key not found: ${AUTH_KEY}"
  exit 2
fi

# grep "compilation units (100%) successfully" ${OUTPUT_COV_EMIT}/build-log.txt and check the return code and summaries to make sure cov-build worked fine
if ! grep "compilation units (100%) successfully" "${BUILD_LOG}"; then
  echo "cov-build did not complete successfully. Please check ${BUILD_LOG} for details."
  exit 1
fi

#TODO: SUM() the compilation units from all builds and print the total
# if ! COMPILATION_UNITS=$(grep -oE '[0-9]+ C/C\+\+ compilation units \(100%\) successfully' "${BUILD_LOG}" | awk '{print $1}' | tail -n 1); then
#   echo "Error occurred while extracting compilation unit count from ${BUILD_LOG}."
#   exit 1
# fi

# if [ -n "${COMPILATION_UNITS}" ]; then
#   echo "cov-build processed ${COMPILATION_UNITS} compilation units successfully."
# else
#   echo "Warning: could not determine compilation unit count from ${BUILD_LOG}."
#   exit 1
# fi

# If required, https://wiki.ith.intel.com/spaces/SecTools/pages/2260108142/Enterprise+Coverity+Help+Topics#EnterpriseCoverityHelpTopics-ExcludeCode can be used to exclude files or directories from analysis

DISABLE_CHECKERS=(
  ASSERT_SIDE_EFFECT
  AUTO_CAUSES_COPY
  BAD_CHECK_OF_WAIT_COND
  BAD_SHIFT
  COPY_INSTEAD_OF_MOVE
  CUDA.COLLECTIVE_WARP_SHUFFLE_WIDTH
  CUDA.CUDEVICE_HANDLES
  CUDA.DEVICE_DEPENDENT
  CUDA.DEVICE_DEPENDENT_CALLBACKS
  CUDA.DIVERGENCE_AT_COLLECTIVE_OPERATION
  CUDA.ERROR_INTERFACE
  CUDA.ERROR_KERNEL_LAUNCH
  CUDA.FORK
  CUDA.INACTIVE_THREAD_AT_COLLECTIVE_WARP
  CUDA.INITIATION_OBJECT_DEVICE_THREAD_BLOCK
  CUDA.INVALID_MEMORY_ACCESS
  CUDA.SHARE_FUNCTION
  CUDA.SHARE_OBJECT_STREAM_ASSOCIATED
  CUDA.SPECIFIERS_INCONSISTENCY
  CUDA.SYNCHRONIZE_TERMINATION
  INEFFICIENT_RESERVE
  MISSING_COMMA
  MISSING_MOVE_ASSIGNMENT
  OVERLAPPING_COPY
  STREAM_FORMAT_STATE
  UNINTENDED_INTEGER_DIVISION
)

DISABLE_ARGS=()
for checker in "${DISABLE_CHECKERS[@]}"; do
  DISABLE_ARGS+=("--disable" "${checker}")
done

echo "Starting Coverity analysis..."

# A for ANALYZE
cov-analyze --dir "${OUTPUT_COV_EMIT}" \
  --security --concurrency --enable-constraint-fpp --enable-fnptr --enable-virtual \
  "${DISABLE_ARGS[@]}" \
  --strip-path "${REPO_ROOT}"

echo "Committing Coverity analysis results..."

# C for COMMIT
cov-commit-defects --dir "${OUTPUT_COV_EMIT}" --stream "${PROJECT_NAME}" --url "${COVERITY_SERVER_URL}" --auth-key-file "${AUTH_KEY}" --strip-path "${REPO_ROOT}" --noxrefs

# https://wiki.ith.intel.com/spaces/SecTools/pages/2260108142/Enterprise+Coverity+Help+Topics#EnterpriseCoverityHelpTopics-Excludeunwantedheaderfilesgettingcommitted tells us the best way to avoid unwanted header files getting committed is from WebUI

echo "Coverity analysis and commit completed successfully."
