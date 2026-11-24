#ifndef ROBOT_TYPES_H
#define ROBOT_TYPES_H

// shared types for the whole firmware
// dont change without telling the group, it breaks other peoples code

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LINE_SENSOR_COUNT 5

// robot modes, numbers are also used in the BT protocol (SET_MODE)
typedef enum {
    MODE_IDLE = 0,
    MODE_MANUAL = 1,
    MODE_LINE = 2,
    MODE_FOLLOW = 3,
    MODE_AVOID = 4,
    MODE_FAILSAFE = 5
} robot_mode_t;

// one set of sensor readings, filled by the drivers
typedef struct {
    uint16_t line_raw[LINE_SENSOR_COUNT];  // ADC 0-4095, index 0 = leftmost sensor
    float distance_cm;                     // HC-SR04, -1 = no echo
    float battery_v;                       // battery voltage in V
    uint32_t timestamp_ms;                 // time of the reading
} sensor_data_t;

// command for the motors
typedef struct {
    float left;   // -1.0 .. 1.0, plus = forward
    float right;  // -1.0 .. 1.0, plus = forward
} motor_cmd_t;

// tuning params, can be changed from the laptop app
typedef struct {
    // line tracking
    float line_kp;
    float line_ki;
    float line_kd;
    float line_base_speed;                 // 0 .. 1
    uint16_t line_min[LINE_SENSOR_COUNT];  // calibration, ADC on white
    uint16_t line_max[LINE_SENSOR_COUNT];  // calibration, ADC on black

    // object following
    float follow_target_cm;  // distance to keep, about 20 cm
    float follow_kp;

    // obstacle avoiding
    float avoid_dist_cm;     // start avoiding below this distance
    float avoid_speed;       // 0 .. 1
} autonomy_params_t;

#ifdef __cplusplus
}
#endif

#endif
