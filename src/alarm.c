#include "alarm.h"
int alarm_update(int active, int16_t temperature) {
    if (temperature >= 3000) {
        return 1;
    }
    if (temperature <= 2800) {
        return 0;
    }
    return active != 0;
}
