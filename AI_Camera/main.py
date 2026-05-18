import cv2
import time
from modules.hardware import send_servo_command, wait_for_esp32_log, get_esp32_connection
from modules.ai_engine import load_yolo_model, predict_waste

# --- THÊM ĐOẠN FLASK STREAM TRÊN RAM NÀY ĐỂ TƯƠNG THÍCH VỚI FILE MAIN.JS CỦA ÔNG ---
import threading
from flask import Flask, Response
from flask_cors import CORS

app = Flask(__name__)
CORS(app)  # Cho phép Web của ông kéo luồng ảnh về vẽ lên Canvas thoải mái

streaming_frame = None
frame_lock = threading.Lock()
is_camera_on = False

def generate_mjpeg_stream():
    global streaming_frame, is_camera_on
    while True:
        with frame_lock:
            active = is_camera_on
            frame = streaming_frame
        
        if active and frame is not None:
            # Mã hóa ảnh sang JPG trực tiếp trên RAM để bắn lên Web nhanh nhất
            ret, jpeg = cv2.imencode('.jpg', frame, [int(cv2.IMWRITE_JPEG_QUALITY), 80])
            if ret:
                yield (b'--frame\r\n'
                       b'Content-Type: image/jpeg\r\n\r\n' + jpeg.tobytes() + b'\r\n')
            time.sleep(0.04)  # Giới hạn ~25 FPS tránh tràn băng thông Pi
        else:
            time.sleep(0.1)

@app.route('/video_feed')
def video_feed():
    return Response(generate_mjpeg_stream(), mimetype='multipart/x-mixed-replace; boundary=frame')

def run_flask_server():
    # Mở đúng cổng 8085 
    app.run(host='0.0.0.0', port=8085, threaded=True, use_reloader=False)
# ---------------------------------------------------------------------------------

# --- Cấu hình hệ thống ---
CONF_THRESHOLD = 0.6
STABLE_FRAMES_REQ = 5
BIN_FULL_THRESHOLD = 10.0  # Ngưỡng báo đầy: Khoảng cách từ cảm biến đến mặt rác < 10cm

def main():
    global streaming_frame, is_camera_on  # Khai báo sử dụng biến toàn cục để chia sẻ dữ liệu
    
    print("==================================================")
    print("🌿 SWS CORE SYSTEM - CHẾ ĐỘ TIẾT KIỆM NĂNG LƯỢNG")
    print("==================================================")
    
    # 1. Nạp Model NCNN (Chỉ nạp 1 lần duy nhất vào RAM lúc khởi động)
    print("[HỆ THỐNG] Đang nạp mô hình YOLO NCNN...")
    model = load_yolo_model()
    if model is None:
        print("[LỖI] Không tìm thấy thư mục model NCNN tại models/waste_ncnn!")
        return
    print("[HỆ THỐNG] Nạp model thành công! Hệ thống sẵn sàng.")
    print("[HỆ THỐNG] Chế độ ngủ đông 💤: Đang lắng nghe tín hiệu IR từ ESP32...")

    try:
        while True:
            # GIAI ĐOẠN 1: Ngủ đông - Chỉ quét cổng UART để kiểm tra lệnh từ cảm biến IR
            ser = get_esp32_connection()
            is_triggered = False
            
            if ser and ser.in_waiting > 0:
                try:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    if "CMD_START_CAM" in line:
                        is_triggered = True
                except:
                    pass

            # GIAI ĐOẠN 2: Khi IR phát hiện vật cản -> Đánh thức Camera và AI
            if is_triggered:
                print("\n🔔 [IR TRIGGER] Phát hiện rác! Đang cấp nguồn bật Camera...")
                cap = cv2.VideoCapture(0)  # Bắt đầu mở và luồng dữ liệu từ camera
                
                if not cap.isOpened():
                    print("[LỖI] Không thể kết nối với Camera!")
                    continue
                
                # BÁO HIỆU CAMERA LÊN NGUỒN ĐỂ SERVER PHÁT LUỒNG NGẦM
                with frame_lock:
                    is_camera_on = True
                
                stable_counter = 0
                last_label = ""
                start_session_time = time.time()
                max_session_timeout = 10  # BẢO VỆ: Quá 10 giây không nhận diện xong sẽ tự ngắt Cam
                
                while cap.isOpened():
                    # Cơ chế an toàn tránh treo Camera làm cạn pin xe
                    if time.time() - start_session_time > max_session_timeout:
                        print("\n⏳ [TIMEOUT] Không chốt được nhãn ổn định trong 10s. Tự động đóng cam...")
                        break
                        
                    ret, frame = cap.read()
                    if not ret:
                        print("[CẢNH BÁO] Mất khung hình từ Camera!")
                        time.sleep(0.1)
                        continue

                    # Chạy YOLOv11 NCNN nhận diện vật thể (Lấy thêm biến ảnh annotated đã vẽ khung)
                    label, prob, annotated = predict_waste(model, frame)

                    # ĐẨY ẢNH ĐÃ XỬ LÝ LÊN RAM ĐỂ FILE MAIN.JS CỦA ÔNG HỐT VỀ VẼ LÊN CANVAS
                    display_frame = annotated if annotated is not None else frame
                    with frame_lock:
                        streaming_frame = display_frame

                    if label and prob is not None and (prob / 100) > CONF_THRESHOLD:
                        if label == last_label:
                            stable_counter += 1
                        else:
                            stable_counter = 1
                            last_label = label

                        progress = int((stable_counter / STABLE_FRAMES_REQ) * 100)
                        print(f"🔍 [AI KHẢO SÁT] Đang phân tích: {label} ({progress}%)", end="\r")

                        # Khi nhãn rác đã đạt độ ổn định liên tục qua 5 khung hình
                        if stable_counter >= STABLE_FRAMES_REQ:
                            print(f"\n🚀 [XÁC NHẬN NHÃN] Kết quả: {label} ({prob:.1f}%)")
                            
                            # GIAI ĐOẠN 3: Gạt rác và đồng bộ dữ liệu siêu âm HC-SR04
                            print(f"📤 [UART] Gửi lệnh gạt [{label}] xuống ESP32...")
                            if send_servo_command(label):
                                print("⏳ [UART] Đang đợi chu trình cơ khí hoàn tất và phản hồi đo đầy...")
                                
                                raw_log = wait_for_esp32_log(timeout=15)
                                if raw_log:
                                    try:
                                        data = raw_log.split(":")[1].split("|")
                                        d_vals = [float(val) for val in data]
                                        
                                        print("📊 [DỮ LIỆU CẢM BIẾN] Trạng thái các ngăn chứa hiện tại:")
                                        names = ["Hữu cơ", "Vô cơ", "Tái chế", "Khác"]
                                        
                                        for i in range(4):
                                            if d_vals[i] > 0:
                                                # Logic kiểm tra khoảng cách mặt rác d < 10cm để báo đầy
                                                status = "🔴 ĐẦY! (CẦN XỬ LÝ)" if d_vals[i] < BIN_FULL_THRESHOLD else "🟢 Còn chỗ"
                                                print(f"   -> Ngăn {i+1} ({names[i]}): {d_vals[i]} cm | Trạng thái: {status}")
                                    except:
                                        print("[LỖI] Sai cấu trúc dữ liệu phản hồi từ board cơ khí.")
                                else:
                                    print("❌ [LỖI] Quá thời gian phản hồi (Timeout) từ ESP32!")
                            
                            # Xử lý xong lượt rác này -> Thoát ngay vòng lặp Camera
                            break
                    else:
                        stable_counter = 0

                    time.sleep(0.01)  # Giảm tải cho CPU Pi 4 trong luồng nhận diện
                
                # NGẮT NGUỒN CAMERA: Giải phóng tài nguyên phần cứng ngay lập tức
                cap.release()
                
                # DỌN SẠCH BỘ NHỚ RAM VÀ KHÓA STREAM KHI CAMERA TẮT
                with frame_lock:
                    is_camera_on = False
                    streaming_frame = None
                    
                print("[HỆ THỐNG] Đã giải phóng Camera thành công. Quay lại chế độ ngủ đông... 💤")
                print("==================================================")

            # Chu kỳ quét UART khi đang ngủ đông - Giảm tải CPU bằng cách tăng thời gian chờ giữa các lần quét
            time.sleep(0.05)

    except KeyboardInterrupt:
        print("\n👋 [HỆ THỐNG] Đã ngắt chương trình chủ động.")

if __name__ == "__main__":
    # Luồng phụ: Khởi chạy Flask Server phát luồng ảnh ngầm lên cổng 8085
    flask_thread = threading.Thread(target=run_flask_server, daemon=True)
    flask_thread.start()
    
    # Luồng chính: Khởi chạy bộ điều khiển AI + UART gốc nguyên bản của ông
    main()