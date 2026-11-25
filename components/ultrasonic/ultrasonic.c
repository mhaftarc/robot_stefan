#include <stdint.h>
#include "ultrasonic.h"
#include "board_pins.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define ECHO_TIMEOUT_MS  30        // ~5 m, more than the sensor can do
#define US_PER_CM        58.0f     // sound there and back

static SemaphoreHandle_t echo_done;
static volatile int64_t t_rise;
static volatile int64_t t_fall;

// runs on both edges of ECHO, just saves the times
static void echo_isr(
    void *arg
)
{
    (void)arg;
    int64_t now = esp_timer_get_time();

    if (gpio_get_level(PIN_US_ECHO)) {
        t_rise = now;
    } else {
        t_fall = now;
        BaseType_t woken = pdFALSE;
        xSemaphoreGiveFromISR(echo_done, &woken);
        portYIELD_FROM_ISR(woken);
    }
}

esp_err_t ultrasonic_init(
    void
)
{
    gpio_config_t trig = {
        .pin_bit_mask = 1ULL << PIN_US_TRIG,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&trig);
    if (err != ESP_OK) {
        return err;
    }
    gpio_set_level(PIN_US_TRIG, 0);

    gpio_config_t echo = {
        .pin_bit_mask = 1ULL << PIN_US_ECHO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    err = gpio_config(&echo);
    if (err != ESP_OK) {
        return err;
    }

    echo_done = xSemaphoreCreateBinary();
    if (echo_done == NULL) {
        return ESP_ERR_NO_MEM;
    }

    // someone else may have installed it already, thats fine
    err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }

    return gpio_isr_handler_add(PIN_US_ECHO, echo_isr, NULL);
}

float ultrasonic_read_cm(
    void
)
{
    xSemaphoreTake(echo_done, 0);  // clear old result if any
    t_rise = 0;
    t_fall = 0;

    // 10 us pulse starts the measurement
    gpio_set_level(PIN_US_TRIG, 1);
    esp_rom_delay_us(10);
    gpio_set_level(PIN_US_TRIG, 0);

    if (xSemaphoreTake(echo_done, pdMS_TO_TICKS(ECHO_TIMEOUT_MS)) != pdTRUE) {
        return -1.0f;
    }

    int64_t us = t_fall - t_rise;
    if (t_rise == 0 || us <= 0) {
        return -1.0f;
    }

    return us / US_PER_CM;
}
