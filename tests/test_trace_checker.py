import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
from check_qemu import verify

trace = Path('docs/evidence/qemu-output.txt').read_text()
assert 'PASS' in verify(trace)

corruptions = [
    trace.replace('a55a010000c409e4f0', 'a55a010000c409e4f1'),
    trace.replace('dropped=5', 'dropped=4'),
    trace.replace('FRAME seq=0 temp=2500 alarm=0', 'FRAME seq=0 temp=2500 alarm=1'),
    trace.replace('status=PASS', 'status=FAIL'),
    trace[:trace.index('SUMMARY')],
    trace + trace[trace.index('SUMMARY'):],
]

for altered in corruptions:
    try:
        verify(altered)
    except ValueError:
        pass
    else:
        raise AssertionError('invalid trace accepted')
        
print('QEMU trace checker: recorded run accepted; corrupt/incomplete traces rejected')
