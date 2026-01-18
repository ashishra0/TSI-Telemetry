#ifndef OBDPARSER_H
#define OBDPARSER_H

#include <Arduino.h>
#include "CarData.h"

class OBDParser {
public:
  OBDParser(CarData* data);

  void processChar(char c);
  void parseResponse(const String& response);
  String getBuffer() const;
  void clearBuffer();

private:
  CarData* carData;
  String responseBuffer;

  int hexCharToInt(char c);
  int hexByteToInt(const String& hex);
  void parsePID(const String& pid, int a, int b);
};

#endif
