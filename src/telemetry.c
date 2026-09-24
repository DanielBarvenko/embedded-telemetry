#include "telemetry.h"

static uint16_t crc(const uint8_t *p, size_t n) {
    uint16_t c = 0xffffu;
    for (size_t i = 0; i < n; i++) {
        c ^= (uint16_t)((uint16_t)p[i] << 8);

        for (unsigned j = 0; j < 8; j++) {
            c = (uint16_t)((c & 0x8000u) ? ((unsigned)c << 1) ^ 0x1021u : (unsigned)c << 1);
        }    
    }
    return c;
}

static uint16_t get16(const uint8_t *p) { 
    return (uint16_t)((unsigned)p[0] | ((unsigned)p[1] << 8)); 
}

static void set16(uint8_t *p, uint16_t v) { 
    p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); 
}

void frame_encode(uint8_t out[FRAME_SIZE], sample_t s) {
    out[0] = 0xa5; out[1] = 0x5a; 
    out[2] = 1;
    set16(out + 3, s.sequence); 
    set16(out + 5, (uint16_t)s.temperature);
    set16(out + 7, crc(out + 2, 5));
}

int frame_decode(const uint8_t in[FRAME_SIZE], sample_t *s) {
    if (in[0] != 0xa5 || in[1] != 0x5a || in[2] != 1 || get16(in + 7) != crc(in + 2, 5)) {
        return 0;
    }

    uint16_t raw = get16(in + 5);

    s->sequence = get16(in + 3);
    s->temperature = (int16_t)(raw <= 32767u ? (int32_t)raw : (int32_t)raw - 65536);
    
    return 1;
}
