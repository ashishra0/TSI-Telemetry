/**
 * TSI-Telemetry: Basic ESP32 to BLE OBD-II Connection Framework
 *
 * This is the foundation for:
 * - Scanning for BLE OBD-II adapters
 * - Connecting via BLEClient
 * - Preparing for ELM327 AT commands
 */

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>

static BLEAddress obdAddress("");       // to be discovered via scan
static BLEClient* client = nullptr;
bool connected = false;

// Change this if your adapter exposes a specific BLE name
const char* TARGET_NAME = "OBDII";  // Common names: “OBDII”, “OBD2”, “V-LINK”, “ELM327”

void setup() {
  Serial.begin(115200);
  Serial.println("Starting ESP32 BLE OBD-II Framework...");

  BLEDevice::init("");

  // Scan for adapters
  Serial.println("Scanning for BLE OBD adapters...");
  BLEScan* scan = BLEDevice::getScan();
  scan->setActiveScan(true);

  BLEScanResults results = scan->start(5);
  Serial.printf("Found %d BLE devices\n", results.getCount());

  for (int i = 0; i < results.getCount(); i++) {
    BLEAdvertisedDevice dev = results.getDevice(i);
    Serial.println(dev.toString().c_str());

    if (dev.getName() == TARGET_NAME) {
      Serial.println("FOUND OBD-II adapter!");
      obdAddress = dev.getAddress();
    }
  }

  if (obdAddress.toString() == "") {
    Serial.println("ERROR: No OBD-II BLE device named \"OBDII\" found.");
    return;
  }

  // Attempt connect
  Serial.printf("Connecting to: %s\n", obdAddress.toString().c_str());
  client = BLEDevice::createClient();

  if (client->connect(obdAddress)) {
    connected = true;
    Serial.println("Connected to OBD-II adapter!");
  } else {
    Serial.println("Failed to connect.");
  }
}

void loop() {
  if (!connected) {
    Serial.println("Not connected. Restart ESP32 to retry.");
    delay(3000);
    return;
  }

  // At this stage you are connected over BLE.
  // Actual ELM327 AT command service discovery will be added next.
  Serial.println("BLE link alive...");
  delay(2000);
}
