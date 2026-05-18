import serial
import time
from modules.constants import SERIAL_PORT, BAUD_RATE, WASTE_COMMANDS

# Biến toàn cục đóng vai trò thay thế cho @st.cache_resource của Streamlit
_ser_instance = None

def get_esp32_connection():
    global _ser_instance
    # Nếu chưa có kết nối hoặc cổng bị đóng, tiến hành mở mới và giữ lại dùng mãi mãi
    if _ser_instance is None or not _ser_instance.is_open:
        try:
            _ser_instance = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
            time.sleep(2) # Chờ ESP32 ổn định sau khi mở cổng
        except Exception as e:
            _ser_instance = None
    return _ser_instance

def wait_for_ir_trigger():
    """Đợi tín hiệu CMD_START_CAM từ ESP32 khi IR phát hiện rác"""
    ser = get_esp32_connection()
    if not ser: return False
    
    if ser.in_waiting > 0:
        try:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if "CMD_START_CAM" in line:
                return True
        except:
            pass
    return False

def send_servo_command(label):
    ser = get_esp32_connection()
    cmd = WASTE_COMMANDS.get(label)
    if ser and cmd:
        ser.reset_input_buffer() # Xóa log cũ trong hàng đợi
        ser.write(cmd)
        print(f"--- LOG: Đã gửi mã {cmd} cho nhãn {label} ---")
        return True
    return False

def wait_for_esp32_log(timeout=15):
    """Đợi ESP32 thực hiện xong chu trình cơ khí và gửi kết quả siêu âm"""
    ser = get_esp32_connection()
    if not ser: return None
    
    start_time = time.time()
    while (time.time() - start_time) < timeout:
        if ser.in_waiting > 0:
            try:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line.startswith("BIN_LOG:"):
                    return line
            except:
                pass
        time.sleep(0.1)
    return None