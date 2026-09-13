import cv2
import numpy as np
import sys

def analyze_image(path):
    img = cv2.imread(path)
    if img is None:
        print(f"Could not load {path}")
        return
    
    print(f"--- Analysis of {path} ---")
    print(f"Shape: {img.shape}")
    
    # Just print unique colors
    pixels = img.reshape(-1, 3)
    unique_colors = np.unique(pixels, axis=0)
    print(f"Unique colors (BGR):")
    for c in unique_colors:
        print(c)

analyze_image(r"C:\Users\tymon\.gemini\antigravity\brain\978f81bb-3c64-4a05-8490-47876b90ea88\.user_uploaded\media_1789314892900.png")
analyze_image(r"C:\Users\tymon\.gemini\antigravity\brain\978f81bb-3c64-4a05-8490-47876b90ea88\.user_uploaded\media_1789315013100.png")
