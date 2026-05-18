#ifndef SENSOR_HANDLER_H
#define SENSOR_HANDLER_H

// Khởi tạo các chân GPIO cho IR và 4 cảm biến HC-SR04
void sensor_init(void);

// Hàm kích hoạt Buzzer kêu 1 tiếng ngắn
void buzzer_beep(void);

// Task FreeRTOS chạy ngầm để giám sát rác vào và mức đầy thùng
float sensor_get_dist(int echo_pin);

#endif