import binascii
import struct
import subprocess

def expected(seq, temp):
    body = struct.pack('<BHh', 1, seq, temp)
    return b'\xa5\x5a' + body + struct.pack('<H', binascii.crc_hqx(body, 0xffff))

temps = [2500, 2950, 3100, 2900, 2750, -125]
for arguments, count, drops in [([], 6, 0), (['--slow-consumer'], 4, 2)]:
    result = subprocess.run(['./build/producer', *arguments], capture_output=True, check=True, timeout=5)

    assert result.stdout == b''.join(expected(i, t) for i, t in enumerate(temps[:count]))
    assert f'summary generated=6 dropped={drops} alarm=0'.encode() in result.stderr

    if drops:
        # A dropped sample at index 4 still clears the alarm
        assert b'sample=4 alarm=0 dropped=1' in result.stderr

bad = subprocess.run(['./build/producer', '--invalid'], capture_output=True, timeout=5)

assert bad.returncode == 2 and not bad.stdout
print('producer: independent wire oracle, normal/overload modes and argument rejection passed')
