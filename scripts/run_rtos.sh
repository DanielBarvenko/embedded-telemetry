#!/usr/bin/env bash
set -euo pipefail
project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
source "$project_dir/scripts/zephyr_env.sh"
python "$project_dir/scripts/check_qemu.py" --build-dir "$project_dir/build/rtos"
