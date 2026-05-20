#include "driver/gpio.h"    
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "app_config.h" 
#include "servo_control.h"
#include "sensor_handler.h"
#include "uart_handler.h"

static const char *TAG = "3AE_MAIN";
QueueHandle_t xCmdQueue; 

void sorting_task(void *pvParameters) {
    char cmd;
    bool object_present = false; 

    // 1. Đưa Robot về vị trí IDLE (Tấm gạt nằm ngang tại vị trí nhận rác)
    servo_write_angle(SERVO_PAN, 175); 
    servo_write_angle(SERVO_TILT, 90);

    vTaskDelay(pdMS_TO_TICKS(2000)); 

    while (1) {
        int ir_val = gpio_get_level(IR_SENSOR_GPIO);

        if (ir_val == 0 && !object_present) {
            vTaskDelay(pdMS_TO_TICKS(50)); 
            if (gpio_get_level(IR_SENSOR_GPIO) == 0) {
                printf("CMD_START_CAM\n"); // Báo Pi bật Camera
                object_present = true; 
            }
        }

        if (object_present && xQueueReceive(xCmdQueue, &cmd, pdMS_TO_TICKS(10))) {
            if (cmd >= '1' && cmd <= '4') {
                buzzer_beep();
                vTaskDelay(pdMS_TO_TICKS(100)); // Khoảng nghỉ ngắn giữa 2 tiếng bíp
                buzzer_beep(); 

                int pan_target = 175; int tilt_target = 90;
                int active_echo = ECHO_BIN1; 

                switch (cmd) {
                    case '1': pan_target = 175; tilt_target = 180; active_echo = ECHO_BIN1; break;
                    case '2': pan_target = 0;   tilt_target = 180; active_echo = ECHO_BIN1; break;
                    case '3': pan_target = 65;  tilt_target = 180; active_echo = ECHO_BIN1; break; 
                    case '4': pan_target = 175; tilt_target = 0;   active_echo = ECHO_BIN1; break;
                }

                // --- BƯỚC 1: QUAY PAN ĐẾN THÙNG ---
                servo_write_angle(SERVO_PAN, pan_target);
                vTaskDelay(pdMS_TO_TICKS(1000));

                // --- BƯỚC 2: QUAY TILT ĐỂ GẠT RÁC ---
                servo_write_angle(SERVO_TILT, tilt_target);
                vTaskDelay(pdMS_TO_TICKS(1000)); // Chờ rác rơi hết

                // --- BƯỚC 3: QUAY TILT VỀ 90 ĐỂ ĐO MỨC RÁC ---
                // Lúc này khay PAN vẫn đang ở trên miệng thùng rác tương ứng
                servo_write_angle(SERVO_TILT, 90); 
                vTaskDelay(pdMS_TO_TICKS(1500)); // Đứng yên 1s cho khay hết rung và đo siêu âm

                // Đo khoảng cách 
               int bin_full = 0;
               float dist = sensor_get_dist(ECHO_BIN1); 
                if (dist < DIST_THRESHOLD_FULL) {
                    bin_full = 1; 
                }

                buzzer_beep();

                if (bin_full == 1){
                    printf("BIN_FULL:%c\n", cmd); // Báo Pi thùng đầy
                }
                
                // --- BƯỚC 4: GỬI DỮ LIỆU ĐỊNH DẠNG 4 NGĂN ---
                // Chỉ gửi giá trị cho ngăn vừa đổ, các ngăn khác gửi 0.0
                // printf("BIN_LOG:%.1f|%.1f|%.1f|%.1f\n", 
                //        (cmd == '1' ? dist : 0.0), 
                //        (cmd == '2' ? dist : 0.0),
                //        (cmd == '3' ? dist : 0.0),
                //        (cmd == '4' ? dist : 0.0));

                // --- BƯỚC 4: SAU KHI ĐO XONG MỚI QUAY PAN VỀ HOME ---
                vTaskDelay(pdMS_TO_TICKS(500));
                servo_write_angle(SERVO_PAN, 175);
                
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
    
    xCmdQueue = xQueueCreate(10, sizeof(char));

    // Tao cac Task
    xTaskCreate(uart_rx_task, "UART_RX", 4096, NULL, 10, NULL);
    xTaskCreate(sorting_task, "SORTER", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "SWS System Started!");
}