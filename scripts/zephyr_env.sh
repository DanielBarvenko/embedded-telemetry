# Source from a Bash script. Does not edit shell profiles or global configuration.
workspace="${ZEPHYR_WORKSPACE:-$HOME/zephyr-telemetry}"

if [[ ! -f "$workspace/.venv/bin/activate" ]]; then
    echo "Run bash scripts/setup_zephyr.sh first (see docs/RTOS.md)." >&2
    return 1
fi

source "$workspace/.venv/bin/activate"

export ZEPHYR_BASE="$workspace/zephyr"
export ZEPHYR_SDK_INSTALL_DIR="$workspace/zephyr-sdk-0.17.2"
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr

if [[ ! -x "$ZEPHYR_SDK_INSTALL_DIR/arm-zephyr-eabi/bin/arm-zephyr-eabi-gcc" ]]; then
    echo "ARM SDK missing; run setup_zephyr.sh." >&2
    return 1
fi
