#include "controller.h"
#include <stdio.h>

int main(void) {
    controller_t c = {0};
    const int16_t temperatures[] = {2500, 2950, 3100, 2900, 2750, -125};

    for (size_t i = 0; i < sizeof temperatures / sizeof temperatures[0]; i++) {
        controller_sample(&c, (sample_t){(uint16_t)i, temperatures[i]});
        fprintf(stderr, "sample=%zu alarm=%d dropped=%u\n", i, c.alarm, c.dropped);
        sample_t s; uint8_t b[FRAME_SIZE];

        if (controller_pop(&c, &s)) {
            frame_encode(b, s);
            if (fwrite(b, 1, FRAME_SIZE, stdout) != FRAME_SIZE) {
                return 1;
            }
        }
    }
    
    return fflush(stdout) == 0 ? 0 : 1;
}
