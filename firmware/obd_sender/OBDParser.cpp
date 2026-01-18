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
    // RPM: ((A * 256) + B) / 4
    carData->rpm = ((a * 256) + b) / 4;
  }
  else if (pid == "0D") {
    // Speed: A (km/h)
    carData->speed = a;
  }
  else if (pid == "05") {
    // Coolant temp: A - 40 (°C)
    carData->coolantTemp = a - 40;
  }
  else if (pid == "0F") {
    // Intake air temp: A - 40 (°C)
    carData->intakeTemp = a - 40;
  }
  else if (pid == "11") {
    // Throttle position: (A * 100) / 255 (%)
    carData->throttle = (a * 100) / 255;
  }
  else if (pid == "04") {
    // Engine load: (A * 100) / 255 (%)
    carData->engineLoad = (a * 100) / 255;
  }
  else if (pid == "0B") {
    // Manifold pressure: A (kPa)
    carData->manifoldPressure = a;
  }
  else if (pid == "2F") {
    // Fuel level: (A * 100) / 255 (%)
    carData->fuelLevel = (a * 100) / 255;
  }
  else if (pid == "0E") {
    // Timing advance: (A / 2) - 64 (degrees)
    carData->timingAdvance = (a / 2) - 64;
  }
  // New PIDs for transmission diagnosis
  else if (pid == "A4") {
    // Actual gear: Lower nibble of byte A contains gear (0=N, 1-7)
    // Some ECUs return gear directly, some use bitmask
    carData->actualGear = a & 0x0F;
  }
  else if (pid == "42") {
    // Control module voltage: ((A * 256) + B) / 1000 (V)
    // Store as V * 10 for 1 decimal precision (e.g., 125 = 12.5V)
    carData->batteryVoltage = ((a * 256) + b) / 100;
  }
  else if (pid == "61") {
    // Driver demanded torque: A - 125 (%)
    carData->demandedTorque = a - 125;
  }
  else if (pid == "62") {
    // Actual engine torque: A - 125 (%)
    carData->actualTorque = a - 125;
  }
  else if (pid == "5E") {
    // Fuel rate: ((A * 256) + B) / 20 (L/h)
    // Store as L/h * 10 for 1 decimal precision
    carData->fuelRate = ((a * 256) + b) / 2;
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
