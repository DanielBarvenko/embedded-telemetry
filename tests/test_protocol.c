#include "telemetry.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    uint8_t b[FRAME_SIZE]; 
    sample_t out;
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
