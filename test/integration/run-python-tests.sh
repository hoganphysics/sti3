#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/../.." && pwd)"

build_dir="${STI3_BUILD_DIR:-${repo_root}/build-ninja}"
if [[ "${build_dir}" != /* ]]; then
  build_dir="${repo_root}/${build_dir}"
fi

conda_env="${STI3_CONDA_ENV:-sti3-build}"

if [[ -n "${STI3_INTEGRATION_PYTHON:-}" ]]; then
  python_cmd=("${STI3_INTEGRATION_PYTHON}")
elif command -v conda >/dev/null 2>&1; then
  python_cmd=(conda run --no-capture-output -n "${conda_env}" python)
else
  python_cmd=(python3)
fi

if [[ -n "${STI3_CONDA_PREFIX:-}" ]]; then
  python_prefix="${STI3_CONDA_PREFIX}"
else
  python_prefix="$("${python_cmd[@]}" -c 'import sys; print(sys.prefix)')"
fi

required_paths=(
  "${build_dir}/Lib/site-packages"
  "${repo_root}/test/integration/python"
  "${build_dir}/src/device/src"
  "${build_dir}/src/network/src"
)

missing_paths=()
for path in "${required_paths[@]}"; do
  if [[ ! -e "${path}" ]]; then
    missing_paths+=("${path}")
  fi
done

if (( ${#missing_paths[@]} > 0 )); then
  printf 'Missing required build-tree paths:\n' >&2
  printf '  %s\n' "${missing_paths[@]}" >&2
  printf 'Build the project or set STI3_BUILD_DIR to the build directory.\n' >&2
  exit 2
fi

join_by_colon() {
  local IFS=:
  printf '%s' "$*"
}

pythonpath_entries=(
  "${build_dir}/Lib/site-packages"
  "${repo_root}/test/integration/python"
)

ld_library_entries=()
if [[ -d "${python_prefix}/lib" ]]; then
  ld_library_entries+=("${python_prefix}/lib")
fi
ld_library_entries+=(
  "${build_dir}/src/device/src"
  "${build_dir}/src/network/src"
)
for optional_dir in \
  "${build_dir}/src/stipy/src" \
  "${build_dir}/src/stipy/stidevicepy/src" \
  "${build_dir}/Lib/site-packages/stipy" \
  "${build_dir}/Lib/site-packages/stipy/stidevicepy" \
  "${build_dir}/Lib/site-packages/stipy/stipybase"
do
  if [[ -d "${optional_dir}" ]]; then
    ld_library_entries+=("${optional_dir}")
  fi
done

export PYTHONPATH="$(join_by_colon "${pythonpath_entries[@]}")${PYTHONPATH:+:${PYTHONPATH}}"
export LD_LIBRARY_PATH="$(join_by_colon "${ld_library_entries[@]}")${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
export PYTHONDONTWRITEBYTECODE=1

print_env() {
  printf 'Repository: %s\n' "${repo_root}"
  printf 'Build directory: %s\n' "${build_dir}"
  printf 'Python command:'
  printf ' %q' "${python_cmd[@]}"
  printf '\n'
  printf 'Python prefix: %s\n' "${python_prefix}"
  printf 'PYTHONPATH: %s\n' "${PYTHONPATH}"
  printf 'LD_LIBRARY_PATH: %s\n' "${LD_LIBRARY_PATH}"
}

check_imports() {
  local output
  if ! output="$("${python_cmd[@]}" -c 'import pytest; import stipy; import stipy.stidevicepy' 2>&1)"; then
    printf '%s\n' "${output}" >&2
    printf '\nSelected Python could not import pytest and the build-tree stipy package together.\n' >&2
    printf 'Install pytest into the selected Python environment or set STI3_INTEGRATION_PYTHON.\n' >&2
    printf 'For the default conda env, use: conda install -n %s -c conda-forge pytest\n' "${conda_env}" >&2
    exit 3
  fi
}

case "${1:-}" in
  --print-env)
    print_env
    exit 0
    ;;
  --check-env)
    check_imports
    printf 'Integration Python environment is ready.\n'
    exit 0
    ;;
esac

check_imports

if (( $# == 0 )); then
  pytest_args=("${repo_root}/test/integration/python" "-q")
elif [[ "${1}" == -* ]]; then
  pytest_args=("${repo_root}/test/integration/python" "$@")
else
  pytest_args=("$@")
fi

exec "${python_cmd[@]}" -m pytest "${pytest_args[@]}"
