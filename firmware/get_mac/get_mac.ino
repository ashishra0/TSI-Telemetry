// Simple utility to get the ESP32's MAC address
// Flash this to each ESP32 to find its MAC for ESP-NOW configuration

#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);

  Serial.println("\n\n================================");
  Serial.println("ESP32 MAC Address Finder");
  Serial.println("================================\n");

  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());

  Serial.println("\nFor ESP-NOW configuration:");
  Serial.print("  uint8_t mac[] = {");

  uint8_t mac[6];
  WiFi.macAddress(mac);

  for (int i = 0; i < 6; i++) {
    Serial.printf("0x%02X", mac[i]);
    if (i < 5) Serial.print(", ");
  }
  Serial.println("};");

  Serial.println("\nCopy the MAC address above to configure ESP-NOW.");
  Serial.println("- For the sender: update RECEIVER_MAC in Config.h");
  Serial.println("- The sender's MAC will be auto-accepted by receiver");
}

void loop() {
  delay(10000);
  Serial.println("MAC: " + WiFi.macAddress());
}
