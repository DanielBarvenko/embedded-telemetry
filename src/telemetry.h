#ifndef TELEMETRY_H
#define TELEMETRY_H
#include <stddef.h>
#include <stdint.h>

#define FRAME_SIZE 9u

typedef struct { 
    uint16_t sequence; 
    int16_t temperature; 
} sample_t;

void frame_encode(uint8_t out[FRAME_SIZE], sample_t sample);
int frame_decode(const uint8_t in[FRAME_SIZE], sample_t *sample);

#endif
