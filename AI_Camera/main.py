import cv2
import time
import threading
from flask import Flask, Response, jsonify
from flask_cors import CORS

from modules.hardware import (
    send_servo_command,
    wait_for_esp32_log,
    get_esp32_connection,
)

from modules.ai_engine import (
    load_yolo_model,
    predict_waste,
)


app = Flask(__name__)
CORS(app)


streaming_frame = None
latest_snapshot = None
latest_snapshot_ts = 0
latest_snapshot_label = ""

frame_lock = threading.Lock()
is_camera_on = False
uart_rx_buffer = ""

CONF_THRESHOLD = 0.6
STABLE_FRAMES_REQ = 10
BIN_FULL_THRESHOLD = 10.0
CAMERA_INDEX = 0
MAX_SESSION_TIMEOUT = 10


def generate_mjpeg_stream():
    global streaming_frame, is_camera_on

    while True:
        with frame_lock:
            active = is_camera_on
            frame = None if streaming_frame is None else streaming_frame.copy()

        if active and frame is not None:
            ret, jpeg = cv2.imencode(
                ".jpg",
                frame,
                [int(cv2.IMWRITE_JPEG_QUALITY), 80],
            )

            if ret:
                yield (
                    b"--frame\r\n"
                    b"Content-Type: image/jpeg\r\n\r\n"
                    + jpeg.tobytes()
                    + b"\r\n"
                )

            time.sleep(0.04)
        else:
            time.sleep(0.1)


@app.route("/video_feed")
def video_feed():
    return Response(
        generate_mjpeg_stream(),
        mimetype="multipart/x-mixed-replace; boundary=frame",
    )


@app.route("/latest.jpg")
def latest_jpg():
    global latest_snapshot

    with frame_lock:
        frame = None if latest_snapshot is None else latest_snapshot.copy()

    if frame is None:
        return Response(status=204)

    ret, jpeg = cv2.imencode(
        ".jpg",
        frame,
        [int(cv2.IMWRITE_JPEG_QUALITY), 90],
    )

    if not ret:
        return Response(status=500)

    response = Response(jpeg.tobytes(), mimetype="image/jpeg")
    response.headers["Cache-Control"] = "no-store, no-cache, must-revalidate, max-age=0"
    response.headers["Pragma"] = "no-cache"
    response.headers["Expires"] = "0"

    return response


@app.route("/latest_info")
def latest_info():
    return jsonify(
        {
            "available": latest_snapshot is not None,
            "ts": latest_snapshot_ts,
            "label": latest_snapshot_label,
        }
    )


def run_flask_server():
    app.run(
        host="0.0.0.0",
        port=8085,
        threaded=True,
        use_reloader=False,
    )


def close_serial_safely(ser):
    try:
        if ser:
            ser.close()
            print("[UART] Serial port closed. It will reconnect on the next loop.")
    except Exception as e:
        print(f"[UART WARNING] Could not close serial port: {e}")


def clear_serial_buffers():
    try:
        ser = get_esp32_connection()

        if ser and ser.is_open:
            ser.reset_input_buffer()
            ser.reset_output_buffer()
            print("[UART] Serial buffers cleared.")

    except Exception as e:
        print(f"[UART WARNING] Could not clear serial buffers: {e}")


def read_uart_trigger():
    global uart_rx_buffer

    ser = None

    try:
        ser = get_esp32_connection()

        if not ser:
            return False

        if hasattr(ser, "is_open") and not ser.is_open:
            return False

        if ser.in_waiting <= 0:
            return False

        raw = ser.read(ser.in_waiting)

        if not raw:
            return False

        text = raw.decode("utf-8", errors="ignore")
        text = text.replace("\r", "").replace("\n", "").replace(" ", "")

        if text:
            print(f"[UART RX RAW] {text}")

        uart_rx_buffer += text

        if len(uart_rx_buffer) > 300:
            uart_rx_buffer = uart_rx_buffer[-300:]

        # Nhận cả chuỗi đầy đủ và chuỗi bị mất ký tự đầu
        if "CMD_START_CAM" in uart_rx_buffer or "START_CAM" in uart_rx_buffer:
            print("[UART TRIGGER] START_CAM detected.")

            uart_rx_buffer = ""

            try:
                ser.reset_input_buffer()
            except Exception:
                pass

            return True

        return False

    except Exception as e:
        print(f"[UART ERROR] Serial read failed: {e}")

        if ser is not None:
            close_serial_safely(ser)

        time.sleep(1)
        return False


def update_latest_snapshot(frame, label):
    global latest_snapshot
    global latest_snapshot_ts
    global latest_snapshot_label

    with frame_lock:
        latest_snapshot = frame.copy()
        latest_snapshot_ts = int(time.time() * 1000)
        latest_snapshot_label = label

    print("[SNAPSHOT] Latest image updated for web.")


def update_streaming_frame(frame):
    global streaming_frame

    with frame_lock:
        streaming_frame = frame.copy()


def set_camera_state(active):
    global is_camera_on
    global streaming_frame

    with frame_lock:
        is_camera_on = active

        if not active:
            streaming_frame = None


def handle_esp32_after_detection(label):
    print(f"[UART] Sending servo command: {label}")

    try:
        sent_ok = send_servo_command(label)

        if not sent_ok:
            print("[UART ERROR] Failed to send servo command.")
            return

        print("[UART] Waiting for ESP32 mechanical cycle and full-bin status...")

        # Chờ phản hồi từ ESP32 (Thời gian chờ tối đa 15 giây)
        raw_log = wait_for_esp32_log(timeout=15)

        # Mảng tên các ngăn thùng rác để hiển thị log trực quan
        names = ["Organic", "Inorganic", "Recycle", "Other"]

        # TRƯỜNG HỢP 1: Không nhận được phản hồi gì (Thùng không đầy hoặc hết chu kỳ trong im lặng)
        if not raw_log:
            print("[SENSOR DATA] Sorted successfully. No full bin detected (All bins AVAILABLE).")
            return

        # Làm sạch chuỗi ký tự nhận được (Xóa xuống dòng, khoảng trắng)
        raw_log = raw_log.replace("\r", "").replace("\n", "").strip()
        print(f"[UART RX RAW] {raw_log}")

        # TRƯỜNG HỢP 2: Phát hiện từ khóa báo đầy "BIN_FULL" từ ESP32
        if "BIN_FULL" in raw_log:
            try:
                if ":" not in raw_log:
                    print("[ERROR] Invalid BIN_FULL format: missing ':'")
                    return

                # Bóc tách lấy ký tự mã thùng rác (Ví dụ: "BIN_FULL:1" -> lấy "1")
                bin_char = raw_log.split(":", 1)[1].strip()
                
                # Chuyển đổi ký tự mã thùng sang chỉ số mảng (Ví dụ: '1' -> 0, '2' -> 1)
                bin_index = int(bin_char) - 1

                if 0 <= bin_index < 4:
                    print(f"⚠️ [ALERT] Bin {bin_index + 1} ({names[bin_index]}) is FULL! (< 10cm)")
                    
                    # --- THÊM LOGIC KÍCH HOẠT XE TỰ HÀNH TẠI ĐÂY ---
                    # 
                    #
                    #
                else:
                    print(f"[ERROR] Invalid bin index received from ESP32: {bin_char}")

            except Exception as e:
                print(f"[ERROR] Failed to parse BIN_FULL log data: {e}")
        
        # TRƯỜNG HỢP 3: Nhận được log hoàn tất chu trình thông thường từ ESP32 mà không báo đầy
        else:
            print("[SENSOR DATA] Waste sorted successfully. Target bin is still AVAILABLE.")

    except Exception as e:
        print(f"[UART ERROR] ESP32 handling failed: {e}")

        try:
            ser = get_esp32_connection()
            close_serial_safely(ser)
        except Exception:
            pass


def run_camera_ai_session(model):
    print("\n[IR TRIGGER] Trash detected. Opening camera...")

    cap = None

    try:
        cap = cv2.VideoCapture(CAMERA_INDEX)

        if not cap.isOpened():
            print("[ERROR] Cannot open camera.")
            time.sleep(0.5)
            return

        set_camera_state(True)

        # --- KHỞI TẠO LOGIC BẦU CHỌN (VOTING LOGIC) ---
        label_votes = {}       # Dictionary lưu trữ: { "Nhãn_AI": Số_Lượt_Đoán }
        collected_samples = 0   # Bộ đếm tổng số khung hình hợp lệ thu thập được
        start_session_time = time.time()
        
        # Biến lưu trữ khung hình có nhãn cuối cùng để chụp ảnh snapshot lưu web
        last_valid_frame = None

        display_frame = None
        
        while cap.isOpened():
            # Cơ chế an toàn: Nếu quá 10 giây mà không thu thập đủ số khung hình yêu cầu,
            # hệ thống sẽ tự ngắt và thực hiện bầu chọn trên những gì đã thu thập được.
            if time.time() - start_session_time > MAX_SESSION_TIMEOUT:
                print("\n[TIMEOUT] Session timeout reached. Processing collected votes...")
                break

            ret, frame = cap.read()

            if not ret:
                print("[WARNING] Cannot read frame from camera.")
                time.sleep(0.1)
                continue

            label, prob, annotated = predict_waste(model, frame)

            display_frame = annotated if annotated is not None else frame
            update_streaming_frame(display_frame)

            # Chỉ chấp nhận tính điểm bầu chọn cho những khung hình vượt ngưỡng tin cậy
            if label and prob is not None and (prob / 100) > CONF_THRESHOLD:
                # Cộng thêm 1 phiếu bầu cho nhãn này vào từ điển
                label_votes[label] = label_votes.get(label, 0) + 1
                collected_samples += 1
                last_valid_frame = display_frame.copy()

                print(
                    f"[AI] Sampling: Collected {collected_samples}/{STABLE_FRAMES_REQ} valid frames... (Current: {label})",
                    end="\r",
                )

                # Khi đã thu thập đủ tổng số mẫu cần thiết (Ví dụ: 5 hoặc 10 khung hình hợp lệ)
                if collected_samples >= STABLE_FRAMES_REQ:
                    break
            else:
                # Khung hình không đủ độ tin cậy hoặc không có rác, bỏ qua không tính điểm vote
                pass

            time.sleep(0.01)

        # --- QUY TRÌNH THỐNG KÊ VÀ TRÍCH XUẤT NHÃN CHIẾM ĐA SỐ ---
        if label_votes:
            # Dùng hàm max() bốc ra nhãn có giá trị (Value/Số phiếu) lớn nhất trong Dictionary
            final_label = max(label_votes, key=lambda k: label_votes[k])
            
            print(f"\n[AI RESULT] Voting detailed statistics: {label_votes}")
            print(f"[AI RESULT] Final Winner confirmed: {final_label} ({label_votes[final_label]}/{collected_samples} frames)")

            # Cập nhật ảnh snapshot lên web dashboard và bắn lệnh cơ khí xuống cho ESP32-S3
            snap_frame = last_valid_frame if last_valid_frame is not None else display_frame
            update_latest_snapshot(snap_frame, final_label)
            handle_esp32_after_detection(final_label)
            
        else:
            print("\n[AI RESULT] No valid labels detected above confidence threshold during this session.")

    except Exception as e:
        print(f"[CAMERA ERROR] Camera session failed: {e}")

    finally:
        if cap is not None:
            try:
                cap.release()
            except Exception as e:
                print(f"[CAMERA WARNING] Could not release camera: {e}")

        set_camera_state(False)
        clear_serial_buffers()

        print("[SYSTEM] Camera released. Back to sleeping mode.")
        print("==================================================")


def main():
    print("==================================================")
    print("SWS CORE SYSTEM - LOW POWER MODE")
    print("==================================================")

    print("[SYSTEM] Loading YOLO NCNN model...")
    model = load_yolo_model()

    if model is None:
        print("[ERROR] Model folder not found: models/waste_ncnn")
        return

    print("[SYSTEM] Model loaded successfully.")
    print("[SYSTEM] Sleeping mode: waiting for IR trigger from ESP32...")

    try:
        while True:
            is_triggered = read_uart_trigger()

            if is_triggered:
                run_camera_ai_session(model)

            time.sleep(0.05)

    except KeyboardInterrupt:
        print("\n[SYSTEM] Program stopped by user.")

        set_camera_state(False)

        try:
            ser = get_esp32_connection()
            close_serial_safely(ser)
        except Exception:
            pass


if __name__ == "__main__":
    flask_thread = threading.Thread(
        target=run_flask_server,
        daemon=True,
    )

    flask_thread.start()

    main()