#ifndef BATTERY_H
#define BATTERY_H

// battery voltage through the 4x 1k divider on GPIO 34

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t battery_init(
    void
);

// battery voltage in V, about 0.57 V reading = battery not connected
float battery_read_v(
    void
);

#ifdef __cplusplus
}
#endif

#endif
