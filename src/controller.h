#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "telemetry.h"
#define QUEUE_CAPACITY 4u

typedef struct {
    sample_t queue[QUEUE_CAPACITY];
    size_t head, count;
    unsigned dropped;
    int alarm;
} controller_t;

/* 
Single-threaded core
Alarm enters at 30 C and clears at 28 C
*/
void controller_sample(controller_t *c, sample_t s);
int controller_pop(controller_t *c, sample_t *s);

#endif
