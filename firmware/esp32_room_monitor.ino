#include <WiFi.h>
#include <PubSubClient.h>

// Set these values locally. Never commit real credentials.
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_SERVER = "YOUR_HOME_ASSISTANT_IP";
const int MQTT_PORT = 1883;
const char* MQTT_USER = "YOUR_MQTT_USERNAME";
const char* MQTT_PASSWORD = "YOUR_MQTT_PASSWORD";

#define PIR_PIN 27
#define SOUND_PIN 26
const bool SOUND_ACTIVE_LOW = true;
const char* TOPIC_MOTION = "home/motion";
const char* TOPIC_SOUND = "home/sound";
const char* TOPIC_STATUS = "home/esp8266/status"; // Legacy topic name
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
unsigned long lastWiFiAttempt = 0, lastMQTTAttempt = 0, lastSensorRead = 0;
const unsigned long RECONNECT_INTERVAL = 5000, SENSOR_INTERVAL = 50, PIR_WARMUP = 60000;
unsigned long startupTime;
bool previousMotion = false, previousSound = false, firstPublish = true;

void connectWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}
void connectMQTT() {
  if (WiFi.status() != WL_CONNECTED) return;
  String clientID = "ESP32_" + String((uint32_t)ESP.getEfuseMac(), HEX);
  bool connected = mqttClient.connect(clientID.c_str(), MQTT_USER,
      MQTT_PASSWORD, TOPIC_STATUS, 0, true, "offline");
  if (connected) {
    Serial.println("MQTT CONNECTED!");
    mqttClient.publish(TOPIC_STATUS, "online", true);
    firstPublish = true;
  } else {
    Serial.print("MQTT Failed: ");
    Serial.println(mqttClient.state());
  }
}
void readSensors() {
  bool motion = digitalRead(PIR_PIN) == HIGH;
  bool sound = SOUND_ACTIVE_LOW ? digitalRead(SOUND_PIN) == LOW
                                : digitalRead(SOUND_PIN) == HIGH;
  if (firstPublish || motion != previousMotion) {
    if (mqttClient.publish(TOPIC_MOTION, motion ? "ON" : "OFF", true)) {
      previousMotion = motion;
      Serial.print("MOTION: ");
      Serial.println(motion ? "DETECTED" : "NONE");
    }
  }
  if (firstPublish || sound != previousSound) {
    if (mqttClient.publish(TOPIC_SOUND, sound ? "ON" : "OFF", true)) {
      previousSound = sound;
      Serial.print("SOUND: ");
      Serial.println(sound ? "DETECTED" : "NONE");
    }
  }
  firstPublish = false;
}
void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  connectWiFi();
  startupTime = millis();
  Serial.println("ESP32 SMART HOME SYSTEM STARTED");
}
void loop() {
  unsigned long now = millis();
  if (WiFi.status() != WL_CONNECTED) {
    if (now - lastWiFiAttempt >= RECONNECT_INTERVAL) {
      lastWiFiAttempt = now;
      connectWiFi();
    }
    delay(10);
    return;
  }
  static bool wifiPrinted = false;
  if (!wifiPrinted) {
    Serial.println("WIFI CONNECTED!");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
    wifiPrinted = true;
  }
  if (!mqttClient.connected()) {
    if (now - lastMQTTAttempt >= RECONNECT_INTERVAL) {
      lastMQTTAttempt = now;
      connectMQTT();
    }
    delay(10);
    return;
  }
  mqttClient.loop();
  if (now - startupTime < PIR_WARMUP) return;
  if (now - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = now;
    readSensors();
  }
}
