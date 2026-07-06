# 🎵 ESP32-S3 Melody Player
https://www.youtube.com/watch?v=z0VO1zhtYoA

An ESP32-S3 multimedia project that combines a TFT display, I2S audio, Wi-Fi, MQTT communication, and a web interface for creating and playing custom melodies.

---

## Features

* Play built-in melodies
* Create and save custom melodies
* Display images on a ST7789 TFT screen
* Play audio through an I2S speaker
* Control the device from a web browser
* Control playback remotely using MQTT
* Store custom melodies in SPIFFS

---

## Hardware

* ESP32-S3 DevKitM-1
* ST7789 TFT Display (240 × 320)
* I2S Audio Amplifier
* Speaker

---

## Software

* PlatformIO
* Arduino Framework
* Adafruit GFX Library
* Adafruit ST7735/ST7789 Library
* PubSubClient
* SPIFFS

---

## Setup

### 1. Configure Wi-Fi

```cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_PASSWORD";
```

### 2. Configure MQTT (Optional)

```cpp
const char* mqtt_server = "YOUR_MQTT_SERVER_IP";
```

### 3. Upload the Firmware

Build and upload the project using PlatformIO.

---

## Web Interface

After the ESP32 connects to Wi-Fi, open its IP address in your browser.

From the web page you can:

* Play built-in melodies
* Edit custom melodies
* Save melodies to SPIFFS
* Play your custom melody

---

## MQTT

**Topic**

```text
music/control
```

**Commands**

```text
melody1
melody2
twinkle
furelise
harry
avengers
```

---

## Custom Melody Format

Example:

```text
C4,300
D4,300
E4,300
G4,500
```

Each line uses the format:

```text
Note,Duration(ms)
```

Supported notes include:

* C4–B4
* C5–B5
* Sharp notes (C#4, D#4, F#4, G#4, A#4)

---

## Serial Commands

```text
tone
beep
melody1
melody2
clear
test
```

---

## Author

**Cecelia Zhen**

This project is part of my Marvel-inspired robotics platform, combining embedded systems, electronics, software, and mechanical design into an integrated robotics ecosystem.
