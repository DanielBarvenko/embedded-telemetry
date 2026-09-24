# ARM firmware under Zephyr and QEMU (Ubuntu / WSL)

## 1. Install host prerequisites once

Run in the Ubuntu terminal, not PowerShell:

```sh
sudo apt update
sudo apt install -y build-essential python3-venv python3-dev git wget xz-utils file
cd /home/barve/portfolio/portfolio-starter/embedded-telemetry
bash scripts/setup_zephyr.sh
```

Use Ubuntu with Python 3.10 or later
This build was tested on Ubuntu 24.04 with Python 3.12
The script creates `~/zephyr-telemetry`, its own Python environment, a Zephyr checkout and ARM SDK. 
Downloads are substantial so allow several GB of free space and time for the first installation. 
Later builds reuse the tools.

The repository itself stays small. 
Only the ARM support modules are fetched.
The SDK supplies QEMU, the ARM compiler, gperf and the devicetree compiler.
CMake/Ninja are installed into the virtual environment.
The script does not edit shell startup files. 
It refuses to reset an existing checkout of a different Zephyr version.
It may register the SDK's CMake package through the official installer.

Pinned base:

| Component | Version |
| --- | --- |
| Zephyr | v4.2.0 / 413b789deb391d3a37d06b463288a5fe765ee57e |
| SDK | 0.17.2, ARM toolchain plus host tools |
| west | 1.5.0 |
| CMake | 4.4.3 |
| Ninja | 1.13.2 |
| Board | qemu_cortex_m3 (TI LM3S6965 / Cortex-M3) |

Zephyr module revisions are specified by that release's manifest.
Python dependencies use Zephyr's requirements
They are not a fully locked dependency set.

To use a different workspace location, export `ZEPHYR_WORKSPACE` consistently before setup/build/run.
No need to activate a virtual environment manually for these scripts.

## 2. Build and run

```sh
bash scripts/build_rtos.sh
bash scripts/run_rtos.sh
```

The build produces `build/rtos/zephyr/zephyr.elf`.
The runner starts the actual QEMU ARM emulator through west.
It then waits up to 30 wall-clock seconds for a complete summary.
Afterwards it checks its trace and then stops its own emulator process group.
Nonzero exit indicates failure
A missing summary is not success.
Increase the Python runner's `--timeout` option if diagnosing an unusually slow environment.

Look for `status=PASS` and `QEMU CHECK PASS`. 
Full output is saved in `build/rtos/qemu.log` and independently verified binary frames in `build/rtos/frames.bin`.
The QEMU network warning about a NIC with no peer is expected
This demo uses UART, not networking. 
An assertions-enabled build warning is also expected.

The emulator's virtual clock can run much faster than wall time. 
Periods are configured in simulated milliseconds
Host elapsed time is not a measurement of MCU performance.
`k_msleep()` adds a relative delay after work, so the sampling schedule is approximate.

## 3. Feed captured packets to the existing gateway

If the sibling gateway project is present, then do

```sh
make -C ../linux-telemetry-gateway
../linux-telemetry-gateway/build/gateway < build/rtos/frames.bin
```

This validates a captured packet path.
It is not a live UART transport. 
Dropped samples appear as sequence gaps.
Expected captured sequences from the recorded run are 0, 1, 2, 3, 6, 8, 10
Minor scheduling changes may change later accepted samples.

## How the tasks work

1. Both statically created workers initially block on startup semaphores. 
   Main prints the banner and releases the sampling worker.
2. The sampler owns alarm state. 
   It updates the alarm for every measurement, then attempts a nonblocking message enqueue. 
   A full queue drops the newest measurement.
3. The consumer remains gated for the first six generated samples, guaranteeing that a four-entry queue overflows. 
   The sampler then releases it.
4. The consumer copies messages out, serializes them, writes one diagnostic FRAME line and sleeps for 250 ms. 
   It sees the alarm snapshot for that measurement, not global state.
5. After twelve measurements the sampler sends an end marker using a blocking enqueue.
   This is permitted because acquisition is finished. 
   FIFO order ensures accepted samples precede the marker. 
   The consumer drains them and returns.
6. Main joins both workers and checks generated = accepted + dropped and accepted = transmitted.

Lower numerical priority has higher scheduling priority
I.e. Sampler 4, consumer 5.
The single-threaded `controller_t` queue is deliberately not used between these workers.

## If something fails

- If setup loses its network connection, rerun it. Do not commit downloaded tools.
- If the workspace version check fails, select a new `ZEPHYR_WORKSPACE`; do not reset unrelated work.
- If a build configuration is stale: `make clean-rtos`, then build again.
- If QEMU cannot lock its PID file, close a previous manual emulator run first.
- If you see an assertion, missing summary or CRC failure, retain `build/rtos/qemu.log`.
- If clocks cause Make warnings again, check the WSL clock and normalize extracted file timestamps.

References: [Zephyr v4.2.0 source](https://github.com/zephyrproject-rtos/zephyr/tree/v4.2.0),
[message queues](https://docs.zephyrproject.org/latest/kernel/services/data_passing/message_queues.html),
[Cortex-M3 emulation](https://docs.zephyrproject.org/latest/boards/qemu/cortex_m3/doc/index.html).
