# Telemetry wire format v1

A fixed nine-byte frame
C structs are not transmitted directly

| Offset | Size | Meaning |
| --- | --- | --- |
| 0 | 2 | Synchronization bytes A5 5A |
| 2 | 1 | Version: 01 |
| 3 | 2 | Unsigned sequence, little-endian; wraps after 65535 |
| 5 | 2 | Signed 16-bit two's-complement temperature, hundredths Celsius, little-endian |
| 7 | 2 | CRC-16/CCITT-FALSE over offsets 2–6, little-endian |

CRC polynomial 0x1021, initial value 0xFFFF, input/output not reflected, final XOR 0.
CRC detects accidental errors and provides no authentication.

Independent golden packet: sequence 0x1234, temperature -125 (-1.25 C):

```text
a5 5a 01 34 12 83 ff 7d 20
```

The fixture is checked in C Python's `binascii.crc_hqx(body, 0xffff)` independently.
Checks producer output and captured ARM packets.

## Single-frame API versus stream handling

This repository's `frame_decode()` validates exactly one complete nine-byte candidate.
It does not buffer fragments, search for synchronization, consume stream bytes or recover from noise.
Callers must provide at least FRAME_SIZE readable bytes.
`frame_encode()` likewise requires at least FRAME_SIZE writable bytes.

The separate linux-telemetry-gateway repository implements the sliding-window stream parser.
A valid candidate consumes nine bytes
An invalid candidate consumes one and retains eight. 
That parser handles noise and EOF statistics. 

Version 1 has no acknowledgments, timestamps or authentication. 
The sequence allows a receiver to observe gaps but does not itself implement a sequence-gap policy.
Alarm state is an internal/diagnostic snapshot, not an extra field in the v1 wire frame.

## RTOS UART demonstration

Zephyr emits human-readable FRAME lines with the packet bytes represented as hex.
This prevents mixing raw binary with boot logs. 
`scripts/check_qemu.py` validates those lines and saves concatenated binary frames for the companion gateway.
