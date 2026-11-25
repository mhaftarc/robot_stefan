#ifndef BOARD_PINS_H
#define BOARD_PINS_H

// real wiring from Kuba (22.11.2026)
// if you move a cable, change it here too

// TB6612FNG
#define PIN_MOTOR_PWMA  25   // left motor speed
#define PIN_MOTOR_AIN1  26
#define PIN_MOTOR_AIN2  27
#define PIN_MOTOR_PWMB  32   // right motor speed
#define PIN_MOTOR_BIN1  33
#define PIN_MOTOR_BIN2  14
#define PIN_MOTOR_STBY  13   // LOW = motors off

// HC-SR04, echo goes through 3x 1k divider
#define PIN_US_TRIG     18
#define PIN_US_ECHO     19

// line sensors HW-006, digital OUT, numbered 1-5 from the left
// index 0 in code = sensor 1 (leftmost)
#define PIN_LINE_1      35
#define PIN_LINE_2      36   // VP
#define PIN_LINE_3      39   // VN, middle
#define PIN_LINE_4      23
#define PIN_LINE_5      22

// battery through 4x 1k divider (V_bat / 4)
#define PIN_BATTERY     34

#define PIN_LED         2

// GPIO 12 is free but HIGH at boot breaks the start, dont use it

#endif
