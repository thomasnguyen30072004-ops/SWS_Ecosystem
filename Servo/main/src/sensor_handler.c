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

    // 2. Khởi tạo cho Buzzer
    gpio_reset_pin(BUZZER_GPIO);
    gpio_set_direction(BUZZER_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(BUZZER_GPIO, 0); // Kéo xuống mức 0 ban đầu để CHỐNG còi kêu rò lúc vừa bật nguồn

    // 3. Siêu âm HC-SR04
    gpio_set_direction(TRIG_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_BIN1, GPIO_MODE_INPUT);
    // gpio_set_direction(ECHO_BIN2, GPIO_MODE_INPUT);
}

void buzzer_beep(void) {
    gpio_set_level(BUZZER_GPIO, 1); // Cấp điện áp HIGH liên tục để kích mạch dao động nội của còi Active hú
    vTaskDelay(pdMS_TO_TICKS(200)); // Giữ tiếng bíp trong 200ms
    gpio_set_level(BUZZER_GPIO, 0); // Tắt còi dứt khoát
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