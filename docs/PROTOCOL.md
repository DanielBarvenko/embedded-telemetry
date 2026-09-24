# Telemetry wire format v1

A fixed nine-byte frame 

| Offset | Size | Meaning                                                                      |
| ---    | ---  | ---                                                                          |
| 0      | 2    | Synchronization bytes A5 5A                                                  |
| 2      | 1    | Version: 01                                                                  |
| 3      | 2    | Unsigned sequence, little-endian, wrapping at 65536                          |
| 5      | 2    | Signed 16-bit two's-complement temperature, hundredths Celsius little-endian |
| 7      | 2    | CRC-16/CCITT-FALSE over offsets 2–6, little-endian                           |

CRC polynomial 0x1021, initial value 0xFFFF.
CRC detects accidental errors but does not provide authentication.

The decoder tests each nine-byte candidate.
A valid frame consumes nine bytes. 
An invalid candidate consumes one byte and retains eight.
This bounded sliding window recovers after noise without trusting a length from the stream.
A CRC collision can still yield a false frame. 
Version 1 has no acknowledgments, retransmission, timestamps or sequence-gap policy. 
The gateway reports trailing bytes at EOF and returns success if I/O succeeded
It does not treat dirty input as a fatal error. 
Discarded bytes and trailing bytes are distinct counts.
