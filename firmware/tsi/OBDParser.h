#ifndef OBDPARSER_H
#define OBDPARSER_H

#include <Arduino.h>
#include "CarData.h"

class OBDParser {
public:
  OBDParser(CarData* data);

  // Process single character from BLE stream
  void processChar(char c);

  // Parse complete OBD response
  void parseResponse(const String& response);

  // Get current response buffer
  String getBuffer() const;

  // Clear response buffer
  void clearBuffer();

private:
  CarData* carData;
  String responseBuffer;

  // Hex conversion utilities
  int hexCharToInt(char c);
  int hexByteToInt(const String& hex);

  // Parse specific PID
  void parsePID(const String& pid, int a, int b);
};

#endif
