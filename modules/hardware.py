def send_servo_command(label):
    ser = get_esp32_connection()
    cmd = WASTE_COMMANDS.get(label)
    if ser and cmd:
        # Xóa sạch dữ liệu cũ trong buffer để đảm bảo lệnh đi ngay lập tức
        ser.reset_input_buffer() 
        ser.reset_output_buffer()
        ser.write(cmd)
        return True
    return False