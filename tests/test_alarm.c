#include "alarm.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(alarm_update(0, 2999) == 0);
    assert(alarm_update(0, 3000) == 1);
    assert(alarm_update(1, 2801) == 1);
    assert(alarm_update(1, 2800) == 0);
    assert(alarm_update(0, -32768) == 0);
    assert(alarm_update(0, 32767) == 1);
    
    puts("alarm: exact thresholds, hysteresis and signed extremes passed");
}
