#!/usr/bin/env python3
"""
ESP32-CAM Timelapse Video Stitcher
----------------------------------
Stitches captured JPEG images into a smooth MP4 timelapse video.

Usage:
  1. Local folder mode (if images are on your computer/SD card):
     python timelapse_stitcher.py --input ./images --output timelapse.mp4 --fps 30

  2. Direct Download mode (downloads directly from ESP32-CAM via Wi-Fi):
     python timelapse_stitcher.py --url http://192.168.1.100 --output timelapse.mp4 --fps 30
"""

import os
import sys
import glob
import argparse
import urllib.request
import json

def download_images_from_esp32(esp_url, download_dir):
    """Downloads all saved photos from ESP32-CAM Web API."""
    os.makedirs(download_dir, exist_ok=True)
    list_url = f"{esp_url.rstrip('/')}/list"
    print(f"[+] Connecting to ESP32-CAM at {list_url}...")
    
    try:
        req = urllib.request.urlopen(list_url)
        files = json.loads(req.read().decode('utf-8'))
        print(f"[+] Found {len(files)} photos on ESP32-CAM SD card.")
        
        for idx, file_info in enumerate(files, start=1):
            img_path = file_info['path']
            img_url = f"{esp_url.rstrip('/')}{img_path}"
            local_filename = os.path.join(download_dir, os.path.basename(img_path))
            
            print(f"[{idx}/{len(files)}] Downloading {local_filename}...")
            urllib.request.urlretrieve(img_url, local_filename)
            
        print("[+] All photos downloaded successfully!")
    except Exception as e:
        print(f"[-] Error downloading photos from ESP32-CAM: {e}")
        sys.exit(1)

def build_timelapse_cv2(image_files, output_path, fps):
    """Builds timelapse video using OpenCV."""
    import cv2
    
    first_frame = cv2.imread(image_files[0])
    if first_frame is None:
        print(f"[-] Could not read image: {image_files[0]}")
        sys.exit(1)
        
    height, width, _ = first_frame.shape
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter(output_path, fourcc, fps, (width, height))
    
    print(f"[+] Generating video '{output_path}' ({width}x{height} @ {fps} FPS)...")
    for idx, img_path in enumerate(image_files, start=1):
        frame = cv2.imread(img_path)
        if frame is not None:
            # Resize if dimensions differ from first frame
            if (frame.shape[1], frame.shape[0]) != (width, height):
                frame = cv2.resize(frame, (width, height))
            out.write(frame)
        if idx % 50 == 0 or idx == len(image_files):
            print(f"    Processed {idx}/{len(image_files)} frames...")
            
    out.release()
    print(f"[✓] Timelapse video created successfully: {output_path}")

def main():
    parser = argparse.ArgumentParser(description="Stitch ESP32-CAM timelapse photos into an MP4 video.")
    parser.add_argument("--input", "-i", default="./images", help="Path to local folder containing JPEG photos")
    parser.add_argument("--output", "-o", default="timelapse.mp4", help="Output MP4 filename (default: timelapse.mp4)")
    parser.add_argument("--fps", "-f", type=int, default=30, help="Frames per second for output video (default: 30)")
    parser.add_argument("--url", "-u", default=None, help="ESP32-CAM IP URL (e.g. http://192.168.1.100) to auto-download photos")

    args = parser.parse_args()

    if args.url:
        download_images_from_esp32(args.url, args.input)

    search_path = os.path.join(args.input, "*.jpg")
    image_files = sorted(glob.glob(search_path))

    if not image_files:
        search_path_upper = os.path.join(args.input, "*.JPG")
        image_files = sorted(glob.glob(search_path_upper))

    if not image_files:
        print(f"[-] No JPG images found in directory: {args.input}")
        sys.exit(1)

    print(f"[+] Found {len(image_files)} JPEG images in '{args.input}'.")

    try:
        build_timelapse_cv2(image_files, args.output, args.fps)
    except ImportError:
        print("[-] OpenCV (cv2) is not installed.")
        print("[!] You can install OpenCV using: pip install opencv-python")
        print("[!] Alternatively, run FFmpeg command:")
        print(f"    ffmpeg -framerate {args.fps} -pattern_type glob -i '{args.input}/*.jpg' -c:v libx264 -pix_fmt yuv420p {args.output}")

if __name__ == "__main__":
    main()
