#include "autonomy.h"

// TODO Robert: real algorithms

void autonomy_reset(
    void
)
{
}

motor_cmd_t autonomy_step(
    robot_mode_t mode,
    const sensor_data_t *s,
    const autonomy_params_t *p,
    float dt_s
)
{
    // not used yet, stops unused warnings
    (void)mode;
    (void)s;
    (void)p;
    (void)dt_s;

    motor_cmd_t cmd = {0};  // stand still for now
    return cmd;
}
