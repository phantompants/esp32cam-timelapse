/*
 * ESP32-CAM Timelapse Studio Sketch
 * ---------------------------------
 * Description: Captures periodic JPEG photos from an OV2640 camera on ESP32-CAM 
 *              and saves them directly to an onboard MicroSD card.
 * Features:
 *  - Interactive Web Dashboard for live previews, file browsing, & capture control.
 *  - Configurable capture intervals (2s to 1 hour).
 *  - Wi-Fi Access Point mode + Station mode auto-failover.
 *  - MicroSD file browser with inline view & delete functionality.
 *  - Compatible with Python stitcher script for generating high-definition MP4 videos.
 *
 * Board Settings in Arduino IDE:
 *  - Board: "AI Thinker ESP32-CAM"
 *  - Flash Mode: QIO
 *  - Flash Frequency: 80MHz
 *  - Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)"
 *  - PSRAM: Enabled
 */

#define CAMERA_MODEL_AI_THINKER

#include "esp_camera.h"
#include "esp_system.h"
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SD_MMC.h>
#include <Preferences.h>

#include "camera_pins.h"
#include "web_interface.h"

// ================= USER CONFIGURATION =================
const char* WIFI_SSID     = "YOUR_WIFI_SSID";     // Change to your Wi-Fi SSID
const char* WIFI_PASS     = "YOUR_WIFI_PASSWORD"; // Change to your Wi-Fi Password

const char* AP_SSID       = "ESP32CAM-Timelapse"; // Fallback Access Point SSID
const char* AP_PASS       = "12345678";           // Fallback Access Point Password
// ======================================================

WebServer server(80);
Preferences preferences;

// Global State
bool isLapseRunning = false;
unsigned long captureIntervalMs = 5000; // Default: 5 seconds
unsigned long lastCaptureTime = 0;
uint32_t photoCounter = 0;

// Function Prototypes
void initCamera();
void initSDCard();
void initWiFi();
void setupWebRoutes();
void handleCapture();
void takeTimelapsePhoto();
String getSDFreeSpace();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("\n--- ESP32-CAM Timelapse Studio Starting ---");

  pinMode(LED_BUILTIN_GPIO, OUTPUT);
  pinMode(LED_FLASH_GPIO, OUTPUT);
  digitalWrite(LED_BUILTIN_GPIO, HIGH); // Turn off (active low)
  digitalWrite(LED_FLASH_GPIO, LOW);    // Turn off flash

  // Load counter from EEPROM/Preferences
  preferences.begin("timelapse", false);
  photoCounter = preferences.getUInt("counter", 0);

  // Initialize Hardware
  initCamera();
  initSDCard();
  initWiFi();
  setupWebRoutes();

  server.begin();
  Serial.println("Web server started successfully!");
  Serial.print("Access URL: http://");
  Serial.println(WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : WiFi.softAPIP().toString());
}

void loop() {
  server.handleClient();

  // Automatic Timelapse Capture Logic
  if (isLapseRunning) {
    unsigned long currentMs = millis();
    if (currentMs - lastCaptureTime >= captureIntervalMs) {
      lastCaptureTime = currentMs;
      takeTimelapsePhoto();
    }
  }
}

void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }
  Serial.println("Camera initialized successfully.");
}

void initSDCard() {
  Serial.println("Initializing MicroSD Card...");
  if (!SD_MMC.begin("/sdcard", true)) { // 1-bit mode for higher stability on ESP32-CAM
    Serial.println("SD Card Mount Failed! Check card insertion.");
    return;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD Card attached.");
    return;
  }

  if (!SD_MMC.exists("/timelapse")) {
    SD_MMC.mkdir("/timelapse");
    Serial.println("Created directory /timelapse");
  }

  Serial.printf("SD Card Size: %llu MB\n", SD_MMC.cardSize() / (1024 * 1024));
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to Wi-Fi");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 15) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nConnected! IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWi-Fi connection failed. Starting Access Point mode...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
  }
}

void takeTimelapsePhoto() {
  digitalWrite(LED_BUILTIN_GPIO, LOW); // Flash blue indicator LED during capture

  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed!");
    digitalWrite(LED_BUILTIN_GPIO, HIGH);
    return;
  }

  photoCounter++;
  preferences.putUInt("counter", photoCounter);

  char filename[64];
  snprintf(filename, sizeof(filename), "/timelapse/img_%05d.jpg", photoCounter);

  File file = SD_MMC.open(filename, FILE_WRITE);
  if (!file) {
    Serial.printf("Failed to open file %s for writing\n", filename);
  } else {
    file.write(fb->buf, fb->len);
    file.close();
    Serial.printf("Saved photo: %s (%u bytes)\n", filename, fb->len);
  }

  esp_camera_fb_return(fb);
  digitalWrite(LED_BUILTIN_GPIO, HIGH); // Turn off indicator LED
}

void setupWebRoutes() {
  // Serve Web Interface
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", INDEX_HTML);
  });

  // Serve Live Snapshot / Stream Frame
  server.on("/capture", HTTP_GET, []() {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      server.send(500, "text/plain", "Camera Frame Capture Failed");
      return;
    }
    server.sendHeader("Content-Type", "image/jpeg");
    server.sendHeader("Content-Length", String(fb->len));
    server.sendHeader("Cache-Control", "no-cache");
    server.sendContent((const char *)fb->buf, fb->len);
    esp_camera_fb_return(fb);
  });

  // Take single photo on demand
  server.on("/snap", HTTP_GET, []() {
    takeTimelapsePhoto();
    server.send(200, "text/plain", "Snapshot captured to SD card!");
  });

  // Control Routes
  server.on("/start", HTTP_GET, []() {
    isLapseRunning = true;
    lastCaptureTime = millis();
    server.send(200, "application/json", "{\"running\":true}");
  });

  server.on("/stop", HTTP_GET, []() {
    isLapseRunning = false;
    server.send(200, "application/json", "{\"running\":false}");
  });

  server.on("/set_interval", HTTP_GET, []() {
    if (server.hasArg("val")) {
      int sec = server.arg("val").toInt();
      if (sec >= 1) {
        captureIntervalMs = sec * 1000;
      }
    }
    server.send(200, "text/plain", "OK");
  });

  server.on("/set_framesize", HTTP_GET, []() {
    if (server.hasArg("val")) {
      int val = server.arg("val").toInt();
      sensor_t * s = esp_camera_sensor_get();
      if (s) {
        s->set_framesize(s, (framesize_t)val);
      }
    }
    server.send(200, "text/plain", "OK");
  });

  server.on("/status", HTTP_GET, []() {
    String json = "{";
    json += "\"running\":" + String(isLapseRunning ? "true" : "false") + ",";
    json += "\"interval\":" + String(captureIntervalMs / 1000) + ",";
    json += "\"count\":" + String(photoCounter) + ",";
    json += "\"sdFree\":\"" + getSDFreeSpace() + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI());
    json += "}";
    server.send(200, "application/json", json);
  });

  // File Manager Routes
  server.on("/list", HTTP_GET, []() {
    File root = SD_MMC.open("/timelapse");
    if (!root || !root.isDirectory()) {
      server.send(200, "application/json", "[]");
      return;
    }

    String json = "[";
    File file = root.openNextFile();
    bool first = true;
    while (file) {
      if (!file.isDirectory()) {
        if (!first) json += ",";
        first = false;
        json += "{\"name\":\"" + String(file.name()) + "\",";
        json += "\"path\":\"/timelapse/" + String(file.name()) + "\",";
        json += "\"size\":\"" + String(file.size() / 1024) + " KB\"}";
      }
      file = root.openNextFile();
    }
    json += "]";
    server.send(200, "application/json", json);
  });

  // Serve image file from SD card
  server.onNotFound([]() {
    String path = server.uri();
    if (path.startsWith("/timelapse/") && SD_MMC.exists(path)) {
      File file = SD_MMC.open(path, FILE_READ);
      server.streamFile(file, "image/jpeg");
      file.close();
    } else {
      server.send(404, "text/plain", "File Not Found");
    }
  });

  server.on("/delete", HTTP_GET, []() {
    if (server.hasArg("path")) {
      String path = server.arg("path");
      if (SD_MMC.exists(path)) {
        SD_MMC.remove(path);
        server.send(200, "text/plain", "File deleted successfully");
        return;
      }
    }
    server.send(400, "text/plain", "Invalid path");
  });
}

String getSDFreeSpace() {
  uint64_t totalBytes = SD_MMC.totalBytes();
  uint64_t usedBytes = SD_MMC.usedBytes();
  if (totalBytes == 0) return "No SD Card";
  uint64_t freeBytes = totalBytes - usedBytes;
  return String(freeBytes / (1024 * 1024)) + " MB";
}
