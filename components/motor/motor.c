#include <stdio.h>
#include "motor.h"
#include "board_pins.h"
#include "driver/gpio.h"
#include "driver/ledc.h"


#define PWM_FREQUENCY 20000
#define PWM_RES LEDC_TIMER_10_BIT
#define PWM_MAX 1023
#define MAX_DUTY 0.7*PWM_MAX  // gives around 6V for batter 8.4V
#define PWM_MODE LEDC_LOW_SPEED_MODE
#define CH_LEFT LEDC_CHANNEL_0
#define CH_RIGHT LEDC_CHANNEL_1




