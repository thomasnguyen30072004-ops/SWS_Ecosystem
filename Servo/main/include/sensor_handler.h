#ifndef SENSOR_HANDLER_H
#define SENSOR_HANDLER_H


#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

// Kết quả của một lần đo pin
typedef struct {
    int raw;                    // Giá trị ADC thô trung bình
    uint16_t vout_mv;           // Điện áp thực tế tại GPIO12, đơn vị mV
    float vout_v;               // Điện áp sau cầu phân áp
    float vin_v;                // Điện áp pin tính ngược trước cầu phân áp
    uint8_t percent;            // Phần trăm pin nội suy từ bảng Vout
    bool below_stop_threshold;  // Vout <= ngưỡng tương ứng Vin 12.8 V
} battery_data_t;

// Khởi tạo các chân GPIO cho IR và 4 cảm biến HC-SR04
void sensor_init(void);

// Khởi tạo ADC đo pin tại BATTERY_ADC_GPIO
esp_err_t battery_adc_init(void);

// Đọc một mẫu pin đã lấy trung bình và hiệu chuẩn
esp_err_t battery_read(battery_data_t *data);

// Quy đổi trực tiếp Vout tại GPIO12 sang phần trăm pin
uint8_t battery_percent_from_vout(uint16_t vout_mv);

// Trạng thái pin yếu đã được xác nhận nhiều lần liên tiếp
bool battery_is_low(void);

// Task đọc và gửi trạng thái pin định kỳ qua UART
void battery_monitor_task(void *pvParameters);

// Hàm kích hoạt Buzzer kêu 1 tiếng ngắn
void buzzer_beep(void);

// Task FreeRTOS chạy ngầm để giám sát rác vào và mức đầy thùng
float sensor_get_dist(int echo_pin);

#endif