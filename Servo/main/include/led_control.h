#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <stdbool.h>

// Hàm khởi tạo cấu hình phần cứng cho các chân LED RGB
void led_control_init(void);

// Hàm ép trạng thái logic bật/tắt trực tiếp cho từng màu LED
void led_control_set_status(bool green, bool yellow, bool red);

// Hàm xử lý thuật toán phân cấp ưu tiên hiển thị LED theo trạng thái hệ thống
void led_control_update(bool bin_full, bool battery_low);

#endif 