#include "telemetry.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    uint8_t b[FRAME_SIZE]; 
    sample_t out;

    /* Independent CRC-16 fixture: version=1, seq=0x1234, temp=-125. */
    const uint8_t expected[FRAME_SIZE] = {0xa5, 0x5a, 0x01, 0x34, 0x12, 0x83, 0xff, 0x7d, 0x20};
    frame_encode(b, (sample_t){0x1234, -125});

    assert(memcmp(b, expected, FRAME_SIZE) == 0);
    assert(frame_decode(expected, &out));
    assert(out.sequence == 0x1234 && out.temperature == -125);

    const int16_t temps[] = {-32768, -125, 0, 2500, 32767};
    
    for (size_t i = 0; i < sizeof temps / sizeof temps[0]; i++) {
        frame_encode(b, (sample_t){65535, temps[i]});
        assert(frame_decode(b, &out)); 
        assert(out.sequence == 65535 && out.temperature == temps[i]);

        for (size_t j = 0; j < FRAME_SIZE; j++) {
            b[j] ^= 1; assert(!frame_decode(b, &out)); b[j] ^= 1;
        }
    }

    puts("protocol: boundary values and corruption checks passed");
}
