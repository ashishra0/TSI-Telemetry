#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <esp_now.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "CarData.h"
#include "Credentials.h"

// HiveMQ Cloud root CA certificate
// This is the ISRG Root X1 certificate used by Let's Encrypt
// Valid for HiveMQ Cloud connections
static const char* root_ca = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

// Heartbeat topics for supervisor monitoring
#define TOPIC_HEARTBEAT_RECEIVER "tsi/heartbeat/receiver"
#define TOPIC_HEARTBEAT_SENDER   "tsi/heartbeat/sender"
#define HEARTBEAT_INTERVAL 10000

// Global instances
WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);
CarData carData;
volatile bool newDataReceived = false;
unsigned long lastDataTime = 0;
unsigned long lastHeartbeat = 0;
unsigned long lastMqttRetry = 0;
#define MQTT_RETRY_INTERVAL 5000

// ESP-NOW receive callback
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len == sizeof(CarData)) {
    memcpy(&carData, data, sizeof(CarData));
    newDataReceived = true;
    lastDataTime = millis();

    Serial.print("Data received from: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", info->src_addr[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
  }
}

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Receiver MAC: ");
    Serial.println(WiFi.macAddress());
  } else {
    Serial.println(" Failed!");
  }
}

bool initESPNow() {
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return false;
  }

  esp_now_register_recv_cb(onDataRecv);
  Serial.println("ESP-NOW initialized - waiting for data...");
  return true;
}

void connectMQTT() {
  if (mqttClient.connected()) {
    return;
  }

  // Generate unique client ID using MAC
  String clientId = "tsi-recv-";
  clientId += String(WiFi.macAddress());
  clientId.replace(":", "");

  Serial.print("Connecting to HiveMQ as ");
  Serial.print(clientId);
  Serial.print("...");

  if (mqttClient.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.println(" Connected!");
    Serial.print("MQTT state: ");
    Serial.println(mqttClient.state());
  } else {
    Serial.print(" Failed, rc=");
    Serial.println(mqttClient.state());
  }
}

String buildJSON() {
  JsonDocument doc;

  // Core metrics
  doc["rpm"] = carData.rpm;
  doc["speed"] = carData.speed;
  doc["coolant_temp"] = carData.coolantTemp;
  doc["intake_temp"] = carData.intakeTemp;
  doc["throttle"] = carData.throttle;
  doc["engine_load"] = carData.engineLoad;
  doc["manifold_pressure"] = carData.manifoldPressure;
  doc["fuel_level"] = carData.fuelLevel;
  doc["timing_advance"] = carData.timingAdvance;

  // New transmission diagnosis metrics
  doc["actual_gear"] = carData.actualGear;
  doc["battery_voltage"] = carData.batteryVoltage;  // V * 10 (e.g., 125 = 12.5V)
  doc["demanded_torque"] = carData.demandedTorque;  // % (-125 to +125)
  doc["actual_torque"] = carData.actualTorque;      // % (-125 to +125)
  doc["fuel_rate"] = carData.fuelRate;              // L/h * 10 (e.g., 55 = 5.5 L/h)

  // Calculated values
  int boost = carData.manifoldPressure - 100;
  doc["boost_pressure"] = (boost > 0) ? boost : 0;

  // Torque slip indicator: difference between demanded and actual torque
  // Positive value means engine delivering less than demanded (possible slip)
  doc["torque_slip"] = carData.demandedTorque - carData.actualTorque;

  // Timestamps
  doc["sender_timestamp"] = carData.timestamp;
  doc["receiver_timestamp"] = millis();

  String output;
  serializeJson(doc, output);
  return output;
}

void publishData() {
  String json = buildJSON();

  if (mqttClient.publish(MQTT_TOPIC, json.c_str())) {
    Serial.println("Published to MQTT:");
    Serial.println(json);
  } else {
    Serial.println("MQTT publish failed!");
  }
}

void printCarData() {
  Serial.println("\n========================================");
  Serial.printf("RPM:                 %4d rpm\n", carData.rpm);
  Serial.printf("Speed:               %4d km/h\n", carData.speed);
  Serial.printf("Throttle Position:   %4d %%\n", carData.throttle);
  Serial.printf("Engine Load:         %4d %%\n", carData.engineLoad);
  Serial.println("--- Transmission Diagnosis ---");
  Serial.printf("Actual Gear:         %4d\n", carData.actualGear);
  Serial.printf("Demanded Torque:     %4d %%\n", carData.demandedTorque);
  Serial.printf("Actual Torque:       %4d %%\n", carData.actualTorque);
  Serial.printf("Torque Slip:         %4d %%\n", carData.demandedTorque - carData.actualTorque);
  Serial.println("--- Other Metrics ---");
  Serial.printf("Coolant Temp:        %4d C\n", carData.coolantTemp);
  Serial.printf("Intake Air Temp:     %4d C\n", carData.intakeTemp);
  Serial.printf("Manifold Pressure:   %4d kPa\n", carData.manifoldPressure);
  Serial.printf("Fuel Level:          %4d %%\n", carData.fuelLevel);
  Serial.printf("Timing Advance:      %4d deg\n", carData.timingAdvance);
  Serial.printf("Battery Voltage:     %4.1f V\n", carData.batteryVoltage / 10.0);
  Serial.printf("Fuel Rate:           %4.1f L/h\n", carData.fuelRate / 10.0);
  Serial.println("========================================\n");
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1000);

  Serial.println("\n\n================================");
  Serial.println("TSI Telemetry - MQTT Receiver");
  Serial.println("================================\n");

  // Connect to WiFi first
  connectWiFi();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi required! Restarting...");
    delay(3000);
    ESP.restart();
  }

  // Initialize ESP-NOW (works alongside WiFi in AP_STA mode)
  if (!initESPNow()) {
    Serial.println("ESP-NOW init failed! Restarting...");
    delay(3000);
    ESP.restart();
  }

  // Setup MQTT with TLS
  // Try without certificate verification first to test connection
  espClient.setInsecure();  // Skip cert verification (for testing only)
  // espClient.setCACert(root_ca);  // Re-enable this later

  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setBufferSize(1024);  // Larger buffer for TLS
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);

  // Connect to MQTT
  connectMQTT();

  Serial.println("Ready! Waiting for OBD data via ESP-NOW...\n");
}

void loop() {
  // Maintain MQTT connection (with retry interval)
  if (!mqttClient.connected()) {
    unsigned long now = millis();
    if (now - lastMqttRetry > MQTT_RETRY_INTERVAL) {
      lastMqttRetry = now;
      Serial.printf("MQTT disconnected (state=%d), reconnecting...\n", mqttClient.state());
      connectMQTT();
    }
  }

  if (!mqttClient.loop()) {
    // loop() returns false if not connected
    return;
  }

  // Check for new data from ESP-NOW
  if (newDataReceived) {
    newDataReceived = false;

    printCarData();
    publishData();
  }

  // Check for data timeout
  if (lastDataTime > 0 && (millis() - lastDataTime) > DATA_TIMEOUT_MS) {
    Serial.println("Warning: No data received for 10 seconds");
    lastDataTime = millis();
  }

  // Send heartbeats for supervisor
  if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
    lastHeartbeat = millis();

    // Receiver heartbeat
    String recvHB = "{\"uptime\":" + String(millis()) + ",\"wifi_rssi\":" + String(WiFi.RSSI()) + "}";
    mqttClient.publish(TOPIC_HEARTBEAT_RECEIVER, recvHB.c_str());

    // Sender heartbeat (based on ESP-NOW data reception)
    bool senderAlive = (lastDataTime > 0) && ((millis() - lastDataTime) < DATA_TIMEOUT_MS);
    String senderHB = "{\"alive\":" + String(senderAlive ? "true" : "false") + ",\"last_seen\":" + String(lastDataTime) + "}";
    mqttClient.publish(TOPIC_HEARTBEAT_SENDER, senderHB.c_str());
  }

  delay(10);
}
