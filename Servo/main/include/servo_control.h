#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

// Định nghĩa ID để phân biệt 2 Servo trong code
#define SERVO_PAN  0 // ID cho Servo trục ngang
#define SERVO_TILT 1 // ID cho Servo trục dọc

// Khởi tạo phần cứng MCPWM cho 2 Servo
void servo_init_hardware(void);

// Hàm điều khiển Servo xoay đến góc cụ thể (từ -90 đến 90 độ)
void servo_write_angle(int servo_id, int angle);

#endif