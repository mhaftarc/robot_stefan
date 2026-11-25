#include "line_sensor.h"
#include "board_pins.h"
#include "driver/gpio.h"

static const int pins[LINE_SENSOR_COUNT] = {
    PIN_LINE_1, PIN_LINE_2, PIN_LINE_3, PIN_LINE_4, PIN_LINE_5
};

esp_err_t line_sensor_init(
    void
)
{
    uint64_t mask = 0;
    for (int i = 0; i < LINE_SENSOR_COUNT; i++) {
        mask |= 1ULL << pins[i];
    }

    // module drives OUT itself, no pull needed
    // (34-39 dont even have internal pulls)
    gpio_config_t io = {
        .pin_bit_mask = mask,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    return gpio_config(&io);
}

void line_sensor_read(
    uint8_t out[LINE_SENSOR_COUNT]
)
{
    // module gives 0 on black, we flip it so 1 = line
    for (int i = 0; i < LINE_SENSOR_COUNT; i++) {
        out[i] = gpio_get_level(pins[i]) ? 0 : 1;
    }
}
