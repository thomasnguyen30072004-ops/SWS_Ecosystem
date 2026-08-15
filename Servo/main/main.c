#include "driver/gpio.h"    
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "app_config.h" 
#include "servo_control.h"
#include "sensor_handler.h"
#include "uart_handler.h"
#include "led_control.h"

static const char *TAG = "3AE_MAIN";
QueueHandle_t xCmdQueue; 

// Biến tĩnh toàn cục lưu trữ trạng thái thùng đầy qua các vòng lặp quét siêu âm
static bool g_bin_full = false;

void sorting_task(void *pvParameters) {
    char cmd;
    bool object_present = false; 

    // Đưa về vị trí Start (Tấm gạt nằm ngang tại vị trí nhận rác)
    servo_write_angle(SERVO_PAN, 180); 
    servo_write_angle(SERVO_TILT, 92);

    vTaskDelay(pdMS_TO_TICKS(2000)); 

    while (1) {
        led_control_update(g_bin_full, battery_is_low());
        int ir_val = gpio_get_level(IR_SENSOR_GPIO);

        if (ir_val == 0 && !object_present) {
            vTaskDelay(pdMS_TO_TICKS(50)); 
            if (gpio_get_level(IR_SENSOR_GPIO) == 0) {
                buzzer_beep();
                
                // xQueueReset(xCmdQueue);
                
                printf("CMD_START_CAM\n"); 
                fflush(stdout); 
                
                object_present = true; 
            }
        }

        if (object_present && xQueueReceive(xCmdQueue, &cmd, pdMS_TO_TICKS(8000))) {
            if (cmd >= '1' && cmd <= '4') {
                buzzer_beep();
                vTaskDelay(pdMS_TO_TICKS(100)); // Khoảng nghỉ ngắn giữa 2 tiếng bíp
                buzzer_beep(); 

                int pan_target = 180; int tilt_target = 92;

                switch (cmd) {
                    case '1': pan_target = 180; tilt_target = 180; break;
                    case '2': pan_target = 0;   tilt_target = 180; break;
                    case '3': pan_target = 90;  tilt_target = 150; break; 
                    case '4': pan_target = 180; tilt_target = 0;   break;
                }

                // --- QUAY PAN ĐẾN THÙNG ---
                servo_write_angle(SERVO_PAN, pan_target);
                vTaskDelay(pdMS_TO_TICKS(1000));

                // --- QUAY TILT ĐỂ GẠT RÁC ---
                servo_write_angle(SERVO_TILT, tilt_target);
                vTaskDelay(pdMS_TO_TICKS(1000)); // Chờ rác rơi hết

                // --- QUAY TILT VỀ 90 ĐỂ ĐO MỨC RÁC ---
                servo_write_angle(SERVO_TILT, 92); 
                vTaskDelay(pdMS_TO_TICKS(2000)); // Đứng yên 2s cho khay hết rung và đo siêu âm

                // Đo khoảng cách thực tế
                int bin_full = 0;
                float dist = sensor_get_dist(ECHO_BIN1); 

                // printf("[SIEU_AM_LOG] CMD:%c | DIST_RAW: %.2f cm\n", cmd, dist);
                // fflush(stdout);
                
                // Kiểm tra nếu khoảng cách đo được nhỏ hơn ngưỡng thì báo thùng đầy
                if (dist > 0.0f && dist < DIST_THRESHOLD_FULL) {
                    bin_full = 1;
                    g_bin_full = true; // Khóa cờ
                } else {
                    g_bin_full = false; // Nhả cờ
                }


                if (bin_full == 1){
                    buzzer_long_beep();
                    printf("BIN_FULL:%c\n", cmd); // Báo Pi thùng đầy để kích hoạt xe AGV
                    fflush(stdout);               // Ép đẩy dữ liệu đi ngay không giữ trong bộ đệm RAM
                } else {
                    buzzer_beep();
                    printf("BIN_AVAILABLE:%c\n", cmd);    // Báo Pi thùng còn chỗ để Pi đóng Cam tắt luồng đi ngủ ngay
                    fflush(stdout);
                }

                // --- SAU KHI ĐO XONG MỚI QUAY PAN VỀ HOME ---
                vTaskDelay(pdMS_TO_TICKS(500));
                servo_write_angle(SERVO_PAN, 180);
                
                vTaskDelay(pdMS_TO_TICKS(1000));
                object_present = false; 
                ESP_LOGI(TAG, "Hoan tat chu ky. San sang tiep nhan rác.");
            }
        }

        if (ir_val == 1 && object_present) {
            object_present = false;
        }

        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}


void uart_rx_task(void *pvParameters) {
    char cmd;
    while (1) {
        if (uart_receive_cmd(&cmd)) {
            xQueueSend(xCmdQueue, &cmd, portMAX_DELAY);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void) {
    // Khoi tao phan cung
    servo_init_hardware(); 
    uart_init_handler(); 
    sensor_init();
    // ESP_ERROR_CHECK(battery_adc_init());
    led_control_init();
    
    xCmdQueue = xQueueCreate(10, sizeof(char));

    // Tao cac Task
    xTaskCreate(uart_rx_task, "UART_RX", 4096, NULL, 10, NULL);
    xTaskCreate(sorting_task, "SORTER", 4096, NULL, 5, NULL);
    xTaskCreate(battery_monitor_task, "BATTERY", 4096, NULL, 4, NULL);

    ESP_LOGI(TAG, "SWS System Started!");
}