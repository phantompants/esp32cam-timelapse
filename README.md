# 📷 ESP32-CAM Timelapse Studio

A feature-rich, open-source timelapse photography suite designed specifically for the **AI-Thinker ESP32-CAM** (OV2640 camera module).

It captures high-resolution JPEG photos at customizable intervals, saves them to an onboard MicroSD card, provides a modern responsive web dashboard for remote control, and includes a Python utility to stitch images into ultra-smooth MP4 videos.

---

## ✨ Features

- **📷 High-Resolution Capture**: Supports resolutions up to UXGA (1600x1200) with OV2640 sensor.
- **💾 MicroSD Card Storage**: Automatically organizes photos into `/timelapse/img_00001.jpg`.
- **🌐 Responsive Web Dashboard**:
  - Live preview & snapshot capture.
  - Start / Pause / Stop timelapse sessions remotely.
  - Customizable capture intervals (2s, 5s, 10s, 30s, 1m, 5m, 30m, 1h).
  - MicroSD File Manager (browse, view, download, delete images).
- **📶 Dual Wi-Fi Modes**: Connects to your home Wi-Fi network, or falls back to creating its own Access Point (`ESP32CAM-Timelapse`).
- **🎬 Python Video Stitcher**: One-line command to auto-download photos from ESP32-CAM and render high-quality MP4 videos.

---

## 🛠️ Hardware Requirements & Wiring

### Components Needed
1. **ESP32-CAM Board** (AI-Thinker OV2640 module)
2. **MicroSD Card** (FAT32 formatted, up to 32GB recommended)
3. **FTDI USB-to-TTL Serial Adapter** (3.3V / 5V)
4. **Jumper Wires** & 5V Power Supply (min 1A)

### FTDI Flashing Wiring Diagram

| FTDI Adapter Pin | ESP32-CAM Pin | Notes |
| :--- | :--- | :--- |
| **VCC (5V)** | **5V** | Provide stable 5V power supply |
| **GND** | **GND** | Common ground |
| **TX** | **U0R (GPIO 3)** | Serial Receive |
| **RX** | **U0T (GPIO 1)** | Serial Transmit |
| **GND** | **GPIO 0** | **Connect ONLY when flashing code**, remove after upload! |

> [!IMPORTANT]
> Connect **GPIO 0 to GND** before powering on the board to enter **flashing mode**. Disconnect GPIO 0 from GND and press the RESET button to run the program normally.

---

## 🚀 Software Setup & Installation

### 1. Arduino IDE Setup
1. Open **Arduino IDE**.
2. Go to **File -> Preferences**.
3. Add the following URL to **Additional Boards Manager URLs**:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools -> Board -> Boards Manager**, search for `esp32` by Espressif, and click **Install**.

### 2. Configure Board Options
- **Board**: `AI Thinker ESP32-CAM`
- **CPU Frequency**: `240MHz (WiFi/BT)`
- **Flash Frequency**: `80MHz`
- **Flash Mode**: `QIO`
- **Partition Scheme**: `Huge APP (3MB No OTA/1MB SPIFFS)`
- **PSRAM**: `Enabled`

### 3. Flash Code
1. Open `esp32cam_timelapse.ino`.
2. Update `WIFI_SSID` and `WIFI_PASS` with your network credentials.
3. Put ESP32-CAM into bootloader mode (GPIO 0 connected to GND).
4. Click **Upload** in Arduino IDE.
5. Once complete, disconnect GPIO 0 from GND and press the **RESET** button.
6. Open **Serial Monitor** at **115200 baud** to see the IP address.

---

## 🎬 Generating MP4 Timelapse Videos

Use the included `timelapse_stitcher.py` script:

### Option A: Auto-Download from ESP32-CAM over Wi-Fi & Render
```bash
python timelapse_stitcher.py --url http://192.168.1.100 --output timelapse.mp4 --fps 30
```

### Option B: Local MicroSD Images Folder
```bash
python timelapse_stitcher.py --input ./images --output timelapse.mp4 --fps 30
```

---

## 📄 License
Distributed under the MIT License. See `LICENSE` for details.
