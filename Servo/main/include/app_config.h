#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>
#include "driver/uart.h" // Cần để nhận diện UART_NUM_0

/* --- Cấu hình Giao tiếp UART --- */
#define UART_PORT_NUM      UART_NUM_0    // Cổng USB kết nối PC
#define UART_BAUD_RATE     115200        // Tốc độ truyền nhận chuẩn

/* --- Cấu hình chân Servo (MCPWM) --- */
#define SERVO_PAN_GPIO     5 // Servo xoay ngang chọn thùng
#define SERVO_TILT_GPIO    4 // Servo gạt rác xuống

/* --- Cấu hình Cảm biến --- */
#define IR_SENSOR_GPIO     6 // Hồng ngoại E18-D80NK

// Hệ thống Siêu âm HC-SR04
#define TRIG_GPIO          18  // Chân phát chung
#define ECHO_BIN1          10 // Trong
#define ECHO_BIN2          11 // Ngoai
/* --- Ngưỡng khoảng cách (cm) --- */
#define DIST_THRESHOLD_FULL    10.0  // Bé hơn 10cm là báo đầy
#define DIST_MAX_RANGE         400.0 // Giới hạn tối đa của cảm biến
// #define ECHO_BIN3          6 // Thùng Tái chế
// #define ECHO_BIN4          7 // Thùng Khác 

/* --- Buzzer  --- */
#define BUZZER_GPIO        13 // Buzzer tích cực (Báo khi nhận diện xong)


// LED Trạng thái 
// #define LED_READY_GPIO     2  // LED Xanh lá: Hệ thống sẵn sàng
// #define LED_BUSY_GPIO      3  // LED Vàng/Xanh dương: Đang xử lý/Quay Servo
// #define LED_ERROR_GPIO     9  // LED Đỏ: Thùng đầy hoặc lỗi kết nối Pi 4

#endif