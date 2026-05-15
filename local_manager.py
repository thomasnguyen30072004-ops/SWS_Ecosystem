import cv2
import time
import csv
import os
from modules.hardware import send_servo_command, wait_for_esp32_log, get_esp32_connection
from modules.ai_engine import load_yolo_model, predict_waste

# Cấu hình log
LOG_FILE = "bin_history.csv"
CONF_THRESHOLD = 0.6
STABLE_FRAMES_REQ = 5

# Tạo file log nếu chưa có
if not os.path.exists(LOG_FILE):
    with open(LOG_FILE, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(["Time", "Label", "Bin1_cm", "Bin2_cm", "Bin3_cm", "Bin4_cm"])

def main_loop():
    model = load_yolo_model()
    cap = cv2.VideoCapture(0) # Camera chính của Pi 4
    
    print("--- SWS EDGE: System Started (Headless Mode) ---")

    while cap.isOpened():
        ser = get_esp32_connection()
        if ser and ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            
            if "CMD_START_CAM" in line:
                print("🔔 IR Triggered! Starting AI...")
                
                stable_cnt = 0
                found_label = ""
                
                # Nhận diện cho đến khi nhãn ổn định
                while stable_cnt < STABLE_FRAMES_REQ:
                    ret, frame = cap.read()
                    if not ret: break
                    
                    label, prob, _ = predict_waste(model, frame)
                    if label and (prob/100) > CONF_THRESHOLD:
                        if label == found_label: stable_cnt += 1
                        else: found_label = label; stable_cnt = 1
                    else: stable_cnt = 0
                
                print(f"🚀 Detected: {found_label}. Sending command...")

                if send_servo_command(found_label):
                    # Đợi ESP32 gửi BIN_LOG sau khi đo siêu âm
                    raw_log = wait_for_esp32_log(timeout=15)
                    if raw_log:
                        try:
                            data = raw_log.split(":")[1].split("|")
                            d_vals = [float(val) for val in data]
                            
                            # Lưu vào CSV
                            with open(LOG_FILE, 'a', newline='') as f:
                                writer = csv.writer(f)
                                writer.writerow([time.strftime("%H:%M:%S"), found_label] + d_vals)
                            print(f"✅ Logged: {found_label} | Data: {d_vals}")
                        except:
                            print("Error parsing BIN_LOG")
                
                print("--- Cycle Finished. Waiting for next object... ---")
        
        time.sleep(0.05) # Tránh nghẽn CPU

if __name__ == "__main__":
    main_loop()