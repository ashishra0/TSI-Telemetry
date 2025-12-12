#include "Config.h"
#include "CarData.h"
#include "BLEManager.h"
#include "OBDParser.h"
#include "DisplayManager.h"
#include "PIDCommands.h"

// Global instances
CarData carData;
BLEManager bleManager;
OBDParser obdParser(&carData);
DisplayManager display;

// BLE notification callback (bridges C-style callback to C++ class)
void notifyCallback(BLERemoteCharacteristic* chr, uint8_t* data, size_t len, bool isNotify) {
  for (int i = 0; i < len; i++) {
    obdParser.processChar((char)data[i]);
  }
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(500);

  display.printBanner();

  if (!bleManager.begin()) {
    display.printError("BLE init failed!");
    return;
  }

  if (!bleManager.connect(OBD_ADDRESS)) {
    display.printError("Connection failed!");
    return;
  }

  bleManager.setNotifyCallback(notifyCallback);

  if (!bleManager.initializeELM327()) {
    display.printError("ELM327 init failed!");
    return;
  }

  display.printStatus("Ready! Reading telemetry...\n");
}

void loop() {
  if (!bleManager.isConnected()) {
    display.printError("Disconnected!");
    delay(5000);
    return;
  }

  // Poll all PIDs
  for (int i = 0; i < POLL_COMMANDS_COUNT; i++) {
    String command = String(POLL_COMMANDS[i]) + "\r";
    bleManager.sendCommand(command);
    delay(POLL_DELAY_MS);
  }

  delay(RESPONSE_DELAY_MS);
  display.printCarData(carData);
  delay(DATA_REFRESH_MS);
}
