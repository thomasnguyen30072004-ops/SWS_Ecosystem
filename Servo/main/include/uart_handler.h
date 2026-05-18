#ifndef UART_HANDLER_H
#define UART_HANDLER_H

// Khởi tạo Driver UART và cấu hình bộ đệm
void uart_init_handler(void);

// Hàm kiểm tra và đọc lệnh từ PC gửi xuống (Trả về 1 nếu có lệnh mới)
int uart_receive_cmd(char *out_char);

#endif