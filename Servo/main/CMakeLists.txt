# Đăng ký các file nguồn và các thư viện phụ thuộc
idf_component_register(
    SRCS 
        "main.c" 
        "src/servo_control.c" 
        "src/sensor_handler.c" 
        "src/uart_handler.c"
    INCLUDE_DIRS 
        "include" 
        "."
    REQUIRES 
        esp_driver_ledc
        esp_driver_gpio      # THAY CHO "driver" ĐỂ CHẮC CHẮN NHẬN driver/gpio.h
        esp_driver_mcpwm      # Cho Servo
        esp_driver_uart       # Cho UART
        esp_timer             # Cho Siêu âm
        log                   # Cho việc xuất log ESP_LOGI
        driver
)