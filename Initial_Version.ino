/*
 * SmartInventoryAI - ESP32-CAM IoT Gateway Firmware
 *
 * Descripción / Description:
 * Firmware para captura de imágenes en tiempo real usando ESP32-CAM
 * Publícalas vía MQTT hacia el broker en Raspberry Pi para procesamiento
 * con YOLOv8 y análisis multiagente con LangGraph.
 *
 * Firmware for real-time image capture using ESP32-CAM.
 * Publishes images via MQTT to Raspberry Pi broker for processing
 * with YOLOv8 and multi-agent analysis with LangGraph.
 *
 * Hardware / Dispositivos:
 * - ESP32-CAM (sensor OV2640 2MP, WiFi 802.11 b/g/n)
 * - LED de estado integrado (GPIO 33)
 *
 * Autor / Author: Samuel Alarcón Hernández
 * Fecha / Date: 2026-06-11
 */

// Librerías requeridas / Required Libraries
#include <WiFi.h>         // WiFi connectivity
#include <PubSubClient.h> // MQTT client protocol
#include <esp_camera.h>   // ESP32-CAM camera control
#include <SPIFFS.h>       // File system storage
#include <time.h>         // Time functions
#include <ArduinoJson.h>  // JSON parsing

// Configuración WiFi / WiFi Configuration
const char *WIFI_SSID = "your-wifi-ssid";
const char *WIFI_PASSWORD = "your-wifi-password";

// Configuración MQTT / MQTT Configuration (Broker Raspberry Pi)
const char *MQTT_BROKER = "192.168.1.100"; // IP del broker / Broker IP
const int MQTT_PORT = 1883;
const char *MQTT_USERNAME = "mqtt_user";
const char *MQTT_PASSWORD = "mqtt_pass";
const char *DEVICE_ID = "esp32-cam-shelf-001"; // ID único por dispositivo / Unique per device

// Tópicos MQTT / MQTT Topics
const char *TOPIC_STATUS = "inventory/device/status";       // Estado online/offline
const char *TOPIC_IMAGE = "inventory/camera/image";         // Imágenes capturadas
const char *TOPIC_TELEMETRY = "inventory/device/telemetry"; // Métricas del sistema
const char *TOPIC_COMMAND = "inventory/device/command";     // Comandos remotos

// Tiempos de ciclo / Timing Configuration (milisegundos / milliseconds)
const unsigned long CAPTURE_INTERVAL = 3000; // Capturar cada 3 seg / Capture every 3 sec
const unsigned long MQTT_HEARTBEAT = 30000;  // Telemetría cada 30 seg / Telemetry every 30 sec
const unsigned long WIFI_RETRY_DELAY = 5000; // Reintentar WiFi cada 5 seg / Retry WiFi every 5 sec

// Objetos globales / Global Objects
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Variables de control de tiempo / Timing Control Variables
unsigned long lastCaptureTime = 0;
unsigned long lastHeartbeatTime = 0;
bool ledState = false;

// Estructura de telemetría del dispositivo / Device Telemetry Structure
struct
{
  int wifiSignal = 0;       // Potencia WiFi en dBm / WiFi signal strength dBm
  uint32_t freeHeap = 0;    // Memoria libre / Free memory bytes
  uint32_t totalHeap = 0;   // Memoria total / Total memory bytes
  unsigned long uptime = 0; // Tiempo activo / Uptime milliseconds
  int captureCount = 0;     // Total de capturas / Total captures
  int mqttPublished = 0;    // Imágenes publicadas / Published images
} telemetry;

// Configuración de cámara OV2640 / OV2640 Camera Configuration

camera_config_t camera_config = {
    .pin_d0 = Y2_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    // Configuración de pines y parámetros de la cámara / Camera pins and settings
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
        .pixel_format = PIXFORMAT_JPEG, // Compresión JPEG / JPEG compression
        .frame_size = FRAMESIZE_QVGA,   // Resolución 320x240 / Resolution 320x240
        .jpeg_quality = 10,             // Calidad JPEG 0-63 / JPEG quality 0-63
        .fb_count = 1,                  // Un buffer de fotograma / One frame buffer
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    };

// Función d {
// Inicializar puerto serial para debug / Initialize serial for debugging
Serial.begin(115200);
delay(1000);

Serial.println("\n\nSmartInventoryAI - ESP32-CAM Startup");
Serial.println("Camera-Only Mode (No Servo Control)");

// Inicializar LED de estado / Initialize status LED
pinMode(PIN_STATUS_LED, OUTPUT);
digitalWrite(PIN_STATUS_LED, LOW);

// Inicializar sistema de archivos / Initialize file system
if (!SPIFFS.begin(true))
{
  Serial.println("ERROR: SPIFFS initialization failed");
}
else
{
  Serial.println("SPIFFS initialized successfully");
}

// Inicializar cámara / Initialize camera
initializeCamera();

// Conectar a WiFi / Connect to WiFi network
connectToWiFi();

// Configurar cliente MQTT / Configure MQTT client
mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
mqttClient.setCallback(mqttMessageCallback);

// Conectar al broker MQTT / Connect to MQTT broker
connectToMQTT();

// Publicar mensaje de inicio / Publish startup status
publishStatus("online");

Serial.println("Setup complete - Starting image capture loop");
}

// Función principal del programa / Main Program Loop
// Se ejecuta continuamente después del setup / Runs continuously after setup

void loop()
{
  // Update telemetry
  updateTel
  {
    // Actualizar métricas del sistema / Update system telemetry
    updateTelemetry();

    // Mantener conexión WiFi / Maintain WiFi connection
    if (!WiFi.isConnected())
    {
      Serial.println("WiFi disconnected - Reconnecting...");
      connectToWiFi();
    }

    // Mantener conexión MQTT / Maintain MQTT connection
    if (!mqttClient.connected())
    {
      if (!connectToMQTT())
      {
        delay(WIFI_RETRY_DELAY);
        return;
      }
    }
    mqttClient.loop(); // Procesar mensajes MQTT / Process MQTT messages

    // Capturar e publicar imagen si es tiempo / Capture and publish image if interval elapsed
    if (millis() - lastCaptureTime >= CAPTURE_INTERVAL)
    {
      captureAndPublishImage();
      lastCaptureTime = millis();
    }

    // Enviar telemetría cada 30 segundos / Send telemetry every 30 seconds
    if (millis() - lastHeartbeatTime >= MQTT_HEARTBEAT)
    {
      publishTelemetry();
      lastHeartbeatTime = millis();
    }

    // Actualizar LED de estado / Update status LED indicator
    blinkStatusLED();
  }

  // Funciones de inicialización / Initialization Functions
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    publishStatus("camera_error");
    return;
    // Inicializar cámara / Initialize Camera
    // Configura los pines GPIO y ajusta parámetros de sensor OV2640
    void initializeCamera()
    {
      // Intentar inicializar cámara con configuración / Attempt camera initialization
      esp_err_t err = esp_camera_init(&camera_config);
      if (err != ESP_OK)
      {
        Serial.printf("ERROR: Camera init failed (0x%x)\n", err);
        publishStatus("camera_error");
        return;
      }

      // Obtener sensor y ajustar parámetros / Get sensor and adjust parameters
      sensor_t *s = esp_camera_sensor_get();
      s->set_brightness(s, 0);               // Brillo / Brightness: -2 to 2
      s->set_contrast(s, 0);                 // Contraste / Contrast: -2 to 2
      s->set_saturation(s, 0);               // Saturación / Saturation: -2 to 2
      s->set_gainceiling(s, GAINCEILING_2x); // Techo de ganancia / Gain ceiling
      s->set_gain(s, 0);                     // Ganancia / Gain
      s->set_exposure(s, -5);                // Exposición / Exposure: -20 to 20
      s->set_gp_gain(s, 0);
      s->set_bpc(s, 1); // Corrección píxel negro / Black pixel correction
      s->set_wpc(s, 1); // Corrección píxel blanco / White pixel correction
      // Conectar a red WiFi / Connect to WiFi Network
      // Intenta conexión WPA2 con reintentos automáticos
      void connectToWiFi()
      {
        Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        // Esperar hasta 10 segundos para conectar / Wait up to 10 seconds for connection
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
    Serial.println("\nERROR: 
    Serial.println("\nFailed to connect to WiFi");
        }
      }

      // Conectar al broker MQTT / Connect to MQTT Broker
      // Usa autenticación username/password para conexión segura
      bool connectToMQTT()
      {
        Serial.printf("Connecting to MQTT broker: %s:%d\n", MQTT_BROKER, MQTT_PORT);

        if (mqttClient.connect(DEVICE_ID, MQTT_USERNAME, MQTT_PASSWORD))
        {
          Serial.println("MQTT connected successfully");

          // Suscribirse a comandos remotos / Subscribe to command topic
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

      // Funciones de Cámara e Imágenes / Camera and Image Functions

      // Capturar y publicar imagen vía MQTT / Capture and Publish Image via MQTT
      // Comprime en JPEG y envía al broker con timestamp incluido
      void captureAndPublishImage()
      {
        // Obtener buffer de fotograma / Get frame buffer from camera
        camera_fb_t *fb = esp_camera_fb_get();

        if (!fb)
        {
          Serial.println("ERROR: Camera capture failed");
          return;
        }

        // Crear tópico con timestamp único / Create unique topic with timestamp
        char topic_with_timestamp[100];
        snprintf(topic_with_timestamp, sizeof(topic_with_timestamp),
                 "%s/%s/%ld", TOPIC_IMAGE, DEVICE_ID, millis());

        // Publicar datos binarios de imagen / Publish binary image data
        // QoS 0 para máxima velocidad / QoS 0 for speed
        boolean result = mqttClient.publish(
            (char *)topic_with_timestamp,
            fb->buf,
            fb->len,
            false);

        if (result)
        {
          telemetry.captureCount++;
          telemetry.mqttPublished++;
          Serial.printf("Image published: %d bytes\n", fb->len);
        }
        else
        {
          Serial.println("ERROR: Image publish failed");
        }

        // Liberar buffer / Release frame buffer
        esp_camera_fb_return(fb);
      }

      // Funciones MQTT / MQTT Functions

      // Publicar estado del dispositivo / Publish Device Status
      // Envía JSON con estado actual y tiempo activo
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

        Serial.printf("Status published: %s\n", status);
      }

      // Publicar telemetría del sistema / Publish System Telemetry
      // Envía métricas: señal WiFi, memoria libre, capturas, uptime
      void publishTelemetry()
      {
        StaticJsonDocument<400> doc;
        doc["device_id"] = DEVICE_ID;
        doc["wifi_signal_dbm"] = telemetry.wifiSignal;   // Potencia WiFi / WiFi signal
        doc["free_heap"] = telemetry.freeHeap;           // Memoria disponible / Free memory
        doc["total_heap"] = telemetry.totalHeap;         // Memoria total / Total memory
        doc["uptime_sec"] = telemetry.uptime / 1000;     // Tiempo activo en seg / Uptime seconds
        doc["capture_count"] = telemetry.captureCount;   // Total capturas / Total captures
        doc["mqtt_published"] = telemetry.mqttPublished; // Imágenes publicadas / Published images
        doc["timestamp"] = millis();

        char buffer[512];
        size_t n = serializeJson(doc, buffer);
        boolean result = mqttClient.publish(TOPIC_TELEMETRY, buffer, n);

        if (result)
        {
          Serial.printf("Telemetry sent: heap=%u bytes, uptime=%lu sec\n",
                        telemetry.freeHeap, telemetry.uptime / 1000);
        }
      }
      // Callback para mensajes MQTT recibidos / MQTT Message Callback
      // Procesa comandos remotos en formato JSON
      void mqttMessageCallback(char *topic, byte *payload, unsigned int length)
      {
        Serial.printf("Command received on topic: %s\n", topic);

        // Parsear JSON / Parse JSON command
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, payload, length);

        if (error)
        {
          Serial.printf("ERROR: JSON parse failed - %s\n", error.c_str());
          return;
        }

        // Comando capturar / Capture command: {"command": "capture"}
        if (doc.containsKey("command"))
        {
          String cmd = doc["command"].as<String>();
          if (cmd == "capture")
          {
            Serial.println("Remote capture command received");
            captureAndPublishImage();
          }
          else if (cmd == "status")
          {
            Serial.println("Status request received");
            publishStatus("healthy");
          }
          else if (cmd == "heartbeat")
          {
            Serial.println("Heartbeat request received");
      publishTelemetry(
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
        tFunciones Utilidad / Utility Functions

            // Actualizar métricas de telemetría / Update Telemetry Metrics
            // Lee estado del sistema: tiempo activo, memoria, señal WiFi
            void updateTelemetry()
        {
          telemetry.uptime = millis();             // Tiempo desde encendido / Time since startup
          telemetry.freeHeap = ESP.getFreeHeap();  // Memoria RAM disponible / Free RAM bytes
          telemetry.totalHeap = ESP.getHeapSize(); // Memoria RAM total / Total RAM bytes
          telemetry.wifiSignal = WiFi.RSSI();      // Potencia WiFi / WiFi signal strength
        }

        // Parpadear LED de estado / Blink Status LED
        // Rápido (5Hz) = desconectado / Fast = disconnected
        // Lento (1Hz) = conectado / Slow = connected
        void blinkStatusLED()
        {
          static unsigned long lastBlink = 0;
          // Intervalo rápido si desconectado, lento si conectado
          // Fast blink if disconnected, slow if connected
          unsigned long blinkInterval = mqttClient.connected() ? 1000 : 200;

          if (millis() - lastBlink >= blinkInterval)
          {
            ledState = !ledState;
            digitalWrite(PIN_STATUS_LED, ledState ? HIGH : LOW);
            lastBlink = millis();
          }
        }