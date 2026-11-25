#include <stdbool.h>
#include "battery.h"
#include "board_pins.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"

#define BAT_DIVIDER  4.0f   // 4x 1k, we read 1/4 of the battery
#define BAT_SAMPLES  8

static const char *TAG = "battery";

static adc_oneshot_unit_handle_t adc1;
static adc_channel_t ch;
static adc_cali_handle_t cali;
static bool cali_ok = false;

esp_err_t battery_init(
    void
)
{
    adc_unit_t unit;
    esp_err_t err = adc_oneshot_io_to_channel(PIN_BATTERY, &unit, &ch);
    if (err != ESP_OK || unit != ADC_UNIT_1) {
        ESP_LOGE(TAG, "gpio %d is not on ADC1", PIN_BATTERY);
        return ESP_ERR_INVALID_ARG;
    }

    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    err = adc_oneshot_new_unit(&unit_cfg, &adc1);
    if (err != ESP_OK) {
        return err;
    }

    adc_oneshot_chan_cfg_t cfg = {
        .atten = ADC_ATTEN_DB_12,       // up to ~3.1 V, we get max 8.4 / 4 = 2.1 V
        .bitwidth = ADC_BITWIDTH_12,
    };
    err = adc_oneshot_config_channel(adc1, ch, &cfg);
    if (err != ESP_OK) {
        return err;
    }

    // raw -> mV calibration, if it fails use a rough formula
    adc_cali_line_fitting_config_t cali_cfg = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
        .default_vref = 1100,
    };
    cali_ok = (adc_cali_create_scheme_line_fitting(&cali_cfg, &cali) == ESP_OK);
    if (!cali_ok) {
        ESP_LOGW(TAG, "no ADC calibration, voltage will be rough");
    }

    return ESP_OK;
}

float battery_read_v(
    void
)
{
    // average a few samples, ESP32 ADC is noisy
    int sum = 0;
    for (int i = 0; i < BAT_SAMPLES; i++) {
        int raw = 0;
        adc_oneshot_read(adc1, ch, &raw);
        sum += raw;
    }
    int raw = sum / BAT_SAMPLES;

    int mv = 0;
    if (cali_ok) {
        adc_cali_raw_to_voltage(cali, raw, &mv);
    } else {
        mv = raw * 3300 / 4095;
    }

    // floats on purpose, with ints 1500 / 1000 would give 1
    return (mv / 1000.0f) * BAT_DIVIDER;
}
