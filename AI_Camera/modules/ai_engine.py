import os
import cv2 
from ultralytics import YOLO # type: ignore

def load_yolo_model():
    # TRỎ CHÍNH XÁC: Vào tên thư mục mang đuôi _ncnn_model tuyệt đối
    folder_path = os.path.abspath(os.path.join('models', 'waste_ncnn_model')) 
    
    if os.path.exists(folder_path):
        print(f"[AI ENGINE] Đang kích hoạt bộ não NCNN từ: {folder_path}")
        return YOLO(folder_path, task='classify')
    else:
        print(f"[LỖI AI] Không tìm thấy thư mục model tại: {folder_path}")
        return None

def predict_waste(model, image):
    # Định dạng đầu vào cố định 224x224 giúp tăng tốc độ xử lý trên Pi 4
    results = model.predict(source=image, conf=0.4, verbose=False, imgsz=224)
    
    if results and results[0].probs is not None:
        top1_idx = int(results[0].probs.top1)
        label = results[0].names[top1_idx].upper()
        prob = float(results[0].probs.top1conf) * 100
        
        annotated_img = results[0].plot()
        annotated_img_rgb = cv2.cvtColor(annotated_img, cv2.COLOR_BGR2RGB)
        
        return label, prob, annotated_img_rgb
    
    return None, None, None