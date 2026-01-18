#include <esp_now.h>
#include <WiFi.h>
#include "Config.h"
#include "CarData.h"
#include "BLEManager.h"
#include "OBDParser.h"
#include "PIDCommands.h"

// Receiver MAC address (update this with your receiver's MAC)
uint8_t receiverMAC[] = RECEIVER_MAC;

// Global instances
CarData carData;
BLEManager bleManager;
OBDParser obdParser(&carData);

// ESP-NOW send status
bool espNowReady = false;
bool lastSendSuccess = false;

// ESP-NOW send callback (updated for ESP32 Arduino Core 3.x)
void onDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  lastSendSuccess = (status == ESP_NOW_SEND_SUCCESS);
  if (!lastSendSuccess) {
    Serial.println("ESP-NOW send failed!");
  }
}

// BLE notification callback
void notifyCallback(BLERemoteCharacteristic* chr, uint8_t* data, size_t len, bool isNotify) {
  for (int i = 0; i < len; i++) {
    obdParser.processChar((char)data[i]);
  }
}

bool initESPNow() {
  // Set WiFi to station mode (required for ESP-NOW)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.print("Sender MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return false;
  }

  esp_now_register_send_cb(onDataSent);

  // Register peer (receiver)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = ESPNOW_CHANNEL;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer!");
    return false;
  }

  Serial.println("ESP-NOW initialized!");
  return true;
}

void sendCarData() {
  carData.timestamp = millis();

  esp_err_t result = esp_now_send(receiverMAC, (uint8_t*)&carData, sizeof(CarData));

  if (result == ESP_OK) {
    Serial.println("Data sent via ESP-NOW");
  } else {
    Serial.println("ESP-NOW send error");
  }
}

void printCarData() {
  Serial.println("\n========================================");
  Serial.printf("RPM:                 %4d rpm\n", carData.rpm);
  Serial.printf("Speed:               %4d km/h\n", carData.speed);
  Serial.printf("Coolant Temp:        %4d C\n", carData.coolantTemp);
  Serial.printf("Intake Air Temp:     %4d C\n", carData.intakeTemp);
  Serial.printf("Throttle Position:   %4d %%\n", carData.throttle);
  Serial.printf("Engine Load:         %4d %%\n", carData.engineLoad);
  Serial.printf("Manifold Pressure:   %4d kPa\n", carData.manifoldPressure);
  Serial.printf("Fuel Level:          %4d %%\n", carData.fuelLevel);
  Serial.printf("Timing Advance:      %4d deg\n", carData.timingAdvance);
  Serial.println("========================================\n");
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1000);

  Serial.println("\n\n================================");
  Serial.println("TSI Telemetry - OBD Sender");
  Serial.println("================================\n");

  // Initialize ESP-NOW first (before BLE)
  if (!initESPNow()) {
    Serial.println("ESP-NOW init failed! Restarting...");
    delay(3000);
    ESP.restart();
  }

  // Initialize BLE
  if (!bleManager.begin()) {
    Serial.println("BLE init failed!");
    return;
  }

  // Connect to OBD adapter
  if (!bleManager.connect(OBD_ADDRESS)) {
    Serial.println("OBD connection failed!");
    return;
  }

  bleManager.setNotifyCallback(notifyCallback);

  if (!bleManager.initializeELM327()) {
    Serial.println("ELM327 init failed!");
    return;
  }

  Serial.println("Ready! Reading OBD data and sending via ESP-NOW...\n");
}

void loop() {
  if (!bleManager.isConnected()) {
    Serial.println("OBD disconnected! Reconnecting...");
    delay(5000);
    if (!bleManager.connect(OBD_ADDRESS)) {
      Serial.println("Reconnection failed!");
      return;
    }
    bleManager.setNotifyCallback(notifyCallback);
    bleManager.initializeELM327();
  }

  // Poll all PIDs
  for (int i = 0; i < POLL_COMMANDS_COUNT; i++) {
    String command = String(POLL_COMMANDS[i]) + "\r";
    bleManager.sendCommand(command);
    delay(POLL_DELAY_MS);
  }

  delay(RESPONSE_DELAY_MS);

  // Print data locally
  printCarData();

  // Send via ESP-NOW
  sendCarData();

  delay(DATA_REFRESH_MS);
}
