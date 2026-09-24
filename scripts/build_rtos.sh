#!/usr/bin/env bash
set -euo pipefail
project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
source "$project_dir/scripts/zephyr_env.sh"
west build -p auto -b qemu_cortex_m3 -d "$project_dir/build/rtos" "$project_dir/rtos"
