#include "sensor_handler.h"
#include "app_config.h"
#include "driver/gpio.h"
#include "driver/ledc.h" 
#include "esp_timer.h"

void sensor_init(void) {
    // 1. IR Sensor (NPN - Pullup)
    gpio_config_t ir_cfg = { 
        .pin_bit_mask = (1ULL << IR_SENSOR_GPIO), 
        .mode = GPIO_MODE_INPUT, 
        .pull_up_en = GPIO_PULLUP_ENABLE 
    };
    gpio_config(&ir_cfg);

    // 2. Khởi tạo PWM cho Buzzer
    ledc_timer_config_t buz_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 3000, // Tần số 3kHz
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&buz_timer);

    ledc_channel_config_t buz_chan = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .gpio_num = BUZZER_GPIO,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&buz_chan);

    // 3. Siêu âm HC-SR04
    gpio_set_direction(TRIG_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_BIN1, GPIO_MODE_INPUT);
    // gpio_set_direction(ECHO_BIN2, GPIO_MODE_INPUT);
}

void buzzer_beep(void) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 4000); // 50% duty
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    vTaskDelay(pdMS_TO_TICKS(150));
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

float sensor_get_dist(int echo_pin) {
    gpio_set_level(TRIG_GPIO, 0); esp_rom_delay_us(2);
    gpio_set_level(TRIG_GPIO, 1); esp_rom_delay_us(10);
    gpio_set_level(TRIG_GPIO, 0);

    uint64_t start = esp_timer_get_time();
    while (gpio_get_level(echo_pin) == 0 && (esp_timer_get_time() - start) < 30000);
    uint64_t t1 = esp_timer_get_time();
    while (gpio_get_level(echo_pin) == 1 && (esp_timer_get_time() - t1) < 30000);
    uint64_t t2 = esp_timer_get_time();

    return (t2 - t1) * 0.034 / 2;
}