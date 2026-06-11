/*
 * SmartInventoryAI - ESP32-CAM IoT Gateway Firmware
 * Purpose: Capture warehouse shelf images, control servo actuators, publish via MQTT
 *
 * Hardware:
 * - ESP32-CAM (OV2640 sensor, WiFi, BLE)
 * - 3x Servo Motors (pins 15, 18, 19)
 * - Status LED (built-in GPIO 33)
 *
 * Author: Samuel Alarcón Hernández
 * Date: 2026-06-11
 */

// ============================================================================
// LIBRARIES & DEPENDENCIES
// ============================================================================

#include <ESP32Servo.h>   // Servo control
#include <WiFi.h>         // WiFi connectivity
#include <PubSubClient.h> // MQTT client
#include <esp_camera.h>   // ESP32-CAM library
#include <SPIFFS.h>       // File system
#include <time.h>         // Timekeeping

// ============================================================================
// CONFIGURATION - UPDATE THESE VALUES
// ============================================================================

// WiFi Configuration
const char *WIFI_SSID = "your-wifi-ssid";
const char *WIFI_PASSWORD = "your-wifi-password";

// MQTT Configuration (Raspberry Pi broker)
const char *MQTT_BROKER = "192.168.1.100"; // Raspberry Pi IP
const int MQTT_PORT = 1883;
const char *MQTT_USERNAME = "mqtt_user";
const char *MQTT_PASSWORD = "mqtt_pass";
const char *DEVICE_ID = "esp32-cam-shelf-001"; // Unique device identifier

// MQTT Topics
const char *TOPIC_STATUS = "inventory/device/status";
const char *TOPIC_IMAGE = "inventory/camera/image";
const char *TOPIC_TELEMETRY = "inventory/device/telemetry";
const char *TOPIC_COMMAND = "inventory/device/command";

// Timings (milliseconds)
const unsigned long CAPTURE_INTERVAL = 3000; // Capture every 3 seconds
const unsigned long MQTT_HEARTBEAT = 30000;  // Heartbeat every 30 seconds
const unsigned long WIFI_RETRY_DELAY = 5000; // Retry WiFi every 5 seconds

// ============================================================================
// GLOBAL OBJECTS & VARIABLES
// ============================================================================

Servo servo_A1, servo_A2, servo_A3;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

unsigned long lastCaptureTime = 0;
unsigned long lastHeartbeatTime = 0;
bool ledState = false;

// Device telemetry
struct
{
  int wifiSignal = 0;
  float cpuTemp = 0;
  uint32_t freeHeap = 0;
  uint32_t totalHeap = 0;
  unsigned long uptime = 0;
  int captureCount = 0;
  int mqttPublished = 0;
} telemetry;

// ============================================================================
// CAMERA CONFIGURATION (OV2640)
// ============================================================================

camera_config_t camera_config = {
    .pin_d0 = Y2_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .xclk_freq_hz = 20000000,
    .pixel_format = PIXFORMAT_JPEG, // JPEG compression
    .frame_size = FRAMESIZE_QVGA,   // 320x240 (good balance)
    .jpeg_quality = 10,             // 0-63 (lower = higher quality)
    .fb_count = 1,                  // Frame buffer count
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

// ============================================================================
// SETUP FUNCTION
// ============================================================================

void setup()
{
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n===========================================");
  Serial.println("SmartInventoryAI - ESP32-CAM Startup");
  Serial.println("===========================================");

  // Initialize LED pin
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);

  // Initialize SPIFFS (file system)
  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS mount failed");
  }

  // Initialize camera
  initializeCamera();


  // Connect to WiFi
  connectToWiFi();

  // Configure MQTT
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttMessageCallback);

  // Connect to MQTT broker
  connectToMQTT();

  // Publish startup message
  publishStatus("online");

  Serial.println("Setup complete. Running main loop...");
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop()
{
  // Update telemetry
  updateTelemetry();

  // Maintain WiFi connection
  if (!WiFi.isConnected())
  {
    Serial.println("WiFi disconnected. Attempting reconnection...");
    connectToWiFi();
  }

  // Maintain MQTT connection
  if (!mqttClient.connected())
  {
    if (!connectToMQTT())
    {
      delay(WIFI_RETRY_DELAY);
      return;
    }
  }
  mqttClient.loop(); // Process MQTT messages

  // Capture and publish image
  if (millis() - lastCaptureTime >= CAPTURE_INTERVAL)
  {
    captureAndPublishImage();
    lastCaptureTime = millis();
  }

  // Send heartbeat telemetry
  if (millis() - lastHeartbeatTime >= MQTT_HEARTBEAT)
  {
    publishTelemetry();
    lastHeartbeatTime = millis();
  }

  // Blink status LED
  blinkStatusLED();
}

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

void initializeCamera()
{
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    publishStatus("camera_error");
    return;
  }

  // Adjust camera settings
  sensor_t *s = esp_camera_sensor_get();
  s->set_brightness(s, 0); // Brightness: -2 to 2
  s->set_contrast(s, 0);   // Contrast: -2 to 2
  s->set_saturation(s, 0); // Saturation: -2 to 2
  s->set_gainceiling(s, GAINCEILING_2x);
  s->set_gain(s, 0);
  s->set_exposure(s, -5); // Exposure: -20 to 20
  s->set_gp_gain(s, 0);
  s->set_bpc(s, 1);      // BPC enabled
  s->set_wpc(s, 1);      // WPC enabled
  s->set_awb_gain(s, 1); // AWB gain enabled
  s->set_agc(s, 25);
  s->set_aec_value(s, 300);
  s->set_whitebal(s, 1);

  Serial.println("Camera initialized successfully");
  publishStatus("camera_ready");
}

void connectToWiFi()
{
  Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.isConnected())
  {
    Serial.printf("\nWiFi connected. IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Signal strength: %d dBm\n", WiFi.RSSI());
  }
  else
  {
    Serial.println("\nFailed to connect to WiFi");
  }
}

bool connectToMQTT()
{
  Serial.printf("Connecting to MQTT broker: %s:%d\n", MQTT_BROKER, MQTT_PORT);

  if (mqttClient.connect(DEVICE_ID, MQTT_USERNAME, MQTT_PASSWORD))
  {
    Serial.println("MQTT connected");

    // Subscribe to command topic
    mqttClient.subscribe(TOPIC_COMMAND);
    publishStatus("mqtt_connected");
    return true;
  }
  else
  {
    Serial.printf("MQTT connection failed, state: %d\n", mqttClient.state());
    return false;
  }
}

// ============================================================================
// CAMERA & IMAGE FUNCTIONS
// ============================================================================

void captureAndPublishImage()
{
  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb)
  {
    Serial.println("Camera capture failed");
    return;
  }

  // Publish image to MQTT (base64 encoded or binary)
  // For bandwidth efficiency, consider downsampling or compression

  char topic_with_timestamp[100];
  snprintf(topic_with_timestamp, sizeof(topic_with_timestamp),
           "%s/%s/%ld", TOPIC_IMAGE, DEVICE_ID, millis());

  boolean result = mqttClient.publish_P(
      (char *)topic_with_timestamp,
      fb->buf,
      fb->len,
      false); // QoS 0 for fast throughput

  if (result)
  {
    telemetry.captureCount++;
    telemetry.mqttPublished++;
    Serial.printf("Image published: %d bytes\n", fb->len);
  }
  else
  {
    Serial.println("Image publish failed");
  }

  esp_camera_fb_return(fb);
}


// ============================================================================
// MQTT FUNCTIONS
// ============================================================================

void publishStatus(const char *status)
{
  StaticJsonDocument<200> doc;
  doc["device_id"] = DEVICE_ID;
  doc["status"] = status;
  doc["timestamp"] = millis();
  doc["uptime_ms"] = telemetry.uptime;

  char buffer[256];
  size_t n = serializeJson(doc, buffer);
  mqttClient.publish(TOPIC_STATUS, buffer, n);
}

void publishTelemetry()
{
  StaticJsonDocument<400> doc;
  doc["device_id"] = DEVICE_ID;
  doc["wifi_signal_dbm"] = telemetry.wifiSignal;
  doc["free_heap"] = telemetry.freeHeap;
  doc["total_heap"] = telemetry.totalHeap;
  doc["uptime_sec"] = telemetry.uptime / 1000;
  doc["capture_count"] = telemetry.captureCount;
  doc["mqtt_published"] = telemetry.mqttPublished;
  doc["timestamp"] = millis();

  char buffer[512];
  size_t n = serializeJson(doc, buffer);
  boolean result = mqttClient.publish(TOPIC_TELEMETRY, buffer, n);

  if (result)
  {
    Serial.printf("Telemetry published: %s\n", buffer);
  }
}

void mqttMessageCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.printf("Message received on topic: %s\n", topic);

  // Parse JSON command
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload, length);

  if (error)
  {
    Serial.printf("JSON parse error: %s\n", error.c_str());
    return;
  }

  // Handle servo command: {"servo": 1, "angle": 45}
  if (doc.containsKey("servo") && doc.containsKey("angle"))
  {
    int servo = doc["servo"];
    int angle = doc["angle"];
    moveServo(servo, angle);
  }

  // Handle capture command: {"command": "capture"}
  if (doc.containsKey("command"))
  {
    String cmd = doc["command"].as<String>();
    if (cmd == "capture")
    {
      captureAndPublishImage();
    }
    else if (cmd == "test_servo")
    {
      testServoSequence();
    }
    else if (cmd == "status")
    {
      publishStatus("healthy");
    }
  }
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void updateTelemetry()
{
  telemetry.uptime = millis();
  telemetry.freeHeap = ESP.getFreeHeap();
  telemetry.totalHeap = ESP.getHeapSize();
  telemetry.wifiSignal = WiFi.RSSI();
}

void blinkStatusLED()
{
  static unsigned long lastBlink = 0;
  unsigned long blinkInterval = mqttClient.connected() ? 1000 : 200;

  if (millis() - lastBlink >= blinkInterval)
  {
    ledState = !ledState;
    digitalWrite(PIN_STATUS_LED, ledState ? HIGH : LOW);
    lastBlink = millis();
  }
}

// Note: Add #include <ArduinoJson.h> to your libraries
// Install via Arduino IDE: Sketch → Include Library → Manage Libraries → Search "ArduinoJson"
