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
#define TRIG_GPIO          9  // Chân phát chung
#define ECHO_BIN1          10 // Trong
// #define ECHO_BIN2          11 // Ngoài

/* --- Ngưỡng khoảng cách (cm) --- */
#define DIST_THRESHOLD_FULL    15.0  // Bé hơn 15cm là báo đầy
#define DIST_MAX_RANGE         400.0 // Giới hạn tối đa của cảm biến
// #define ECHO_BIN3          6 // Thùng Tái chế
// #define ECHO_BIN4          7 // Thùng Khác 

#define BATTERY_ADC_GPIO 12

#define BATTERY_R1_OHM 19630.0f
#define BATTERY_R2_OHM 4657.0f

#define BATTERY_DIVIDER_FACTOR \
    ((BATTERY_R1_OHM + BATTERY_R2_OHM) / BATTERY_R2_OHM)

#define BATTERY_ADC_SAMPLE_COUNT 32
#define BATTERY_READ_PERIOD_MS   10000

#define BATTERY_ADC_GAIN      1.0000f
#define BATTERY_ADC_OFFSET_MV 0.0f

#define BATTERY_STOP_VOUT_MV      2454
#define BATTERY_RECOVER_VOUT_MV   2493
#define BATTERY_LOW_CONFIRM_COUNT 3

/* --- Buzzer  --- */
#define BUZZER_GPIO        13 // Buzzer active
#define BUZZER_ACTIVE_LEVEL 1 // Cấp HIGH để hú


// LED Trạng thái 
#define LED_GREEN_GPIO     35  // LED Xanh lá: Hoạt động bình thường
#define LED_YELLOW_GPIO    36  // LED Vàng: Pin yếu (< 30%)
#define LED_RED_GPIO       37  // LED Đỏ: Thùng rác đầy

#endif