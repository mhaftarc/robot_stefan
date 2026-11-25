#ifndef ULTRASONIC_H
#define ULTRASONIC_H

// HC-SR04 distance sensor

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t ultrasonic_init(
    void
);

// one measurement, waits max ~30 ms
// returns distance in cm, -1 = no echo (nothing in range)
// keep at least 60 ms between calls (sensor needs a break)
float ultrasonic_read_cm(
    void
);

#ifdef __cplusplus
}
#endif

#endif
