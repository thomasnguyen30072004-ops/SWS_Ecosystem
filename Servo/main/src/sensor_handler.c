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

#define BATTERY_TOTAL_SAMPLES       1000  // Gom đủ 1000 mẫu mới đẩy dữ liệu lên Pi một lần
#define BATTERY_SAMPLE_DELAY_MS     300   // 300ms * 1000 mẫu = 300,000ms = ĐÚNG 5 PHÚT TRÒN
#define BATTERY_SUDDEN_DROP_LIMIT   10    // Ngưỡng bảo vệ hủy mẫu nếu tụt áp ảo >= 10%

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
static bool s_battery_low = false; // Cờ báo hiệu Trạng thái Pin yếu ra bên ngoài (để đèn Vàng bật)

static bool battery_adc_calibration_init(void)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = s_battery_adc_unit,
        .chan = s_battery_adc_channel,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    esp_err_t ret = adc_cali_create_scheme_curve_fitting(&cali_config, &s_battery_cali_handle);
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
    gpio_set_level(BUZZER_GPIO, !BUZZER_ACTIVE_LEVEL);

    // 3. Siêu âm HC-SR04
    gpio_set_direction(TRIG_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_BIN1, GPIO_MODE_INPUT);
    gpio_set_level(TRIG_GPIO, 0); 
}

esp_err_t battery_adc_init(void)
{
    esp_err_t ret = adc_oneshot_io_to_channel(BATTERY_ADC_GPIO, &s_battery_adc_unit, &s_battery_adc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO%d is not a valid ADC pin: %s", BATTERY_ADC_GPIO, esp_err_to_name(ret));
        return ret;
    }

    adc_oneshot_unit_init_cfg_t unit_config = {
        .unit_id = s_battery_adc_unit,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ret = adc_oneshot_new_unit(&unit_config, &s_battery_adc_handle);
    if (ret != ESP_OK) {
        return ret;
    }

    adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ret = adc_oneshot_config_channel(s_battery_adc_handle, s_battery_adc_channel, &channel_config);
    if (ret != ESP_OK) {
        return ret;
    }

    s_battery_adc_calibrated = battery_adc_calibration_init();
    return s_battery_adc_calibrated ? ESP_OK : ESP_ERR_NOT_SUPPORTED;
}

uint8_t battery_percent_from_vout(uint16_t vout_mv)
{
    if (vout_mv <= s_battery_curve[0].vout_mv) return s_battery_curve[0].percent;
    if (vout_mv >= s_battery_curve[BATTERY_CURVE_SIZE - 1].vout_mv) return s_battery_curve[BATTERY_CURVE_SIZE - 1].percent;

    for (size_t i = 1; i < BATTERY_CURVE_SIZE; ++i) {
        const battery_curve_point_t *lower = &s_battery_curve[i - 1];
        const battery_curve_point_t *upper = &s_battery_curve[i];

        if (vout_mv <= upper->vout_mv) {
            const uint32_t voltage_position = vout_mv - lower->vout_mv;
            const uint32_t voltage_span = upper->vout_mv - lower->vout_mv;
            const uint32_t percent_span = upper->percent - lower->percent;

            return (uint8_t)(lower->percent + ((voltage_position * percent_span) + (voltage_span / 2U)) / voltage_span);
        }
    }
    return 0;
}

esp_err_t battery_read(battery_data_t *data)
{
    if (data == NULL || s_battery_adc_handle == NULL || !s_battery_adc_calibrated) return ESP_ERR_INVALID_STATE;

    int raw_sum = 0;
    int valid_samples = 0;

    for (int i = 0; i < BATTERY_ADC_SAMPLE_COUNT; ++i) {
        int raw = 0;
        esp_err_t ret = adc_oneshot_read(s_battery_adc_handle, s_battery_adc_channel, &raw);
        if (ret == ESP_OK) {
            raw_sum += raw;
            ++valid_samples;
        }
        esp_rom_delay_us(200);
    }

    if (valid_samples == 0) return ESP_ERR_TIMEOUT;

    const int raw_average = (raw_sum + (valid_samples / 2)) / valid_samples;
    int calibrated_mv = 0;
    adc_cali_raw_to_voltage(s_battery_cali_handle, raw_average, &calibrated_mv);

    float corrected_mv = ((float)calibrated_mv * BATTERY_ADC_GAIN) + BATTERY_ADC_OFFSET_MV;
    if (corrected_mv < 0.0f) corrected_mv = 0.0f;

    const uint16_t vout_mv = (uint16_t)(corrected_mv + 0.5f);
    data->raw = raw_average;
    data->vout_mv = vout_mv;
    data->vout_v = corrected_mv / 1000.0f;
    data->vin_v = data->vout_v * BATTERY_DIVIDER_FACTOR;
    data->percent = battery_percent_from_vout(vout_mv);
    data->below_stop_threshold = (vout_mv <= BATTERY_STOP_VOUT_MV);

    return ESP_OK;
}

// Hàm getter để file main.c gọi ra check trạng thái kích LED Vàng
bool battery_is_low(void)
{
    return s_battery_low;
}

//  5 PHÚT - 1000 MẪU - TRỘN CÒI HÚ VÀ LED VÀNG < 30%
void battery_monitor_task(void *pvParameters)
{
    (void)pvParameters;
    uint32_t mv_accumulator = 0;
    int valid_samples_counted = 0;
    
    uint8_t last_confirmed_pct = 0;
    bool is_first_calculation = true;
    uint32_t beep_tick_counter = 0; // Đếm chu kỳ nhịp hú còi báo động không chặn task

    ESP_LOGI("BATTERY", "Unified Battery Task Started! (5-Min Cycle / Drop Protection / RGB Sync)");

    while (1) {
        battery_data_t battery;
        esp_err_t ret = battery_read(&battery);

        if (ret == ESP_OK) {
            uint16_t current_inst_mv = battery.vout_mv;
            uint8_t current_inst_pct = battery.percent;

            if (is_first_calculation) {
                last_confirmed_pct = current_inst_pct;
                is_first_calculation = false;
            }

            // Nếu phần trăm tức thời dưới 30%, gạt cờ s_battery_low lên NGAY LẬP TỨC
            if (current_inst_pct < 30) {
                s_battery_low = true; // Kích hoạt driver LED chuyển sang màu Vàng ngay ở chu kỳ sau
            } else {
                s_battery_low = false;
            }

            // Hủy mẫu nếu dính sụt áp động cơ đột ngột >= 10%
            if (last_confirmed_pct > current_inst_pct && (last_confirmed_pct - current_inst_pct) >= BATTERY_SUDDEN_DROP_LIMIT) {
                ESP_LOGW("BATTERY", "⚠️ Phát hiện sụt dòng ảo >= 10%%! Hủy chu kỳ hiện tại, quét lại từ đầu...");
                mv_accumulator = 0;
                valid_samples_counted = 0;
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }

            // Tích lũy nếu mẫu sạch
            mv_accumulator += current_inst_mv;
            valid_samples_counted++;

            // KHỐI HÚ CÒI BÁO ĐỘNG (BÍP KÉP CÁCH NHAU MỖI 6 GIÂY)
            if (s_battery_low) {
                if (beep_tick_counter == 0) {
                    // Thực hiện tiếng hú bíp kép đặc trưng của ông
                    gpio_set_level(BUZZER_GPIO, BUZZER_ACTIVE_LEVEL); 
                    vTaskDelay(pdMS_TO_TICKS(150)); 
                    gpio_set_level(BUZZER_GPIO, !BUZZER_ACTIVE_LEVEL); 
                    vTaskDelay(pdMS_TO_TICKS(100)); 
                    gpio_set_level(BUZZER_GPIO, BUZZER_ACTIVE_LEVEL); 
                    vTaskDelay(pdMS_TO_TICKS(150)); 
                    gpio_set_level(BUZZER_GPIO, !BUZZER_ACTIVE_LEVEL); 

                    // 6000ms / 300ms (mỗi chu kỳ delay) = Khóa đếm ngược chặn lại đúng 20 tick loop tiếp theo
                    beep_tick_counter = 20; 
                } else {
                    beep_tick_counter--;
                }
            } else {
                beep_tick_counter = 0;
            }

            // KHỐI XỬ LÝ CHU KỲ SẠCH ĐỦ 1000 MẪU (SAU MỖI 5 PHÚT)
            if (valid_samples_counted >= BATTERY_TOTAL_SAMPLES) {
                uint16_t average_cycle_mv = mv_accumulator / BATTERY_TOTAL_SAMPLES;
                uint8_t average_cycle_pct = battery_percent_from_vout(average_cycle_mv);

                // Thuật toán lượng hóa: Ép dứt khoát về các mốc cách nhau 5%
                uint8_t quantized_pct = (average_cycle_pct / 5) * 5;
                if (quantized_pct > 100) quantized_pct = 100;

                float average_cycle_vout_v = (float)average_cycle_mv / 1000.0f;
                float average_cycle_vin_v = average_cycle_vout_v * BATTERY_DIVIDER_FACTOR;

                // Xuất log đúng cấu trúc phân tách '|' để Raspberry Pi 4 bóc tách không lỗi
                printf("BATTERY:VIN=%.3f|VOUT=%.3f|PERCENT=%u%%\n", 
                       average_cycle_vin_v, average_cycle_vout_v, quantized_pct);
                fflush(stdout); 

                // Nếu chạm ngưỡng dừng vật lý (20% - 2454mV), bắn log khẩn cấp để Pi gọi xe AMR cứu hộ
                if (average_cycle_mv <= BATTERY_STOP_VOUT_MV) {
                    printf("BATTERY_LOW:VIN=%.3f|VOUT=%.3f|PERCENT=%u%%\n", 
                           average_cycle_vin_v, average_cycle_vout_v, quantized_pct);
                    fflush(stdout);
                }

                last_confirmed_pct = average_cycle_pct;
                mv_accumulator = 0;
                valid_samples_counted = 0;
            }
        }

        // Nhịp độ 300ms rải đều 1000 mẫu nằm trọn vẹn bên trong 5 phút
        vTaskDelay(pdMS_TO_TICKS(BATTERY_SAMPLE_DELAY_MS));
    }
}

void buzzer_beep(void) {
    gpio_set_level(BUZZER_GPIO, BUZZER_ACTIVE_LEVEL); 
    vTaskDelay(pdMS_TO_TICKS(200)); 
    gpio_set_level(BUZZER_GPIO, !BUZZER_ACTIVE_LEVEL); 
}

void buzzer_long_beep(void) {
    gpio_set_level(BUZZER_GPIO, BUZZER_ACTIVE_LEVEL); 
    vTaskDelay(pdMS_TO_TICKS(3000)); 
    gpio_set_level(BUZZER_GPIO, !BUZZER_ACTIVE_LEVEL); 
}

float sensor_get_dist(int echo_pin) {
    float dist_sum = 0.0f;
    int valid_samples = 0;

    for (int i = 0; i < 5; i++) {
        gpio_set_level(TRIG_GPIO, 1);
        esp_rom_delay_us(10);
        gpio_set_level(TRIG_GPIO, 0);

        uint64_t start_time = esp_timer_get_time();
        while (gpio_get_level(echo_pin) == 0) {
            if ((esp_timer_get_time() - start_time) > 30000) goto skip_sample;
        }
        uint64_t t1 = esp_timer_get_time();

        while (gpio_get_level(echo_pin) == 1) {
            if ((esp_timer_get_time() - t1) > 30000) goto skip_sample;
        }
        uint64_t t2 = esp_timer_get_time();

        uint64_t pulse_duration = t2 - t1;
        if (pulse_duration < MIN_PULSE_US) goto skip_sample; 

        float sample_dist = (float)pulse_duration * 0.0343f / 2.0f;
        dist_sum += sample_dist;
        valid_samples++;

    skip_sample:
        vTaskDelay(pdMS_TO_TICKS(20)); 
    }

    if (valid_samples == 0) return -1.0f;
    return dist_sum / (float)valid_samples; 
}