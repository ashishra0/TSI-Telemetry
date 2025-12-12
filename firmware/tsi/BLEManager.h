#ifndef BLEMANAGER_H
#define BLEMANAGER_H

#include <BLEDevice.h>
#include <Arduino.h>
#include "Config.h"

class BLEManager {
public:
  BLEManager();

  // Initialize BLE subsystem
  bool begin();

  // Connect to OBD adapter
  bool connect(const String& address);

  // Check connection status
  bool isConnected();

  // Send command to OBD adapter
  bool sendCommand(const String& command);

  // Register callback for received data
  void setNotifyCallback(notify_callback callback);

  // Initialize ELM327 with AT commands
  bool initializeELM327();

private:
  BLEClient* client;
  BLERemoteCharacteristic* txChar;
  BLERemoteCharacteristic* rxChar;

  // Internal helper methods
  bool discoverServices();
};

#endif
