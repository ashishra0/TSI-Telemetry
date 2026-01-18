#ifndef BLEMANAGER_H
#define BLEMANAGER_H

#include <BLEDevice.h>
#include <Arduino.h>
#include "Config.h"

class BLEManager {
public:
  BLEManager();

  bool begin();
  bool connect(const String& address);
  bool isConnected();
  bool sendCommand(const String& command);
  void setNotifyCallback(notify_callback callback);
  bool initializeELM327();

private:
  BLEClient* client;
  BLERemoteCharacteristic* txChar;
  BLERemoteCharacteristic* rxChar;

  bool discoverServices();
};

#endif
