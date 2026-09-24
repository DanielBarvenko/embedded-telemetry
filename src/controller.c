#include "controller.h"

void controller_sample(controller_t *c, sample_t s) {
    if (s.temperature >= 3000){
        c->alarm = 1;
    } 
    else if (s.temperature <= 2800) {
        c->alarm = 0;
    }

    /* 
    Drop newest
    Peserve queued samples. 
    Alarm still updates.
     */
    if (c->count == QUEUE_CAPACITY) {
        ++c->dropped; 
        return;
    }

    c->queue[(c->head + c->count) % QUEUE_CAPACITY] = s;
    ++c->count;
}

int controller_pop(controller_t *c, sample_t *s) {
    if (!c->count) {
        return 0;
    } 
    *s = c->queue[c->head];
    c->head = (c->head + 1) % QUEUE_CAPACITY; --c->count;
    return 1;
}
