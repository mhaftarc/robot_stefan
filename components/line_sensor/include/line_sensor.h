#ifndef LINE_SENSOR_H
#define LINE_SENSOR_H

// 5x HW-006 line sensors, digital only (no analog out)

#include <stdint.h>
#include "esp_err.h"
#include "robot_types.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t line_sensor_init(
    void
);

// out[i] = 1 if sensor i sees the black line, out[0] = leftmost
void line_sensor_read(
    uint8_t out[LINE_SENSOR_COUNT]
);

#ifdef __cplusplus
}
#endif

#endif
