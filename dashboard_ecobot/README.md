# 📊 SWS Web Dashboard - Hệ Thống Giám Sát Xe Tự Hành & Phân Loại Rác Thông Minh

Web Dashboard đóng vai trò là trung tâm quản lý, cấu hình và giám sát từ xa (Central Monitoring Hub) của hệ thống. Giao diện cung cấp cho người vận hành một cái nhìn toàn diện, trực quan về toàn bộ trạng thái động học, năng lượng và tiến trình phân loại rác của xe tự hành theo thời gian thực.

---

## 🌟 Các Tính Năng Cốt Lõi Trên Giao Diện

### 1. Giám Sát Năng Lượng & Dung Lượng Lưu Trữ

- **Cập nhật dung lượng Pin:** Hiển thị trực quan mức pin hiện tại của hệ thống kèm cảnh báo nhấp nháy đỏ khi năng lượng chạm ngưỡng nguy cấp.
- **Mức độ đầy của các ngăn rác:** Theo dõi thời gian thực trạng thái của 4 hộc chứa độc lập (**Hữu cơ - Vô cơ - Tái chế - Rác khác**). Hệ thống tự động chuyển sang trạng thái màu **ĐỎ (FULL)** khi ngăn chứa đầy để người quản lý kịp thời thu gom.

### 2. Quản Lý Dữ Liệu Phân Loại & Lưu Trữ Ảnh AI

- **Thống kê kết quả AI:** Hiển thị chi tiết nhãn phân loại rác vừa được xử lý qua camera AI biên (YOLOv8 NCNN).
- **Lưu trữ dữ liệu:** Toàn bộ hình ảnh bằng chứng (Snapshot có vẽ khung bounding box) kèm mốc thời gian nhận diện được tự động lưu trữ, phân loại một cách có tổ chức vào các thư mục quản lý trên máy tính phục vụ việc truy xuất dữ liệu sau này.

### 3. Theo Dõi Hành Trình Di Chuyển (Robot Kinematics)

- **Giám sát tuyến đường:** Theo dõi chặt chẽ lộ trình di chuyển của xe tự hành dọc theo hai tuyến đường chính: Từ vị trí thu gom đến khu tập kết rác trung tâm, và tuyến đường tự động quay trở lại trạm xuất phát.
- **Telemetry động học:** Hiển thị tức thời Chế độ chạy (Tự động/Thủ công), Hướng di chuyển (Tiến, Lùi, Quay đầu) và Vận tốc thực tế của xe.

### 4. Khung Nhật Ký Hệ Thống (System Logs Console)

- Hộp thoại dạng dòng lệnh (Console) tự động cuộn, ghi lại dòng chảy sự kiện liên tục theo mốc thời gian (Timestamp) thực, giúp người quản lý dễ dàng nắm bắt các hành vi cơ khí và trạng thái hệ thống mà không cần can thiệp sâu vào mã nguồn.

---

## 🛠️ Công Nghệ Sử Dụng (Tech Stack)

Để tối ưu hóa hiệu năng, giảm độ trễ tối đa khi hiển thị từ phần cứng nhúng, giao diện được xây dựng hoàn toàn bằng **Công nghệ thuần (Vanilla Tech)**:

- **HTML5 Semantic:** Định trúc bố cục Dashboard khoa học, rõ ràng.
- **CSS3 Flexbox & Grid:** Giao diện tối màu (Dark Mode) hiện đại, hỗ trợ Responsive tương thích tốt trên cả Laptop lẫn Máy tính bảng.
- **JavaScript ES6+:** Xử lý kết nối, bóc tách chuỗi JSON tổng hợp và cập nhật dữ liệu lên màn hình tức thời (Real-time DOM Manipulation).

---
