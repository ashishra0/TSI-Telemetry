#include "OBDParser.h"

OBDParser::OBDParser(CarData* data) : carData(data), responseBuffer("") {}

void OBDParser::processChar(char c) {
  if (c == '\r' || c == '\n' || c == '>') {
    if (responseBuffer.length() > 0) {
      parseResponse(responseBuffer);
      responseBuffer = "";
    }
  } else {
    responseBuffer += c;
  }
}

void OBDParser::parseResponse(const String& response) {
  String trimmedResponse = response;
  trimmedResponse.trim();

  if (trimmedResponse == "NO DATA" || trimmedResponse == "NODATA") {
    return;
  }

  if (!trimmedResponse.startsWith("41")) return;

  if (trimmedResponse.length() < 6) return;

  String pid = trimmedResponse.substring(2, 4);
  String dataBytes = trimmedResponse.substring(4);

  int a = 0;
  int b = 0;

  if (dataBytes.length() >= 2) {
    a = hexByteToInt(dataBytes.substring(0, 2));
  }
  if (dataBytes.length() >= 4) {
    b = hexByteToInt(dataBytes.substring(2, 4));
  }

  parsePID(pid, a, b);
}

void OBDParser::parsePID(const String& pid, int a, int b) {
  if (pid == "0C") {
    carData->rpm = ((a * 256) + b) / 4;
  }
  else if (pid == "0D") {
    carData->speed = a;
  }
  else if (pid == "05") {
    carData->coolantTemp = a - 40;
  }
  else if (pid == "0F") {
    carData->intakeTemp = a - 40;
  }
  else if (pid == "11") {
    carData->throttle = (a * 100) / 255;
  }
  else if (pid == "04") {
    carData->engineLoad = (a * 100) / 255;
  }
  else if (pid == "0B") {
    carData->manifoldPressure = a;
  }
  else if (pid == "2F") {
    carData->fuelLevel = (a * 100) / 255;
  }
  else if (pid == "0E") {
    carData->timingAdvance = (a / 2) - 64;
  }
}

int OBDParser::hexCharToInt(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return 0;
}

int OBDParser::hexByteToInt(const String& hex) {
  if (hex.length() < 2) return 0;
  return (hexCharToInt(hex[0]) * 16) + hexCharToInt(hex[1]);
}

String OBDParser::getBuffer() const {
  return responseBuffer;
}

void OBDParser::clearBuffer() {
  responseBuffer = "";
}
