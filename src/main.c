#include "controller.h"
#include <stdio.h>
#include <string.h>

static int emit_one(controller_t *c) {
    sample_t s;
    uint8_t b[FRAME_SIZE];
    if (!controller_pop(c, &s)) {
        return 1;
    }
    frame_encode(b, s);
    return fwrite(b, 1, FRAME_SIZE, stdout) == FRAME_SIZE;
}

int main(int argc, char **argv) {
    int slow = argc == 2 && strcmp(argv[1], "--slow-consumer") == 0;
    if (argc != 1 && !slow) {
        fprintf(stderr, "usage: %s [--slow-consumer]\n", argv[0]);
        return 2;
    }
    controller_t c = {0};
    const int16_t temperatures[] = {2500, 2950, 3100, 2900, 2750, -125};
    for (size_t i = 0; i < sizeof temperatures / sizeof temperatures[0]; i++) {
        controller_sample(&c, (sample_t){(uint16_t)i, temperatures[i]});
        fprintf(stderr, "sample=%zu alarm=%d dropped=%u\n", i, c.alarm, c.dropped);
        if (!slow && !emit_one(&c)) {
            return 1;
        }
    }
    /* Slow mode holds the consumer until all six samples have arrived. */
    while (c.count != 0) {
        if (!emit_one(&c)) {
            return 1;
        }
    }
    fprintf(stderr, "summary generated=6 dropped=%u alarm=%d\n", c.dropped, c.alarm);
    return fflush(stdout) == 0 ? 0 : 1;
}
