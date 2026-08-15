# 🤖 ECOBOT: Hệ Thống Xe Tự Hành Tích Hợp Phân Loại Rác Tại Biên (Edge AI)

<!-- OS, Virtualization & Server -->
[![Ubuntu](https://img.shields.io/badge/Ubuntu-24.04_LTS-E95420?logo=ubuntu&logoColor=white)](https://ubuntu.com/)
[![VMware](https://img.shields.io/badge/VMware-Workstation%20Pro-607078?logo=vmware&logoColor=white)](https://www.vmware.com/)
[![Linux](https://img.shields.io/badge/Linux-Server-FCC624?logo=linux&logoColor=black)](https://www.kernel.org/)

<!-- Robotics, Navigation & Mapping -->
[![ROS 2](https://img.shields.io/badge/ROS%202-Jazzy%20%7C%20Humble-22314E?logo=ros&logoColor=white)](https://docs.ros.org/)
[![Nav2](https://img.shields.io/badge/Navigation-Nav2-3873B3?logo=ros&logoColor=white)](https://navigation.ros.org/)
[![SLAM Toolbox](https://img.shields.io/badge/SLAM-SLAM%20Toolbox-orange?logo=ros&logoColor=white)](https://github.com/SteveMacenski/slam_toolbox)
[![RViz2](https://img.shields.io/badge/Visualization-RViz2-8A2BE2?logo=ros&logoColor=white)](https://github.com/ros2/rviz)

<!-- Hardware & Embedded MCUs -->
[![Raspberry Pi](https://img.shields.io/badge/SBC-Raspberry%20Pi%204B-C51A4A?logo=raspberrypi&logoColor=white)](https://www.raspberrypi.com/)
[![ESP32-S3](https://img.shields.io/badge/MCU-ESP32--S3%20Dual--Core-E7352C?logo=espressif&logoColor=white)](https://www.espressif.com/)
[![ESP-IDF](https://img.shields.io/badge/Framework-ESP--IDF%20v5.x-E7352C?logo=espressif&logoColor=white)](https://idf.espressif.com/)
[![Sensors](https://img.shields.io/badge/Sensors-YDLIDAR%20X3%20%7C%20BNO055%209DOF-blue)](http://www.ydlidar.com/)
[![Power](https://img.shields.io/badge/Battery-LiFePO4%2012.8V-green)](https://en.wikipedia.org/wiki/Lithium_iron_phosphate_battery)

<!-- Edge AI & Computer Vision -->
[![YOLOv8](https://img.shields.io/badge/AI-YOLOv8n--cls-00FFFF?logo=ultralytics&logoColor=black)](https://ultralytics.com)
[![PyTorch](https://img.shields.io/badge/PyTorch-2.x-EE4C2C?logo=pytorch&logoColor=white)](https://pytorch.org/)
[![OpenCV](https://img.shields.io/badge/Vision-OpenCV%20%2B%20CLAHE-5C3EE8?logo=opencv&logoColor=white)](https://opencv.org/)

<!-- Programming Languages & Backend -->
[![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![C](https://img.shields.io/badge/C-11-A8B9CC?logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Python](https://img.shields.io/badge/Python-3.10%2B-3776AB?logo=python&logoColor=white)](https://www.python.org/)
[![Flask](https://img.shields.io/badge/Backend-Flask%20REST%20API-000000?logo=flask&logoColor=white)](https://flask.palletsprojects.com/)
[![JavaScript](https://img.shields.io/badge/Frontend-HTML5%20%7C%20CSS3%20%7C%20JS-F7DF1E?logo=javascript&logoColor=black)](https://developer.mozilla.org/)

<!-- Development Tools & EDA -->
[![VS Code](https://img.shields.io/badge/IDE-Visual%20Studio%20Code-007ACC?logo=visualstudiocode&logoColor=white)](https://code.visualstudio.com/)
[![EasyEDA](https://img.shields.io/badge/EDA-EasyEDA%20%7C%20JLCPCB-0066FF)](https://easyeda.com/)
[![Git](https://img.shields.io/badge/VCS-Git%20%26%20GitHub-F05032?logo=git&logoColor=white)](https://git-scm.com/)
[![License](https://img.shields.io/badge/License-MIT-brightgreen.svg)](LICENSE)

> **Đồ án tốt nghiệp Kỹ sư ngành Điện tử - Viễn thông (Chuyên ngành Máy tính - Hệ thống Nhúng)**  
> **Trường Đại học Khoa học Tự nhiên, ĐHQG-HCM (VNUHCM-US)**  
> **Sinh viên thực hiện:** Nguyễn Vũ Nhật Thành (MSSV: 22200145)  
> **Thời gian thực hiện:** 08/2026  

---

## 📌 Mục Lục
- [1. Giới Thiệu Tổng Quan](#-1-giới-thiệu-tổng-quan)
- [2. Tính Năng Nổi Bật](#-2-tính-năng-nổi-bật)
- [3. Kiến Trúc Hệ Thống](#-3-kiến-trúc-hệ-thống)
  - [3.1. Sơ Đồ Khối Tổng Thể](#31-sơ-đồ-khối-tổng-thể)
  - [3.2. Phân Tầng Phần Cứng (Hardware Layer)](#32-phân-tầng-phần-cứng-hardware-layer)
  - [3.3. Phân Tầng Phần Mềm (Software & Algorithms Layer)](#33-phân-tầng-phần-mềm-software--algorithms-layer)
- [4. Cấu Trúc Thư Mục Repository](#-4-cấu-trúc-thư-mục-repository)
- [5. Sơ Đồ Đấu Nối Phần Cứng](#-5-sơ-đồ-đấu-nối-phần-cứng)
- [6. Hướng Dẫn Cài Đặt & Vận Hành](#-6-hướng-dẫn-cài-đặt--vận-hành)
  - [6.1. Yêu Cầu Môi Trường](#61-yêu-cầu-môi-trường)
  - [6.2. Nạp Firmware cho ESP32-S3](#62-nạp-firmware-cho-esp32-s3)
  - [6.3. Khởi Chạy Hệ Thống ROS 2 & AI Trên Raspberry Pi](#63-khởi-chạy-hệ-thống-ros-2--ai-trên-raspberry-pi)
  - [6.4. Chạy Dashboard Giám Sát Web](#64-chạy-dashboard-giám-sát-web)
- [7. Kết Quả Thực Nghiệm & Đánh Giá](#-7-kết-quả-thực-nghiệm--đánh-giá)
  - [7.1. Độ Chính Xác Phân Loại AI](#71-độ-chính-xác-phân-loại-ai)
  - [7.2. Thời Gian Đáp Ứng Chu Trình](#72-thời-gian-đáp-ứng-chu-trình)
  - [7.3. Đánh Giá Tự Hành & Điều Hướng (SLAM/Nav2)](#73-đánh-giá-tự-hành--điều-hướng-slamnav2)
  - [7.4. Đánh Giá Năng Lượng](#74-đánh-giá-năng-lượng)
- [8. Hướng Phát Triển Tương Lai](#-8-hướng-phát-triển-tương-lai)
- [9. Thông Tin Liên Hệ & Tác Quyền](#-9-thông-tin-liên-hệ--tác-quyền)

---

## 📖 1. Giới Thiệu Tổng Quan

Trong các khuôn viên công cộng, trường đại học, bệnh viện và tòa nhà văn phòng, công tác thu gom và phân loại rác thải phần lớn vẫn được thực hiện theo phương thức thủ công. Việc này tiêu tốn nhiều nhân lực, hiệu quả phân loại tại nguồn chưa cao và tiềm ẩn nhiều rủi ro sức khỏe cho công nhân vệ sinh khi tiếp xúc trực tiếp với môi trường độc hại.

**ECOBOT-01** là một giải pháp robot di động tự hành tích hợp hệ thống phân loại rác thông minh tại biên (Edge AI):
1. **Phân loại rác tại nguồn:** Tự động nhận diện và gạt rác vào 4 ngăn chứa chuyên biệt (*Rác tái chế, Rác hữu cơ, Rác vô cơ, Rác khác*) bằng mô hình thị giác máy tính Deep Learning siêu nhẹ kết hợp cơ cấu servo 2 bậc tự do (Pan-Tilt).
2. **Tự động định vị & di chuyển (AMR):** Tích hợp 2D LiDAR, 9-DOF IMU và 4 bánh động cơ DC Encoder để quét lập bản đồ (SLAM), tự định vị và tự hành tránh vật cản tĩnh/động dựa trên ROS 2 & Navigation2.
3. **Giám sát & Điều phối từ xa:** Tích hợp IoT Dashboard thời gian thực, đo mức pin, cảnh báo đầy thùng bằng cảm biến siêu âm HC-SR04, tự động kích hoạt điều hướng đưa rác về điểm tập kết trung tâm khi đầy.

---

## ⭐ 2. Tính Năng Nổi Bật

- 🎯 **Edge AI Vision Classification**: Sử dụng mô hình `YOLOv8n-cls` tối ưu hóa NCNN/ARM trên CPU Raspberry Pi 4 kết hợp thuật toán tiền xử lý ảnh thích nghi **CLAHE** (Contrast Limited Adaptive Histogram Equalization) trên không gian màu LAB, đạt độ chính xác trung bình **89.0%** (trong đó rác tái chế đạt **96.0%**).
- ⚡ **Cơ Chế Tiết Kiệm Năng Lượng Standby**: Tích hợp cảm biến tiệm cận quang hồng ngoại **E18-D80NK** tại cửa nạp rác. Hệ thống chỉ kích hoạt camera và chạy suy luận AI khi có rác đưa vào, giúp tiết kiệm tối đa năng lượng pin.
- 🚗 **Khung Gầm Di Động 4WD Độc Lập**: Sử dụng 4 động cơ DC giảm tốc kim loại **JGB37-520** tích hợp Magnetic Encoder, điều khiển vòng kín bằng thuật toán **PID (20 Hz)** và mạch cầu H công suất cao **BTS7960** (43A).
- 🗺️ **SLAM & Tự Hành Điều Hướng Thông Minh**: Chạy `slam_toolbox` (Graph-based SLAM với Loop Closure) và `Nav2` (DWB/TEB Controller) trên nền tảng **ROS 2 Jazzy / Ubuntu 24.04**, cho phép quét bản đồ 2D sắc nét và né tránh vật cản động mượt mà.
- 🔄 **Cơ Cấu Gạt & Xoay Phân Phối 4 Ngăn**: Sử dụng 2 động cơ servo kim loại **MG996R** (mô-men xoắn 11 kg·cm) gồm 1 trục xoay định hướng (Pan) và 1 trục hạ máng/gạt (Tilt).
- 📊 **Web Dashboard Quản Trị Đầy Đủ**: Giao diện điều khiển Web Responsive (Flask REST API + WebSockets), hiển thị trạng thái kết nối, mức pin LiFePO4, trạng thái 4 ngăn rác, hình ảnh nhận diện mới nhất có nhãn xác suất và lịch sử vận hành.
- 🛡️ **Nguồn Pin An Toàn & Tách Biệt Nhiễu**: Trang bị khối pin Lithium Iron Phosphate (**LiFePO4 12.8V - 6Ah**) tích hợp BMS 100A, các khối hạ áp **XL4005 (5V/5A)** và **LM2596S (5V/3A)** phân tách độc lập giữa mạch công suất, vi điều khiển logic và servo nhằm chống nhiễu EMI tối đa.

---

## 🏗️ 3. Kiến Trúc Hệ Thống

### 3.1. Sơ Đồ Khối Tổng Thể

```text
                               +-----------------------------+
                               |   Central Web Dashboard     |
                               |   (Admin Web GUI / Server)  |
                               +--------------+--------------+
                                              ^
                                  HTTP / JSON | WebSockets
                                              v
+------------------------------------------------------------------------------------------+
|  KHỐI ĐIỀU KHIỂN TRUNG TÂM CẤP CAO (Raspberry Pi 4 - Ubuntu 24.04 / ROS 2 Jazzy)          |
|  - Edge AI Inference (YOLOv8n-cls + CLAHE)                                               |
|  - Navigation Stack (Nav2: Global/Local Planner, AMCL, Costmaps, Recovery Behaviors)    |
|  - SLAM Mapping (slam_toolbox - Graph SLAM & Loop Closure)                               |
|  - Flask API Server & Stream Dữ Liệu Lịch Sử                                            |
+----------------------+------------------------------------+------------------------------+
                       ^                                    ^
        USB 3.0 Serial | Scan Data           USB 3.0 Camera | RGB Frame
                       v                                    v
            +---------------------+              +---------------------+
            |  YDLIDAR X3 (360°)  |              |  Webcam HD 1080p    |
            |  Laser Distance 8m  |              |  (Top-down View)    |
            +---------------------+              +---------------------+
                       ^
                       | UART / USB Serial (Baudrate 115200)
                       v
+------------------------------------------+    +------------------------------------------+
|  MCU 1: ESP32-S3 (ĐIỀU KHIỂN TỰ HÀNH)    |    |  MCU 2: ESP32-S3 (PHÂN LOẠI & GIÁM SÁT)  |
|  - Thuật toán PID Closed-Loop (20 Hz)    |    |  - Quét cảm biến quang E18-D80NK (Wakeup)|
|  - Đọc xung 4x Magnetic Encoders (PCNT)  |    |  - Điều khiển 2x Servo MG996R (Pan-Tilt) |
|  - Giao tiếp I2C đọc IMU GY-BNO055 (9DOF)|    |  - Mảng 4x Cảm biến siêu âm HC-SR04      |
|  - Phát xung PWM điều khiển BTS7960      |    |  - Đo điện áp pin ADC qua mạch chia áp   |
+----------------------+-------------------+    +--------------------+---------------------+
                       |                                             |
         +-------------+-------------+                 +-------------+-------------+
         |                           |                 |                           |
+-----------------+         +-----------------+  +-----------------+         +-----------------+
| 2x Mạch BTS7960 |         | 4x Động cơ      |  | 2x Servo        |         | 4x HC-SR04      |
| Driver 43A      | ------> | JGB37-520 DC    |  | MG996R Metal    |         | & Còi Buzzer 5V |
+-----------------+         +-----------------+  +-----------------+         +-----------------+

                                     [ KHỐI NGUỒN ]
          LiFePO4 Battery 12.8V 6Ah  ==>  XL4005 (5V/5A cho Pi 4)
                                     ==>  LM2596 (5V/3A Logic/Sensors)
                                     ==>  LM2596 (5V/3A Khối Servo)
```

### 3.2. Phân Tầng Phần Cứng (Hardware Layer)

| Tên Thiết Bị / Module | Thông Số Kỹ Thuật Chính | Vai Trò Trong Hệ Thống |
| :--- | :--- | :--- |
| **Raspberry Pi 4 Model B** | Broadcom BCM2711 4-Core Cortex-A72 @ 1.5GHz, 4GB LPDDR4 | Bộ não cấp cao chạy ROS 2, suy luận AI YOLOv8 và Web Server |
| **2x ESP32-S3 DevKit** | Dual-core Xtensa® LX7 @ 240MHz, 16MB Flash, 8MB PSRAM | 1 kit điều khiển động cơ di chuyển, 1 kit điều khiển cơ cấu phân loại |
| **YDLIDAR X3 Pro** | Quét 360°, Tần số 4000 Ranging/s, Tầm xa 0.12 - 8m, UART | Quét môi trường lập bản đồ SLAM và tránh vật cản |
| **GY-BNO055 9-DOF IMU** | Tích hợp Cortex-M0 tính Sensor Fusion, giao tiếp I2C | Cung cấp góc hướng Yaw chính xác, bù trượt bánh xe |
| **4x Động cơ JGB37-520** | DC 12V 107 RPM, Hộp số kim loại 1:168, Encoder Hall 1848 xung/vòng | Dẫn động 4 bánh xe di chuyển vòng kín |
| **2x Mạch BTS7960** | Cầu H MOSFET dòng đỉnh 43A, tần số PWM lên đến 25kHz | Điều khiển công suất cho 2 cụm động cơ trái/phải |
| **2x Servo MG996R** | Bánh răng kim loại, lực kéo 11 kg·cm tại 5V, góc quay 180° | Xoay mâm phân phối rác và hạ máng gạt |
| **E18-D80NK IR Sensor** | Cảm biến quang hồng ngoại NPN-NO, khoảng cách 3 - 80cm | Nhận diện rác đưa vào khay để đánh thức AI |
| **4x HC-SR04 Ultrasonic** | Đo khoảng cách 2 - 400cm, góc quét < 15°, 40kHz | Đo chiều sâu rác trong 4 ngăn chứa độc lập |
| **Pin LiFePO4 12.8V 6Ah** | Chu kỳ sạc xả > 2000 lần, dòng xả liên tục lên đến 100A (BMS) | Nguồn năng lượng chính cho toàn bộ hệ thống |
| **XL4005 Buck Converter** | Đầu vào 9-36V, ngõ ra 5.2V / 5A (25W) hiệu suất cao | Cung cấp nguồn riêng ổn định cho Raspberry Pi 4 |
| **2x LM2596S Buck** | Ngõ ra 5V / 3A điều chỉnh được, tích hợp lọc tụ chống nhiễu | Hạ áp cấp nguồn độc lập cho Logic MCU và Servos |

### 3.3. Phân Tầng Phần Mềm (Software & Algorithms Layer)

1. **Phần mềm cấp cao (Raspberry Pi 4)**:
   - **Hệ điều hành**: Ubuntu 24.04 LTS (Noble Numbat).
   - **Framework Robot**: ROS 2 Jazzy Jalisco.
   - **SLAM Package**: `slam_toolbox` (Graph-based SLAM, Cartographer hỗ trợ).
   - **Navigation Stack**: `Nav2` (Costmaps tĩnh/động, AMCL Particle Filter, DWB Controller, Nav2 Behavior Trees).
   - **Edge AI Classification**: Ultralytics YOLOv8n-cls (xuất định dạng NCNN/ONNX ARM-optimized).
   - **Xử lý ảnh**: OpenCV + bộ lọc CLAHE (Contrast Limited Adaptive Histogram Equalization) trên kênh L (không gian màu LAB).
   - **Web Server Backend**: Python Flask, Gunicorn, REST API, JSON serialization.
2. **Phần mềm cấp thấp (ESP32-S3 MCUs)**:
   - **Môi trường phát triển**: ESP-IDF / PlatformIO trong VS Code.
   - **Thuật toán điều khiển**: Closed-loop PID Controller kết hợp bộ lọc chống trôi vi phân (Anti-windup).
   - **Đọc Encoder**: Ngoại vi phần cứng PCNT (Pulse Counter) xử lý quadrature pulse Phase A & B.
   - **Giao tiếp liên bộ xử lý**: Giao thức UART Frame đồng bộ hai chiều có kiểm tra tính hợp lệ dữ liệu.

---

## 📂 4. Cấu Trúc Thư Mục Repository

```plaintext
ecobot-01/
├── assets/                          # Hình ảnh minh họa, sơ đồ, tài liệu tham khảo
│   ├── cad_dimensions/              # Bản vẽ kích thước 2D/3D
│   ├── circuit_schematics/          # Sơ đồ nguyên lý mạch
│   └── pcb_layouts/                 # Layout mạch in Altium / EasyEDA
├── firmware/                        # Mã nguồn vi điều khiển ESP32-S3
│   ├── esp32_navigation_controller/ # Firmware điều khiển 4 động cơ, Encoder, IMU, PID
│   │   ├── src/
│   │   │   ├── pid_controller.c
│   │   │   ├── encoder_pcnt.c
│   │   │   ├── bno055_i2c.c
│   │   │   └── main.c
│   │   └── CMakeLists.txt
│   └── esp32_sorting_controller/    # Firmware phân loại rác, cảm biến IR, siêu âm, servo
│       ├── src/
│       │   ├── servo_control.c
│       │   ├── ultrasonic_sensor.c
│       │   ├── battery_adc.c
│       │   └── main.c
│       └── CMakeLists.txt
├── ros2_ws/                         # Workspace ROS 2 (Jazzy/Humble)
│   └── src/
│       ├── ecobot_bringup/          # Launch files khởi chạy tổng thể
│       ├── ecobot_description/      # File mô tả robot URDF / XACRO
│       ├── ecobot_navigation/       # Config Nav2 (costmap, planner, controller parameters)
│       └── ecobot_slam/             # Config SLAM Toolbox (slam_params.yaml)
├── edge_ai/                         # Mã nguồn mô hình phân loại rác
│   ├── dataset_preprocessing/       # Script chuẩn hóa ảnh & bộ lọc CLAHE
│   ├── training/                    # Script train YOLOv8n-cls trên PyTorch
│   ├── inference/                   # Pipeline suy luận đa khung hình (Multi-frame voting)
│   └── weights/                     # Trọng số tối ưu (.pt, .onnx, .bin/.param NCNN)
├── web_dashboard/                   # Giao diện Web quản trị thời gian thực
│   ├── static/
│   │   ├── css/style.css
│   │   └── js/main.js
│   ├── templates/index.html
│   └── app.py                       # Flask Web Server
├── hardware/                        # File Gerber, BOM linh kiện thiết kế mạch
│   ├── gerber_agv_controller/
│   └── gerber_sorting_controller/
├── docs/                            # Báo cáo toàn văn (.pdf) và Slide thuyết trình
│   ├── 22200145_NguyenVuNhatThanh_DATN.pdf
│   └── Slide_Presentation_DATN.pdf
├── LICENSE
└── README.md
```

---

## 🔌 5. Sơ Đồ Đấu Nối Phần Cứng

### 5.1. Bảng Kết Nối ESP32-S3 (AGV Navigation Controller)

| Thiết Bị | Chân Thiết Bị | Chân ESP32-S3 | Chức Năng |
| :--- | :--- | :--- | :--- |
| **IMU GY-BNO055** | SDA / SCL | GPIO 8 / GPIO 9 | Giao tiếp dữ liệu I2C |
| **IMU GY-BNO055** | VIN / GND | 3V3 / GND | Nguồn cung cấp IMU |
| **Encoder Bánh Trước Trái** | Phase A / Phase B | GPIO 18 / GPIO 17 | Đọc xung tốc độ (PCNT) |
| **Encoder Bánh Sau Trái** | Phase A / Phase B | GPIO 1 / GPIO 2 | Đọc xung tốc độ (PCNT) |
| **Encoder Bánh Trước Phải** | Phase A / Phase B | GPIO 12 / GPIO 13 | Đọc xung tốc độ (PCNT) |
| **Encoder Bánh Sau Phải** | Phase A / Phase B | GPIO 10 / GPIO 11 | Đọc xung tốc độ (PCNT) |
| **BTS7960 Bên Phải** | RPWM / LPWM | GPIO 4 / GPIO 5 | Xung PWM điều khiển chiều quay/tốc độ |
| **BTS7960 Bên Trái** | RPWM / LPWM | GPIO 6 / GPIO 7 | Xung PWM điều khiển chiều quay/tốc độ |
| **Raspberry Pi 4** | TX / RX (hoặc USB-C) | GPIO 43 / GPIO 44 | Giao tiếp dữ liệu Odometry / cmd_vel |

### 5.2. Bảng Kết Nối ESP32-S3 (Sorting & Sensor Controller)

| Thiết Bị | Chân Thiết Bị | Chân ESP32-S3 | Chức Năng |
| :--- | :--- | :--- | :--- |
| **Cảm biến hồng ngoại E18** | Output Signal | GPIO 6 | Nhận tín hiệu phát hiện có rác đặt vào khay |
| **4x Cảm biến siêu âm HC-SR04** | TRIG (Ngăn 1, 2, 3, 4) | GPIO 10, 11, 12, 13 | Kích xung phát sóng siêu âm đo từng ngăn |
| **4x Cảm biến siêu âm HC-SR04** | ECHO | GPIO 9 | Nhận xung phản xạ đo độ sâu ngăn rác |
| **Servo Hạ Máng (Tilt)** | PWM Signal | GPIO 37 | Điều khiển gạt rác xuống thùng |
| **Servo Mâm Xoay (Pan)** | PWM Signal | GPIO 38 | Xoay định hướng vào 1 trong 4 ngăn |
| **Mạch Cầu Phân Áp Đo Pin** | Analog Output | GPIO 1 (ADC1) | Đọc điện áp pin LiFePO4 qua ADC |
| **Còi Báo Buzzer 5V** | Control Pin | GPIO 14 | Kích âm báo hiệu đầy rác / pin yếu |

---

## 🚀 6. Hướng Dẫn Cài Đặt & Vận Hành

### 6.1. Yêu Cầu Môi Trường
- **Host PC / Raspberry Pi**: Ubuntu 24.04 (hoặc Ubuntu 22.04), cài đặt **ROS 2 Jazzy** hoặc **ROS 2 Humble**.
- **Python**: Phiên bản >= 3.10 với các gói: `opencv-python`, `ultralytics`, `flask`, `pyserial`, `numpy`.
- **Phát triển nhúng**: VS Code + Tiện ích **ESP-IDF Extension** (v5.x).

### 6.2. Nạp Firmware cho ESP32-S3

1. Mở thư mục firmware trên VS Code:
   ```bash
   cd firmware/esp32_navigation_controller
   ```
2. Cấu hình phần cứng và chọn cổng kết nối:
   ```bash
   idf.py set-target esp32s3
   idf.py menuconfig
   ```
3. Biên dịch và nạp code:
   ```bash
   idf.py build flash -p /dev/ttyUSB0 monitor
   ```
*(Thực hiện tương tự cho thư mục `firmware/esp32_sorting_controller` trên cổng `/dev/ttyUSB1`)*.

### 6.3. Khởi Chạy Hệ Thống ROS 2 & AI Trên Raspberry Pi

1. Clone repository và build ROS 2 workspace:
   ```bash
   mkdir -p ~/ecobot_ws/src && cd ~/ecobot_ws/src
   git clone https://github.com/nhatthanhlgf/ecobot-01.git .
   cd ~/ecobot_ws
   colcon build --symlink-install
   source install/setup.bash
   ```

2. **Chế độ 1: Khởi động Lập bản đồ (SLAM Mapping)**:
   ```bash
   ros2 launch ecobot_bringup ecobot_slam.launch.py
   ```
   *(Sử dụng bàn phím hoặc tay cầm điều khiển xe di chuyển quanh khuôn viên để quét bản đồ trên RViz2, sau đó lưu bản đồ bằng `ros2 run nav2_map_server map_saver_cli -f my_map`)*.

3. **Chế độ 2: Khởi động Chế độ Tự hành & Phân loại thông minh**:
   ```bash
   ros2 launch ecobot_bringup ecobot_navigation.launch.py map:=my_map.yaml
   ```

### 6.4. Chạy Dashboard Giám Sát Web

1. Điều hướng đến thư mục web dashboard:
   ```bash
   cd ~/ecobot_ws/src/web_dashboard
   python3 -m pip install -r requirements.txt
   python3 app.py --host=0.0.0.0 --port=5000
   ```
2. Mở trình duyệt và truy cập vào địa chỉ: `http://<IP_RASPBERRY_PI>:5000`. Đăng nhập tài khoản quản trị để theo dõi mức pin, tình trạng rác và điều phối tự hành.

---

## 📊 7. Kết Quả Thực Nghiệm & Đánh Giá

### 7.1. Độ Chính Xác Phân Loại AI (YOLOv8n-cls)

Thực nghiệm đánh giá trên **100 mẫu thử thực tế** thuộc các nhóm rác thải sinh hoạt phổ biến:

| Nhóm Rác Thải | Số Mẫu Thử | Nhận Diện Đúng | Nhận Diện Sai | Tỷ Lệ Chính Xác (%) |
| :--- | :---: | :---: | :---: | :---: |
| **Rác Tái Chế (Chai nhựa, lon nhôm, hộp sữa)** | 25 | 24 | 1 | **96.0%** |
| **Rác Hữu Cơ (Vỏ trái cây, lá cây, thức ăn thừa)** | 25 | 23 | 2 | **92.0%** |
| **Rác Vô Cơ (Túi nilon biến dạng, hộp xốp)** | 25 | 22 | 3 | **88.0%** |
| **Rác Khác (Bao bì hỗn hợp, rác không xác định)** | 25 | 20 | 5 | **80.0%** |
| **TỔNG CỘNG** | **100** | **89** | **11** | **89.0%** |

### 7.2. Thời Gian Đáp Ứng Chu Trình Phân Loại

```text
+------------------------+------------------------------------+--------------------------+------------------------------------+
| Giai đoạn 1: Kích hoạt | Giai đoạn 2: Camera & Edge AI     | Giai đoạn 3: Cơ cấu Servo| Giai đoạn 4: Đo siêu âm & Về Home  |
| Cảm biến IR (~0.3s)    | Khởi tạo & Suy luận 10 frame (~4.5s)| Xoay & Gạt rác (~2.51s)  | Kiểm tra đầy & Khóa chốt (~3.3s)   |
+------------------------+------------------------------------+--------------------------+------------------------------------+
  ===> Tổng thời gian hoàn thành 1 chu trình: 10.61s (Khi chưa đầy) / 13.41s (Khi phát hiện đầy thùng).
```

### 7.3. Đánh Giá Tự Hành & Điều Hướng (SLAM/Nav2)
- **Lập bản đồ SLAM**: Tạo lập thành công bản đồ lưới 2D diện tích thực nghiệm hành lang kích thước lớn, các đường bao tường và góc cạnh rõ nét nhờ thuật toán Graph-based Loop Closure.
- **Tránh vật cản động**: Khi có người đi bộ cắt ngang hành trình, bộ điều khiển cục bộ (Local Planner) ngay lập tức giảm tốc độ về 0 m/s trong < 0.2s và tái lập quỹ đạo vòng tránh an toàn.

### 7.4. Đánh Giá Năng Lượng (Pin LiFePO4 12V 6Ah - 57.6 Wh khả dụng)
- **Chế độ Chờ / Giám sát**: Công suất tiêu thụ ~15W => Thời gian hoạt động liên tục: **~3.84 giờ**.
- **Chế độ Phân loại rác liên tục**: Công suất tiêu thụ ~20W => Thời gian hoạt động: **~2.88 giờ**.
- **Chế độ Tự hành di chuyển liên tục**: Công suất tiêu thụ => Thời gian hoạt động: **~1.28 giờ**.
- **Chu trình hỗn hợp thực tế**: Công suất tiêu thụ trung bình => Thời gian hoạt động: **~2.19 giờ**.

---

## 🔮 8. Hướng Phát Triển Tương Lai

- [ ] **Tích hợp hộp chụp kín & Đèn LED trợ sáng**: Chuẩn hóa điều kiện ánh sáng 100% trong khay chụp để loại bỏ hiện tượng lóa sáng ngoài trời.
- [ ] **Mở rộng tập dữ liệu AI**: Bổ sung các nhóm rác thải y tế, phế liệu công nghiệp nhẹ và rác thải nguy hại.
- [ ] **Nâng cấp cảm biến tránh điểm mù**: Bổ sung Depth Camera (RGB-D) và cảm biến chống rơi mép sàn (Cliff sensors).
- [ ] **Tối ưu cơ khí & Dung tích chứa**: Mở rộng dung tích thùng rác, nâng cấp cụm bánh xe Mecanum tải trọng cao và tích hợp cơ cấu ép rác cơ học.
- [ ] **Tích hợp Cloud Fleet Management**: Quản lý đa robot tập trung qua điện toán đám mây (Cloud Fleet / Multi-Robot Navigation).

---

## 👨‍💻 9. Thông Tin Liên Hệ & Tác Quyền

- **Tác giả:** Nguyễn Vũ Nhật Thành (Kỹ sư Điện tử - Viễn thông, HCMUS)
- **Email:** `nhatthanh30072004@gmail.com`
- **Trường Đại học:** Trường Đại học Khoa học Tự nhiên - ĐHQG-HCM

---
*Dự án được bảo vệ thành công tại Hội đồng Đồ án Tốt nghiệp - Khoa Điện tử Viễn thông, Trường Đại học Khoa học Tự nhiên TP.HCM (08/2026).*
