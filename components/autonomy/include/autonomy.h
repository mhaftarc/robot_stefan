#ifndef AUTONOMY_H
#define AUTONOMY_H

// line tracking, object following and obstacle avoiding
//
// rules for this code:
// - no ESP32 or FreeRTOS calls, so it can be tested on a PC with gcc
// - no malloc
// - never block, autonomy_step is called every 10 ms (100 Hz)

#include "robot_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// call when the mode changes (clear integral, old errors etc.)
void autonomy_reset(
    void
);

// one step of the algorithm for the given mode
// returns speeds for the motors, -1 .. 1
motor_cmd_t autonomy_step(
    robot_mode_t mode,
    const sensor_data_t *s,
    const autonomy_params_t *p,
    float dt_s
);

#ifdef __cplusplus
}
#endif

#endif
