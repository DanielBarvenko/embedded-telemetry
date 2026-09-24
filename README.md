# Embedded telemetry — C core and Zephyr RTOS demo

Telemetry acquisition with bounded memory, alarm hysteresis, an overload policy,
and explicitly encoded CRC-protected packets. Includes a fast Linux/WSL demo and
an ARM Cortex-M3 firmware target running Zephyr in QEMU.

## Run the host version

Requires GCC, Make and Python 3 on Ubuntu/WSL:

```sh
make test
make demo
make demo-slow
```

Normal mode transmits all six samples. Slow mode holds the consumer while six samples
arrive in a four-slot queue: the first four are retained, two are dropped, and the alarm
still clears on a dropped sample. Binary output and diagnostics use separate streams.

```sh
./build/producer --slow-consumer > samples.bin
od -An -tx1 samples.bin
```

## Run the ARM / RTOS version

Follow [the WSL setup guide](docs/RTOS.md). Once dependencies are installed:

```sh
bash scripts/setup_zephyr.sh
bash scripts/build_rtos.sh
bash scripts/run_rtos.sh
```

The RTOS demo uses separate sampling and output threads, a four-entry Zephyr message
queue, and an explicit drop-newest policy. It samples scripted temperatures every
approximately 100 ms of simulated time and adds 250 ms of consumer delay. A controlled
startup backlog guarantees overload. Alarm logic and binary encoding are shared with
the host build; the host's single-threaded queue is not shared between RTOS threads.

Observed result for the pinned build:

```text
SUMMARY generated=12 accepted=7 dropped=5 transmitted=7 alarm=0 status=PASS
QEMU CHECK PASS: 7 frames, 5 drops; CRC, alarm snapshots and FIFO verified
```

[Captured UART output](docs/evidence/qemu-output.txt) and [validation details](docs/VALIDATION.md)
are included. The runner checks invariants rather than assuming an exact scheduling
outcome. It saves the UART trace and verified binary packets under `build/rtos/`.

## Architecture and decisions

- `src/alarm.c`: pure hysteresis logic; enter at 30 C, clear at 28 C. Each caller owns state.
- `src/controller.c`: bounded, single-threaded host queue and drop counter.
- `src/telemetry.c`: nine-byte framing with explicit byte order and CRC-16.
- `rtos/src/main.c`: Zephyr tasks, copied message snapshots, startup semaphores and orderly completion.
- `scripts/check_qemu.py`: emulator lifecycle and independent Python wire-format verification.

Fixed-size copied messages avoid sharing mutable sample storage or allocating in the
acquisition path. Normal sample enqueue never waits for space. The end marker is
control traffic and may wait so the consumer drains queued data before ending.
Main joins both workers before reading their counters.

The wire format is specified in [PROTOCOL.md](docs/PROTOCOL.md).

## Tests and CI

`make test` covers exact alarm thresholds, FIFO overflow/wraparound, corrupt frames,
an independently generated golden packet, both host demo modes, and rejected corrupt
emulator traces. `make sanitize` repeats host tests with ASan/UBSan. It does not sanitize
the ARM firmware. Host and RTOS builds are separate; `make clean` leaves the ARM build
intact and `make clean-rtos` removes it.

Two GitHub Actions workflows are supplied: host tests and a pinned ARM build/QEMU check.
Local validation is recorded; hosted workflow status must be checked after pushing.

## Scope and limitations

The RTOS image executes ARM firmware in QEMU, but the sensor values are scripted.
There is no physical sensor driver, custom ISR, real-board validation, persistent
logging, watchdog recovery, or hard real-time performance claim. Zephyr provides
startup, scheduling, SysTick and the UART console driver.

The UART demo prints packets as hex alongside diagnostics; it does not stream raw
protocol frames directly into a serial device. The runner validates and extracts the
hex packets to `frames.bin` for the companion gateway. CRC is not authentication.

Application storage is static and application code does not call malloc. This does
not claim the entire RTOS/libc contains no allocator code. Pointer APIs require valid
objects and buffers of the documented size. Counters may wrap in a longer-running adaptation.

The starter and this RTOS extension were developed with AI assistance. Future changes,
measurements and hardware testing should be documented according to what was actually done.
