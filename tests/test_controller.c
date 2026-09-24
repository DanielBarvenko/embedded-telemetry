#include "controller.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    controller_t c = {0}; 
    sample_t s;
    assert(!controller_pop(&c, &s));

    controller_sample(&c, (sample_t){0,3000}); 
    assert(c.alarm);
    controller_sample(&c, (sample_t){1,2900});
    assert(c.alarm);
    controller_sample(&c, (sample_t){2,2800}); 
    assert(!c.alarm);

    controller_sample(&c, (sample_t){3,2700});
    controller_sample(&c, (sample_t){4,3100}); 
    assert(c.dropped == 1 && c.alarm);

    for (uint16_t i = 0; i < 4; i++) { 
        assert(controller_pop(&c, &s)); 
        assert(s.sequence == i); 
    }
    assert(!controller_pop(&c, &s));

    for (uint16_t i = 5; i < 100; i++) {
        controller_sample(&c, (sample_t){i,2500});
        assert(controller_pop(&c, &s) && s.sequence == i);
    }
    puts("controller: hysteresis, overflow, FIFO, wraparound passed");
}
