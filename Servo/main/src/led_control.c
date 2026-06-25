#include "led_control.h"
#include "driver/gpio.h"
#include "app_config.h"

void led_control_init(void) {
    gpio_reset_pin(LED_GREEN_GPIO);
    gpio_reset_pin(LED_YELLOW_GPIO);
    gpio_reset_pin(LED_RED_GPIO);
    
    gpio_set_direction(LED_GREEN_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_YELLOW_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_RED_GPIO, GPIO_MODE_OUTPUT);
    
    // Mặc định ban đầu hệ thống khởi động an toàn: Sáng Xanh lá
    led_control_set_status(true, false, false);
}

void led_control_set_status(bool green, bool yellow, bool red) {
    gpio_set_level(LED_GREEN_GPIO, green ? 1 : 0);
    gpio_set_level(LED_YELLOW_GPIO, yellow ? 1 : 0);
    gpio_set_level(LED_RED_GPIO, red ? 1 : 0);
}

void led_control_update(bool bin_full, bool battery_low) {
    // Thuật toán phân cấp mức độ ưu tiên hiển thị cảnh báo
    if (bin_full) {
        led_control_set_status(false, false, true);  // Ưu tiên 1: Thùng đầy (Đỏ)
    } else if (battery_low) {
        led_control_set_status(false, true, false);  // Ưu tiên 2: Pin yếu (Vàng)
    } else {
        led_control_set_status(true, false, false);  // Trạng thái chuẩn: Bình thường (Xanh)
    }
}