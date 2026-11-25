#ifndef MOTOR_H
#define MOTOR_H              // this module is going to be responsible for motor control


#include "esp_err.h" 
#include "robot_types.h"


esp_err_t motor_init();

void motor_set();

void motor_stop();















#endif