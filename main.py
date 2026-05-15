import streamlit as st
import cv2
import time
from modules.constants import CSS_STYLE
from modules.hardware import send_servo_command, wait_for_esp32_log, get_esp32_connection
from modules.ai_engine import load_yolo_model, predict_waste

st.set_page_config(page_title="SWS Ecosystem - 3AE", layout="wide")
st.markdown(CSS_STYLE, unsafe_allow_html=True)

model = load_yolo_model()

# Khởi tạo dữ liệu 4 ngăn
if 'bin_data' not in st.session_state: 
    st.session_state.bin_data = {"d1": 0.0, "d2": 0.0, "d3": 0.0, "d4": 0.0}

st.markdown("<h1 class='main-title'>🌿 Smart Waste Sorting (3AE)</h1>", unsafe_allow_html=True)

col_cam, col_info = st.columns([1.5, 1])

with col_cam:
    st.subheader("📡 AI View")
    frame_place = st.empty()

with col_info:
    st.subheader("🤖 Robot Status")
    status_place = st.empty()
    st.markdown("---")
    m1, m2 = st.columns(2)
    m3, m4 = st.columns(2)
    b = st.session_state.bin_data
    # Hiển thị 4 mức rác
    m1.metric("Hữu cơ", f"{b['d1']}cm")
    m2.metric("Vô cơ", f"{b['d2']}cm")
    m3.metric("Tái chế", f"{b['d3']}cm")
    m4.metric("Khác", f"{b['d4']}cm")

cap = cv2.VideoCapture(0)

while cap.isOpened():
    ret, frame = cap.read()
    if not ret: break
    
    frame_place.image(cv2.cvtColor(frame, cv2.COLOR_BGR2RGB), use_column_width=True)

    # Lắng nghe IR Trigger
    ser = get_esp32_connection()
    if ser and ser.in_waiting > 0:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if "CMD_START_CAM" in line:
            # Chạy logic nhận diện và gửi lệnh tương tự local_manager.py
            # (Thành có thể copy logic loop ổn định nhãn từ local_manager vào đây)
            st.toast("Phát hiện rác!")
            # ... thực hiện nhận diện và gửi lệnh ...
            st.rerun()

    time.sleep(0.01)