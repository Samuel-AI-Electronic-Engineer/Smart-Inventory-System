# ESP32-CAM Firmware Setup Guide

## Overview

The enhanced `Initial_Version.ino` firmware transforms your ESP32-CAM into a smart IoT device that:

- ✅ Captures warehouse shelf images (JPEG compressed, 320×240)
- ✅ Publishes images to MQTT broker (Raspberry Pi)
- ✅ Controls 3 servo motors via MQTT commands
- ✅ Reports system telemetry (WiFi signal, heap memory, uptime)
- ✅ Responds to remote commands (capture, servo movement, status)

---

## Hardware Requirements

### ESP32-CAM Module

- **Sensor:** OV2640 (2MP, 1600×1200)
- **Connectivity:** WiFi 802.11 b/g/n, BLE
- **Pins Used:**
  - GPIO 15: Servo A1
  - GPIO 18: Servo A2
  - GPIO 19: Servo A3
  - GPIO 33: Status LED (built-in)

### Required Libraries

- **ESP32Servo** — Servo control
- **WiFi** — Built-in
- **PubSubClient** — MQTT client
- **esp_camera** — Built-in
- **SPIFFS** — Built-in
- **ArduinoJson** — JSON parsing (install via Arduino IDE)

### Optional

- 3× Servo motors (or equivalent actuators)
- USB 5V power supply (minimum 2A recommended)
- Micro-USB cable for programming

---

## Installation Instructions

### Step 1: Install Arduino IDE Board Support

1. Open **Arduino IDE**
2. Go to **File → Preferences**
3. Add to "Additional Board Manager URLs":

   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```

4. Go to **Tools → Board → Boards Manager**
5. Search for "esp32" and install **esp32 by Espressif Systems**
6. Select **Tools → Board → ESP32 → AI Thinker ESP32-CAM**

### Step 2: Install Required Libraries

1. Go to **Sketch → Include Library → Manage Libraries**
2. Install each library:
   - **ESP32Servo** (by Kevin Harrington)
   - **PubSubClient** (by Nick O'Leary)
   - **ArduinoJson** (by Benoit Blanchon)

### Step 3: Configure Firmware

Edit these values in `Initial_Version.ino` (lines 26-42):

```cpp
// WiFi Configuration
const char* WIFI_SSID = "your-wifi-ssid";           // Your WiFi network
const char* WIFI_PASSWORD = "your-wifi-password";   // Your WiFi password

// MQTT Configuration (Raspberry Pi broker)
const char* MQTT_BROKER = "192.168.1.100";          // Raspberry Pi IP address
const int MQTT_PORT = 1883;
const char* MQTT_USERNAME = "mqtt_user";            // From your MQTT setup
const char* MQTT_PASSWORD = "mqtt_pass";            // From your MQTT setup
const char* DEVICE_ID = "esp32-cam-shelf-001";      // Unique ID (change for multiple cameras)
```

### Step 4: Upload to ESP32-CAM

1. **Connect** ESP32-CAM to your computer via micro-USB
2. **Select** Tools → Port → (your COM port)
3. **Hold** the **BOOT button** on the camera
4. Click **Upload** (release BOOT after upload starts)
5. **Monitor** serial output: Tools → Serial Monitor (115200 baud)

Expected startup output:

```
===========================================
SmartInventoryAI - ESP32-CAM Startup
===========================================
Connecting to WiFi: your-wifi-ssid
...WiFi connected. IP: 192.168.1.50
Signal strength: -45 dBm
Connecting to MQTT broker: 192.168.1.100:1883
MQTT connected
Setup complete. Running main loop...
```

---

## MQTT Topics & Commands

### Status Messages

**Topic:** `inventory/device/status`  
**Message (JSON):**

```json
{
  "device_id": "esp32-cam-shelf-001",
  "status": "online",
  "timestamp": 12345678,
  "uptime_ms": 3600000
}
```

### Image Publishing

**Topic:** `inventory/camera/image/{device_id}/{timestamp}`  
**Payload:** Binary JPEG image data (no JSON)

### Telemetry Reports

**Topic:** `inventory/device/telemetry`  
**Interval:** Every 30 seconds  
**Message (JSON):**

```json
{
  "device_id": "esp32-cam-shelf-001",
  "wifi_signal_dbm": -45,
  "free_heap": 125420,
  "total_heap": 262144,
  "uptime_sec": 3600,
  "capture_count": 1200,
  "mqtt_published": 1200,
  "timestamp": 12345678
}
```

### Remote Commands

**Topic:** `inventory/device/command`  
**Publish JSON to control:**

#### Move Servo

```json
{
  "servo": 1,
  "angle": 45
}
```

- `servo`: 1, 2, or 3
- `angle`: 0-180 degrees

#### Capture Image

```json
{
  "command": "capture"
}
```

#### Run Servo Test

```json
{
  "command": "test_servo"
}
```

#### Get Status

```json
{
  "command": "status"
}
```

---

## Testing with MQTT

### Option 1: Using Mosquitto CLI

```bash
# Subscribe to all topics
mosquitto_sub -h 192.168.1.100 -p 1883 -u mqtt_user -P mqtt_pass -t "inventory/#" -v

# Publish move command
mosquitto_pub -h 192.168.1.100 -p 1883 -u mqtt_user -P mqtt_pass \
  -t "inventory/device/command" \
  -m '{"servo": 1, "angle": 90}'

# Publish capture command
mosquitto_pub -h 192.168.1.100 -p 1883 -u mqtt_user -P mqtt_pass \
  -t "inventory/device/command" \
  -m '{"command": "capture"}'
```

### Option 2: Using Python Script

```python
import paho.mqtt.client as mqtt
import json

client = mqtt.Client()
client.username_pw_set("mqtt_user", "mqtt_pass")
client.connect("192.168.1.100", 1883)

# Move servo
payload = json.dumps({"servo": 1, "angle": 45})
client.publish("inventory/device/command", payload)

# Capture image
payload = json.dumps({"command": "capture"})
client.publish("inventory/device/command", payload)
```

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| **Won't upload** | Hold BOOT button longer; try different USB cable; check COM port |
| **Can't connect to WiFi** | Verify SSID/password in code; check 2.4GHz band (not 5GHz) |
| **Can't connect to MQTT** | Verify Raspberry Pi is running Mosquitto broker; check firewall rules |
| **Images not publishing** | Check free heap memory; reduce JPEG quality or frame size |
| **Servos don't move** | Verify GPIO pins (15, 18, 19) not used by camera; check servo power |
| **High memory usage** | Reduce frame buffer count or JPEG quality in camera_config |

---

## Performance Tuning

### Reduce Image Size (Lower Bandwidth)

```cpp
.frame_size = FRAMESIZE_CIF,  // 352x288 instead of QVGA
.jpeg_quality = 20,            // Lower quality = smaller file
```

### Improve Image Quality

```cpp
.frame_size = FRAMESIZE_VGA,  // 640x480
.jpeg_quality = 8,             // Higher quality (more detail)
```

### Adjust Capture Frequency

```cpp
const unsigned long CAPTURE_INTERVAL = 5000;  // 5 seconds between captures
```

---

## Security Notes

⚠️ **Important:** This is development firmware. For production:

1. **Change default credentials:** Update MQTT username/password
2. **Use TLS/SSL:** Enable MQTT over TLS (port 8883)
3. **Change Device ID:** Use unique IDs for multiple cameras
4. **Disable Serial Debug:** Remove or guard Serial.println() calls
5. **Secure WiFi:** Use WPA2+ encryption (not open networks)
6. **Firewall Rules:** Restrict MQTT broker access by IP

---

## Next Steps

1. ✅ Upload and test firmware
2. ✅ Verify images publish to MQTT
3. ✅ Test servo control commands
4. ✅ Monitor telemetry in Python script
5. ➡️ Connect to YOLOv8 detector (Python backend)
6. ➡️ Deploy second/third cameras for multi-angle coverage

---

## Reference Documentation

- [ESP32-CAM Pinout](https://github.com/espressif/arduino-esp32/tree/master/variants/esp32)
- [PubSubClient Documentation](https://github.com/knolleary/pubsubclient)
- [ArduinoJson Guide](https://arduinojson.org/)
- [MQTT Protocol Spec](https://mqtt.org/mqtt-specification)

---

**Made with ❤️ by Samuel Alarcón Hernández**
