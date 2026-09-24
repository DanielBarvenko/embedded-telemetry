#ifndef ALARM_H
#define ALARM_H
#include <stdint.h>
/* Caller owns state. 30 C enters alarm; 28 C clears it. */
int alarm_update(int active, int16_t temperature);
#endif
