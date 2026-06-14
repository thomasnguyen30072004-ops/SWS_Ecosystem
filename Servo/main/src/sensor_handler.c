#include "sensor_handler.h"
#include "app_config.h"
#include "driver/gpio.h"
#include "driver/ledc.h" 
#include "esp_timer.h"

// Bộ lọc số giới hạn vật lý của HC-SR04 để triệt tiêu xung ma (Ghost Pulse)
#define MIN_PULSE_US  115   // Tương đương ~2cm. Xung ngắn hơn mức này chắc chắn là nhiễu sụt dòng.

void sensor_init(void) {
    // 1. IR Sensor (NPN - Pullup)
    gpio_config_t ir_cfg = { 
        .pin_bit_mask = (1ULL << IR_SENSOR_GPIO), 
        .mode = GPIO_MODE_INPUT, 
        .pull_up_en = GPIO_PULLUP_ENABLE 
    };
    gpio_config(&ir_cfg);

    // 2. Khởi tạo cho Buzzer Module (Đồng bộ linh hoạt Active High/Low)
    gpio_reset_pin(BUZZER_GPIO);
    gpio_set_direction(BUZZER_GPIO, GPIO_MODE_OUTPUT);
    
    // Tắt còi khi khởi động: Dùng logic đảo (NOT) của mức Active để chống rò tiếng kêu
    gpio_set_level(BUZZER_GPIO, !BUZZER_ACTIVE_LEVEL);

    // 3. Siêu âm HC-SR04
    gpio_set_direction(TRIG_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_BIN1, GPIO_MODE_INPUT);
    gpio_set_level(TRIG_GPIO, 0); // Đảm bảo chân kích ở mức thấp ban đầu
}

void buzzer_beep(void) {
    // Bật còi theo mức kích cấu hình
    gpio_set_level(BUZZER_GPIO, BUZZER_ACTIVE_LEVEL); 
    
    // Giữ tiếng bíp trong 200ms
    vTaskDelay(pdMS_TO_TICKS(200)); 
    
    // Tắt còi dứt khoát
    gpio_set_level(BUZZER_GPIO, !BUZZER_ACTIVE_LEVEL); 
}

float sensor_get_dist(int echo_pin) {
    float dist_sum = 0.0f;
    int valid_samples = 0;

    // Tiến hành quét 5 lần liên tục 
    for (int i = 0; i < 5; i++) {
        // Kích xung Trig (10us)
        gpio_set_level(TRIG_GPIO, 1);
        esp_rom_delay_us(10);
        gpio_set_level(TRIG_GPIO, 0);

        // 1. Đợi Echo lên cao (Start time)
        uint64_t start_time = esp_timer_get_time();
        while (gpio_get_level(echo_pin) == 0) {
            if ((esp_timer_get_time() - start_time) > 30000) { 
                goto skip_sample; // Lỗi lỏng dây -> Bỏ qua mẫu này
            }
        }
        uint64_t t1 = esp_timer_get_time();

        // 2. Đợi Echo xuống thấp (End time)
        while (gpio_get_level(echo_pin) == 1) {
            if ((esp_timer_get_time() - t1) > 30000) { 
                goto skip_sample; // Kẹt chân -> Bỏ qua mẫu này
            }
        }
        uint64_t t2 = esp_timer_get_time();

        // 3. Tính toán độ rộng xung dữ liệu
        uint64_t pulse_duration = t2 - t1;
        
        // CHỐT CHẶN: Diệt sạch xung ma < 115us giống hệt file test thành công của ông
        if (pulse_duration < 115) {
            goto skip_sample; 
        }

        // Tính khoảng cách của mẫu hiện tại
        float sample_dist = (float)pulse_duration * 0.0343f / 2.0f;
        dist_sum += sample_dist;
        valid_samples++;

    skip_sample:
        vTaskDelay(pdMS_TO_TICKS(20)); // Nghỉ ngắn 20ms giữa các lần bắn để sóng âm cũ tan hết
    }

    // --- QUY TRÌNH THỐNG KÊ KẾT QUẢ ---
    if (valid_samples == 0) {
        return -1.0f; // Nếu cả 5 lần đo đều dính nhiễu/lỗi -> Báo hỏng mạch an toàn
    }

    // Trả về giá trị trung bình cộng của các khung hình sạch
    return dist_sum / (float)valid_samples; 
}