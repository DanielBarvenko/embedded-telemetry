# Embedded telemetry: portable controller core

A C11 telemetry controller with a Linux-hosted demonstration
It separates state and framing from the future board/RTOS adapter

## Implemented

- Four-sample FIFO with a drop-newest overflow policy and a dropped count.
- Alarm enters at 30 C and clears at 28 C (the gap prevents threshold chatter).
- Alarm updates even if the outgoing queue is full.
- Fixed-width binary frames with explicit byte order and CRC-16.
- Six deterministic sample inputs (binary stdout, diagnostic stderr).
- Tests for boundaries, FIFO wraparound, overflow and corrupt frames.

```sh
make
./build/producer > samples.bin
od -An -tx1 samples.bin
```

The demo drains after each sample
The overflow path is exercised by the unit test.
The portable core has no heap allocation. 
It is single-threaded, requires a zero-initialized controller, and has not been tested on an MCU. 
There are no timing guarantees, physical sensor drivers, fault-recovery state machine or RTOS tasks yet.

## Design

`src/controller.c` owns alarm and queue state
`src/telemetry.c` serializes records
`src/main.c` supplies scripted sensor data. 
The protocol is in `docs/PROTOCOL.md`.
Counters are unsigned and may wrap in a sufficiently long run.
All pointer arguments must reference valid objects.
NULL is not accepted by controller/protocol APIs.

## Build and verify

On Ubuntu / WSL with GCC, Make and Python 3:

```sh
make test
make sanitize
```

CI runs both commands. 
`make clean && make test` returns to an ordinary build.
Do not compile them with NDEBUG. 
See `NEXT_STEPS.md` for extensions and discussion prompts. 
