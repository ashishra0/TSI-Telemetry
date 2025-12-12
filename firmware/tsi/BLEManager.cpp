#include "BLEManager.h"

BLEManager::BLEManager() : client(nullptr), txChar(nullptr), rxChar(nullptr) {}

bool BLEManager::begin() {
  BLEDevice::init("");
  client = BLEDevice::createClient();
  return client != nullptr;
}

bool BLEManager::connect(const String& address) {
  if (client == nullptr) {
    return false;
  }

  Serial.println("Connecting to OBDII adapter...");

  BLEAddress bleAddress(address.c_str());
  if (!client->connect(bleAddress)) {
    Serial.println("Connection failed!");
    return false;
  }

  Serial.println("Connected!");

  return discoverServices();
}

bool BLEManager::discoverServices() {
  BLEUUID serviceUUID(SERVICE_UUID);
  BLERemoteService* service = client->getService(serviceUUID);

  if (service == nullptr) {
    Serial.println("Service not found!");
    return false;
  }

  BLEUUID txUUID(TX_UUID);
  BLEUUID rxUUID(RX_UUID);

  txChar = service->getCharacteristic(txUUID);
  rxChar = service->getCharacteristic(rxUUID);

  if (txChar == nullptr || rxChar == nullptr) {
    Serial.println("Characteristics not found!");
    return false;
  }

  return true;
}

bool BLEManager::isConnected() {
  return client != nullptr && client->isConnected();
}

bool BLEManager::sendCommand(const String& command) {
  if (txChar == nullptr) {
    return false;
  }

  txChar->writeValue(command.c_str());
  return true;
}

void BLEManager::setNotifyCallback(notify_callback callback) {
  if (rxChar != nullptr) {
    rxChar->registerForNotify(callback);
  }
}

bool BLEManager::initializeELM327() {
  Serial.println("Initializing ELM327...");
  delay(1000);

  sendCommand("ATZ\r");
  delay(2000);

  sendCommand("ATE0\r");
  delay(500);

  sendCommand("ATL0\r");
  delay(500);

  sendCommand("ATS0\r");
  delay(500);

  return true;
}
