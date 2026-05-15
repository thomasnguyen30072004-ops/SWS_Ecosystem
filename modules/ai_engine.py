import os
import cv2 
from ultralytics import YOLO
import streamlit as st

@st.cache_resource
def load_yolo_model():
    # Trỏ thẳng vào thư mục chứa NCNN
    model_path = os.path.join('models', 'waste_ncnn') 
    if os.path.exists(model_path):
        # Ultralytics sẽ tự động tìm .param và .bin trong folder này
        return YOLO(model_path, task='classify') 
    return None

def predict_waste(model, image):
    # NCNN hoạt động tốt nhất khi cố định định dạng đầu vào
    results = model.predict(source=image, conf=0.4, verbose=False, imgsz=224)
    
    if results and results[0].probs is not None:
        top1_idx = int(results[0].probs.top1)
        label = results[0].names[top1_idx].upper()
        prob = float(results[0].probs.top1conf) * 100
        
        # Vẽ khung hình để debug nếu cần
        annotated_img = results[0].plot()
        annotated_img_rgb = cv2.cvtColor(annotated_img, cv2.COLOR_BGR2RGB)
        
        return label, prob, annotated_img_rgb
    
    return None, None, None