#include <stdint.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// TEST ONLY - spins the wheels, delete after
// pins = Kuba's real wiring (same as board_pins.h)

#define PIN_PWMA  GPIO_NUM_25   // left motor
#define PIN_AIN1  GPIO_NUM_26
#define PIN_AIN2  GPIO_NUM_27
#define PIN_PWMB  GPIO_NUM_32   // right motor
#define PIN_BIN1  GPIO_NUM_33
#define PIN_BIN2  GPIO_NUM_14
#define PIN_STBY  GPIO_NUM_13

#define PWM_FREQ_HZ  20000
#define PWM_RES      LEDC_TIMER_10_BIT   // duty 0..1023
#define TEST_DUTY    400                 // about 40%, slow and safe

static const char *TAG = "motor_test";

static void motors_init(
    void
)
{
    // direction pins + standby as normal outputs
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << PIN_AIN1) | (1ULL << PIN_AIN2) |
                        (1ULL << PIN_BIN1) | (1ULL << PIN_BIN2) |
                        (1ULL << PIN_STBY),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io));
    gpio_set_level(PIN_STBY, 0);  // driver off for now

    // one PWM timer for both motors
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = PWM_RES,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    // channel 0 = left (PWMA), channel 1 = right (PWMB)
    ledc_channel_config_t ch = {
        .gpio_num = PIN_PWMA,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch));

    ch.gpio_num = PIN_PWMB;
    ch.channel = LEDC_CHANNEL_1;
    ESP_ERROR_CHECK(ledc_channel_config(&ch));
}

// dir: 1 = forward, -1 = back, 0 = stop
static void motor_set(
    gpio_num_t in1,
    gpio_num_t in2,
    ledc_channel_t ch,
    int dir,
    uint32_t duty
)
{
    gpio_set_level(in1, dir > 0);
    gpio_set_level(in2, dir < 0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, ch, dir == 0 ? 0 : duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, ch);
}

static void left(
    int dir
)
{
    motor_set(PIN_AIN1, PIN_AIN2, LEDC_CHANNEL_0, dir, TEST_DUTY);
}

static void right(
    int dir
)
{
    motor_set(PIN_BIN1, PIN_BIN2, LEDC_CHANNEL_1, dir, TEST_DUTY);
}

static void step(
    const char *what,
    int l,
    int r
)
{
    ESP_LOGI(TAG, "%s", what);
    left(l);
    right(r);
    vTaskDelay(pdMS_TO_TICKS(2000));

    left(0);
    right(0);
    vTaskDelay(pdMS_TO_TICKS(1000));
}

void app_main(
    void
)
{
    motors_init();

    ESP_LOGI(TAG, "wheels up in the air! start in 3 s");
    vTaskDelay(pdMS_TO_TICKS(3000));

    gpio_set_level(PIN_STBY, 1);  // driver on

    while (1) {
        step("left forward", 1, 0);
        step("left back", -1, 0);
        step("right forward", 0, 1);
        step("right back", 0, -1);
        step("both forward", 1, 1);
        step("both back", -1, -1);
        ESP_LOGI(TAG, "loop done, again in 3 s");
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
