#include "controller.h"
#include "alarm.h"

void controller_sample(controller_t *c, sample_t s) {
    c->alarm = alarm_update(c->alarm, s.temperature);

    /* 
    Drop newest
    Preserve queued samples.
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
