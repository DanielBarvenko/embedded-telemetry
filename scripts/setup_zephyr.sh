#!/usr/bin/env bash
set -euo pipefail

export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
workspace="${ZEPHYR_WORKSPACE:-$HOME/zephyr-telemetry}"
zephyr_commit="413b789deb391d3a37d06b463288a5fe765ee57e"

for tool in git python3 gcc g++ make xz wget; do
    command -v "$tool" >/dev/null || { echo "Missing $tool; see docs/RTOS.md" >&2; exit 1; }
done

python3 -c 'import sys; assert sys.version_info >= (3,10), "Python 3.10+ required"'
mkdir -p "$workspace"

if [[ ! -x "$workspace/.venv/bin/python" ]]; then
    python3 -m venv "$workspace/.venv"
fi

source "$workspace/.venv/bin/activate"

python -m pip install 'west==1.5.0' 'cmake==4.4.3' 'ninja==1.13.2'

if [[ ! -d "$workspace/.west" ]]; then
    west init --mr v4.2.0 "$workspace"
fi

if [[ "$(git -C "$workspace/zephyr" rev-parse HEAD)" != "$zephyr_commit" ]]; then
    echo "Existing workspace is not Zephyr v4.2.0. Choose a new ZEPHYR_WORKSPACE; nothing was reset." >&2
    exit 1
fi

cd "$workspace"

# Only the ARM support modules needed by this target, not every vendor HAL
west update --narrow cmsis cmsis_6

python -m pip install -r zephyr/scripts/requirements-base.txt
sdk="$workspace/zephyr-sdk-0.17.2"

if [[ ! -x "$sdk/arm-zephyr-eabi/bin/arm-zephyr-eabi-gcc" || ! -d "$sdk/sysroots" ]]; then
    west sdk install --version 0.17.2 --toolchains arm-zephyr-eabi --install-dir "$sdk"
fi

printf '\nSetup complete. Return to the repository and run:\n  bash scripts/build_rtos.sh\n  bash scripts/run_rtos.sh\n'
