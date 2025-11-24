#ifndef SENSOR_H
#define SENSOR_H
#include <stdint.h>


uint16_t clean = 2650;
uint16_t dirty = 500;

uint16_t sensor_handler(uint16_t raw) {
    float percent = (clean - raw) * 100 / (clean - dirty);
    if (percent > 100.0) percent = 100.0;
    else if (percent < 0.0) percent = 0.0;

    
    return percent;

}

uint8_t data = 0;
#endif 