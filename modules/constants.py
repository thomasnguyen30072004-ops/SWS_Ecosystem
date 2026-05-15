import platform

# --- MAPPING LỆNH ---
WASTE_COMMANDS = {
    "RAC_HUU_CO": b'1',
    "RAC_VO_CO": b'2',
    "RAC_TAI_CHE": b'3',
    "RESIDUAL_OTHER": b'4'
}

# Tự động nhận diện hệ điều hành
if platform.system() == "Windows":
    SERIAL_PORT = 'COM17'
else:
    # Trên Ubuntu Pi 4, cổng thường là /dev/ttyUSB0 hoặc /dev/ttyACM0
    SERIAL_PORT = '/dev/ttyUSB0' 

BAUD_RATE = 115200