#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "line_sensor.h"
#include "battery.h"
#include "ultrasonic.h"

// TEST ONLY - prints all sensors, dont commit

static const char *TAG = "sensor_test";

void app_main(
    void
)
{
    ESP_ERROR_CHECK(line_sensor_init());
    ESP_ERROR_CHECK(battery_init());
    ESP_ERROR_CHECK(ultrasonic_init());

    uint8_t line[LINE_SENSOR_COUNT];

    while (1) {
        line_sensor_read(line);
        float bat = battery_read_v();
        float dist = ultrasonic_read_cm();

        ESP_LOGI(TAG, "line %d %d %d %d %d | dist %6.1f cm | bat %.2f V",
                 line[0], line[1], line[2], line[3], line[4], dist, bat);

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
