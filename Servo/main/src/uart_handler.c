#include "uart_handler.h"
#include "app_config.h"
#include "driver/uart.h"

void uart_init_handler(void) {
    uart_config_t cfg = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    // Cài đặt driver với bộ đệm 1024 bytes
    uart_driver_install(UART_PORT_NUM, 1024, 0, 0, NULL, 0);
    uart_param_config(UART_PORT_NUM, &cfg);
}

int uart_receive_cmd(char *out_char) {
    uint8_t data;
    // Đọc 1 byte từ cổng UART, chờ tối đa 10ms
    int len = uart_read_bytes(UART_PORT_NUM, &data, 1, 10 / portTICK_PERIOD_MS);
    if (len > 0) {
        *out_char = (char)data;
        return 1;
    }
    return 0;
}