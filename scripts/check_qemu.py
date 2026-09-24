"""Run the actual ARM image, verify its UART trace, and stop QEMU after SUMMARY."""
import argparse
import binascii
import os
from pathlib import Path
import re
import selectors
import signal
import struct
import subprocess
import time

FRAME = re.compile(r"^FRAME seq=(\d+) temp=(-?\d+) alarm=([01]) hex=([0-9a-f]{18})$", re.M)
SUMMARY = re.compile(r"^SUMMARY generated=(\d+) accepted=(\d+) dropped=(\d+) transmitted=(\d+) alarm=([01]) status=(PASS|FAIL)$", re.M)
TEMPERATURES = [2500, 2950, 3100, 2900, 2750, -125, 3200, 2900, 2800, 3050, 2850, 2700]

def verify(trace):
    summaries = SUMMARY.findall(trace)
    if len(summaries) != 1:
        raise ValueError('expected exactly one complete SUMMARY line')
    
    generated, accepted, dropped, transmitted, final_alarm, status = summaries[0]
    generated, accepted, dropped, transmitted, final_alarm = map(int, (generated, accepted, dropped, transmitted, final_alarm))
    
    rows = FRAME.findall(trace)

    if status != 'PASS' or generated != 12 or accepted + dropped != generated or transmitted != accepted or len(rows) != accepted or dropped < 2 or final_alarm != 0:
        raise ValueError('summary accounting or completion check failed')
    
    alarms = []
    alarm = 0
    
    for temperature in TEMPERATURES:
        if temperature >= 3000:
            alarm = 1
        elif temperature <= 2800:
            alarm = 0
        alarms.append(alarm)
    
    sequences = []
    
    for seq_text, temp_text, alarm_text, hex_text in rows:
        seq, temp, state = int(seq_text), int(temp_text), int(alarm_text)
        if not 0 <= seq < generated or temp != TEMPERATURES[seq] or state != alarms[seq]:
            raise ValueError('sample value or alarm snapshot mismatch')
        
        body = struct.pack('<BHh', 1, seq, temp)
        expected = b'\xa5\x5a' + body + struct.pack('<H', binascii.crc_hqx(body, 0xffff))
        
        if bytes.fromhex(hex_text) != expected:
            raise ValueError('wire encoding or independent CRC check failed')
        
        sequences.append(seq)
    
    if sequences != sorted(set(sequences)) or sequences[:4] != [0, 1, 2, 3] or 4 in sequences or 5 in sequences:
        raise ValueError('FIFO ordering or drop-newest policy failed')
    
    return f'QEMU CHECK PASS: {accepted} frames, {dropped} drops; CRC, alarm snapshots and FIFO verified'

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir', type=Path, required=True)
    parser.add_argument('--timeout', type=float, default=30)
    args = parser.parse_args()
    build = args.build_dir.resolve()
    
    if not (build / 'zephyr/zephyr.elf').is_file():
        parser.error('ARM image missing; run bash scripts/build_rtos.sh first')
   
    process = subprocess.Popen(['west', 'build', '-d', str(build), '-t', 'run'],
                               stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               stdin=subprocess.DEVNULL, start_new_session=True)
    
    selector = selectors.DefaultSelector()
    selector.register(process.stdout, selectors.EVENT_READ)
    output = bytearray()
    deadline = time.monotonic() + args.timeout
    complete = False
    
    try:
        while time.monotonic() < deadline:
            ready = selector.select(timeout=min(0.2, max(0, deadline - time.monotonic())))
            
            if not ready:
                if process.poll() is not None:
                    break
                continue
            chunk = os.read(process.stdout.fileno(), 4096)
            
            if not chunk:
                break
            
            output.extend(chunk)
            print(chunk.decode(errors='replace'), end='', flush=True)
            text = output.decode(errors='replace').replace('\r', '')
            
            if re.search(r'^SUMMARY [^\n]*\n', text, re.M):
                complete = True
                break
    finally:
        selector.close()
        
        try:
            os.killpg(process.pid, signal.SIGTERM)
        except ProcessLookupError:
            pass
        
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            process.wait()
        
        process.stdout.close()
        trace = output.decode(errors='replace').replace('\r', '')
        (build / 'qemu.log').write_text(trace)
    
    if not complete:
        raise SystemExit('QEMU CHECK FAIL: no complete summary before exit/timeout; see build/rtos/qemu.log')
    try:
        result = verify(trace)
    
    except ValueError as error:
        raise SystemExit(f'QEMU CHECK FAIL: {error}') from error
    
    frames = b''.join(bytes.fromhex(row[3]) for row in FRAME.findall(trace))
    (build / 'frames.bin').write_bytes(frames)
    
    print(result)
    print(f'Validated binary frames saved: {build / "frames.bin"}')
    print(f'Trace saved: {build / "qemu.log"}')

if __name__ == '__main__':
    main()
