#include "sensor_handler.h"
#include "app_config.h"

#include <stddef.h>
#include <stdio.h>

#include "driver/gpio.h"
#include "driver/ledc.h" 
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SENSOR_HANDLER";

// Bộ lọc số giới hạn vật lý của HC-SR04 để triệt tiêu xung ma (Ghost Pulse)
#define MIN_PULSE_US  115   // Tương đương ~2cm. Xung ngắn hơn mức này chắc chắn là nhiễu sụt dòng.


typedef struct {
    uint16_t vout_mv;
    uint8_t percent;
} battery_curve_point_t;

/*
 * Bảng được tính trực tiếp với:
 * Vout = Vin * 4657 / (19630 + 4657)
 *
 * Các điểm được sắp xếp từ thấp đến cao để nội suy tuyến tính.
 */
static const battery_curve_point_t s_battery_curve[] = {
    {2378,   0}, // Vin 12.4 V
    {2416,   5}, // Vin 12.6 V
    {2435,  10}, // Vin 12.7 V
    {2454,  20}, // Vin 12.8 V - ngưỡng dừng
    {2474,  35}, // Vin 12.9 V
    {2493,  50}, // Vin 13.0 V
    {2512,  65}, // Vin 13.1 V
    {2531,  80}, // Vin 13.2 V
    {2550,  90}, // Vin 13.3 V
    {2569, 100}, // Vin 13.4 V
};

#define BATTERY_CURVE_SIZE \
    (sizeof(s_battery_curve) / sizeof(s_battery_curve[0]))

// Static variables for battery ADC handling
static adc_unit_t s_battery_adc_unit;
static adc_channel_t s_battery_adc_channel;
static adc_cali_handle_t s_battery_cali_handle;
static adc_oneshot_unit_handle_t s_battery_adc_handle;
static bool s_battery_adc_calibrated = false;
static bool s_battery_low = false;

static bool battery_adc_calibration_init(void)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = s_battery_adc_unit,
        .chan = s_battery_adc_channel,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    esp_err_t ret = adc_cali_create_scheme_curve_fitting(
        &cali_config,
        &s_battery_cali_handle
    );

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "ADC calibration: curve fitting enabled");
        return true;
    }

    ESP_LOGE(TAG, "Cannot initialize ADC calibration: %s", esp_err_to_name(ret));
    return false;
#else
    ESP_LOGE(TAG, "ADC curve-fitting calibration is not supported");
    return false;
#endif
}

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

/////////////////////////////////////////

esp_err_t battery_adc_init(void)
{
    esp_err_t ret = adc_oneshot_io_to_channel(
        BATTERY_ADC_GPIO,
        &s_battery_adc_unit,
        &s_battery_adc_channel
    );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO%d is not a valid ADC pin: %s",
                 BATTERY_ADC_GPIO, esp_err_to_name(ret));
        return ret;
    }

    adc_oneshot_unit_init_cfg_t unit_config = {
        .unit_id = s_battery_adc_unit,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ret = adc_oneshot_new_unit(&unit_config, &s_battery_adc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Cannot create ADC unit: %s", esp_err_to_name(ret));
        return ret;
    }

    adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ret = adc_oneshot_config_channel(
        s_battery_adc_handle,
        s_battery_adc_channel,
        &channel_config
    );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Cannot configure battery ADC channel: %s",
                 esp_err_to_name(ret));
        return ret;
    }

    s_battery_adc_calibrated = battery_adc_calibration_init();
    if (!s_battery_adc_calibrated) {
        ESP_LOGE(TAG, "Battery ADC is disabled because calibration failed");
        return ESP_ERR_NOT_SUPPORTED;
    }

    ESP_LOGI(TAG,
             "Battery ADC ready: GPIO%d, ADC unit %d, channel %d",
             BATTERY_ADC_GPIO,
             (int)s_battery_adc_unit,
             (int)s_battery_adc_channel);

    return ESP_OK;
}

uint8_t battery_percent_from_vout(uint16_t vout_mv)
{
    if (vout_mv <= s_battery_curve[0].vout_mv) {
        return s_battery_curve[0].percent;
    }

    if (vout_mv >= s_battery_curve[BATTERY_CURVE_SIZE - 1].vout_mv) {
        return s_battery_curve[BATTERY_CURVE_SIZE - 1].percent;
    }

    for (size_t i = 1; i < BATTERY_CURVE_SIZE; ++i) {
        const battery_curve_point_t *lower = &s_battery_curve[i - 1];
        const battery_curve_point_t *upper = &s_battery_curve[i];

        if (vout_mv <= upper->vout_mv) {
            const uint32_t voltage_position = vout_mv - lower->vout_mv;
            const uint32_t voltage_span = upper->vout_mv - lower->vout_mv;
            const uint32_t percent_span = upper->percent - lower->percent;

            // Nội suy và làm tròn về phần trăm gần nhất
            return (uint8_t)(
                lower->percent +
                ((voltage_position * percent_span) + (voltage_span / 2U)) /
                    voltage_span
            );
        }
    }

    return 0;
}

esp_err_t battery_read(battery_data_t *data)
{
    if (data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (s_battery_adc_handle == NULL || !s_battery_adc_calibrated) {
        return ESP_ERR_INVALID_STATE;
    }

    int raw_sum = 0;
    int valid_samples = 0;

    for (int i = 0; i < BATTERY_ADC_SAMPLE_COUNT; ++i) {
        int raw = 0;
        esp_err_t ret = adc_oneshot_read(
            s_battery_adc_handle,
            s_battery_adc_channel,
            &raw
        );

        if (ret == ESP_OK) {
            raw_sum += raw;
            ++valid_samples;
        } else if (ret != ESP_ERR_TIMEOUT) {
            return ret;
        }

        esp_rom_delay_us(200);
    }

    if (valid_samples == 0) {
        return ESP_ERR_TIMEOUT;
    }

    const int raw_average =
        (raw_sum + (valid_samples / 2)) / valid_samples;

    int calibrated_mv = 0;
    esp_err_t ret = adc_cali_raw_to_voltage(
        s_battery_cali_handle,
        raw_average,
        &calibrated_mv
    );
    if (ret != ESP_OK) {
        return ret;
    }

    float corrected_mv =
        ((float)calibrated_mv * BATTERY_ADC_GAIN) + BATTERY_ADC_OFFSET_MV;

    if (corrected_mv < 0.0f) {
        corrected_mv = 0.0f;
    }

    const uint16_t vout_mv = (uint16_t)(corrected_mv + 0.5f);
    const float vout_v = corrected_mv / 1000.0f;

    data->raw = raw_average;
    data->vout_mv = vout_mv;
    data->vout_v = vout_v;
    data->vin_v = vout_v * BATTERY_DIVIDER_FACTOR;
    data->percent = battery_percent_from_vout(vout_mv);
    data->below_stop_threshold = (vout_mv <= BATTERY_STOP_VOUT_MV);

    // Giả lập pin đầy (Vin ~ 13.3V)
    // data->vout_mv = 2550; data->percent = 90; data->below_stop_threshold = false;
    
    // Giả lập PIN YẾU CHẠY CÒI HÚ (Vin ~ 12.6V < Ngưỡng dừng 12.8V)
    // data->vout_mv = 2416; data->percent = 5; data->below_stop_threshold = true;

    return ESP_OK;
}

bool battery_is_low(void)
{
    return s_battery_low;
}


void battery_monitor_task(void *pvParameters)
{
    (void)pvParameters;
    uint8_t low_count = 0;
    bool low_message_sent = false;
    
    uint32_t read_counter = 0; // Đếm chu kỳ đọc dữ liệu pin (10 giây)
    uint32_t beep_counter = 0; // Đếm chu kỳ hú còi báo pin yếu (5 giây)

    while (1) {
        // 1. Kiểm tra đọc và gửi dữ liệu điện áp pin định kỳ mỗi 10 giây (BATTERY_READ_PERIOD_MS)
        if (read_counter == 0) {
            battery_data_t battery;
            esp_err_t ret = battery_read(&battery);

            if (ret == ESP_OK) {
                // ÉP XẢ BỘ ĐỆM: Thêm fflush để đẩy dữ liệu pin ngay lên RAM của Pi 4 xử lý
                printf("BATTERY:VIN=%.3f|VOUT=%.3f|PERCENT=%u\n",
                       battery.vin_v, battery.vout_v, (unsigned)battery.percent);
                fflush(stdout);

                if (!s_battery_low) {
                    if (battery.vout_mv <= BATTERY_STOP_VOUT_MV) {
                        if (low_count < BATTERY_LOW_CONFIRM_COUNT) ++low_count;
                    } else {
                        low_count = 0;
                    }

                    if (low_count >= BATTERY_LOW_CONFIRM_COUNT) {
                        s_battery_low = true;
                        if (!low_message_sent) {
                            printf("BATTERY_LOW:VIN=%.3f|VOUT=%.3f|PERCENT=%u\n",
                                   battery.vin_v, battery.vout_v, (unsigned)battery.percent);
                            fflush(stdout);
                            low_message_sent = true;
                        }
                    }
                } else if (battery.vout_mv >= BATTERY_RECOVER_VOUT_MV) {
                    s_battery_low = false;
                    low_count = 0;
                    low_message_sent = false;

                    printf("BATTERY_RECOVERED:VIN=%.3f|VOUT=%.3f|PERCENT=%u\n",
                           battery.vin_v, battery.vout_v, (unsigned)battery.percent);
                    fflush(stdout);
                }
            } else {
                ESP_LOGW(TAG, "Battery ADC read failed: %s", esp_err_to_name(ret));
            }
        }

        // Tăng bộ đếm chu kỳ đọc pin (Đạt mức 10 chu kỳ = 10 giây)
        read_counter = (read_counter + 1) % (BATTERY_READ_PERIOD_MS / 1000);

        // 2. XỬ LÝ ÂM THANH YÊU CẦU: Cảnh báo hết pin bằng còi hú (beep beep => 5s => beep beep)
        if (s_battery_low) {
            if (beep_counter == 0) {
                // Phát âm bíp kép liên tiếp dứt khoát
                gpio_set_level(BUZZER_GPIO, BUZZER_ACTIVE_LEVEL); 
                vTaskDelay(pdMS_TO_TICKS(150)); 
                gpio_set_level(BUZZER_GPIO, !BUZZER_ACTIVE_LEVEL); 
                vTaskDelay(pdMS_TO_TICKS(100)); // Khoảng nghỉ ngắn giữa 2 tiếng bíp
                gpio_set_level(BUZZER_GPIO, BUZZER_ACTIVE_LEVEL); 
                vTaskDelay(pdMS_TO_TICKS(150)); 
                gpio_set_level(BUZZER_GPIO, !BUZZER_ACTIVE_LEVEL); 

                beep_counter = 2; // Khóa đếm ngược chặn lại đúng 2 giây tiếp theo mới hú lại
            } else {
                beep_counter--;
            }
        } else {
            beep_counter = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Đập nhịp tuần tự 1 giây một lần
    }
}
/////////////////////////////////////////

void buzzer_beep(void) {
    // Bật còi theo mức kích cấu hình
    gpio_set_level(BUZZER_GPIO, BUZZER_ACTIVE_LEVEL); 
    
    // Giữ tiếng bíp trong 200ms
    vTaskDelay(pdMS_TO_TICKS(200)); 
    
    // Tắt còi dứt khoát
    gpio_set_level(BUZZER_GPIO, !BUZZER_ACTIVE_LEVEL); 
}

void buzzer_long_beep(void) {
    gpio_set_level(BUZZER_GPIO, BUZZER_ACTIVE_LEVEL); 
    vTaskDelay(pdMS_TO_TICKS(3000)); // Kéo dài thời gian hú còi lên 1000ms
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