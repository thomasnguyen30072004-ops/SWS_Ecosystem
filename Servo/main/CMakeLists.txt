# Đăng ký các file nguồn và các thư viện phụ thuộc
idf_component_register(
    SRCS 
        "main.c" 
        "src/servo_control.c" 
        "src/sensor_handler.c" 
        "src/uart_handler.c"
        "src/led_control.c"
    INCLUDE_DIRS 
        "include" 
        "."
    REQUIRES 
        esp_driver_ledc
        esp_driver_gpio      
        esp_driver_mcpwm      # Cho Servo
        esp_driver_uart       # Cho UART
        esp_timer             # Cho Siêu âm
        esp_adc               # ADC
        log                   # Cho việc xuất log ESP_LOGI
        driver
)