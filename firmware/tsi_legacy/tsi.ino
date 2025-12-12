#include <BLEDevice.h>

BLEClient* client = nullptr;
BLERemoteCharacteristic* txChar = nullptr;
BLERemoteCharacteristic* rxChar = nullptr;

String obdAddress = "00:33:cc:4f:36:03";
BLEUUID serviceUUID("0000fff0-0000-1000-8000-00805f9b34fb");
BLEUUID txUUID("0000fff2-0000-1000-8000-00805f9b34fb");
BLEUUID rxUUID("0000fff1-0000-1000-8000-00805f9b34fb");

String responseBuffer = "";

struct CarData {
  int rpm = 0;
  int speed = 0;
  int coolantTemp = 0;
  int intakeTemp = 0;
  int throttle = 0;
  int engineLoad = 0;
  int manifoldPressure = 0;
  int fuelLevel = 0;
  int timingAdvance = 0;
};

CarData carData;

int hexCharToInt(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return 0;
}

int hexByteToInt(String hex) {
  if (hex.length() < 2) return 0;
  return (hexCharToInt(hex[0]) * 16) + hexCharToInt(hex[1]);
}

void parseOBDResponse(String response) {
  response.trim();

  if (response == "NO DATA" || response == "NODATA") {
    return;
  }

  if (!response.startsWith("41")) return;

  if (response.length() < 6) return;

  String pid = response.substring(2, 4);
  String dataBytes = response.substring(4);

  int a = 0;
  int b = 0;

  if (dataBytes.length() >= 2) {
    a = hexByteToInt(dataBytes.substring(0, 2));
  }
  if (dataBytes.length() >= 4) {
    b = hexByteToInt(dataBytes.substring(2, 4));
  }

  if (pid == "0C") {
    carData.rpm = ((a * 256) + b) / 4;
  }
  else if (pid == "0D") {
    carData.speed = a;
  }
  else if (pid == "05") {
    carData.coolantTemp = a - 40;
  }
  else if (pid == "0F") {
    carData.intakeTemp = a - 40;
  }
  else if (pid == "11") {
    carData.throttle = (a * 100) / 255;
  }
  else if (pid == "04") {
    carData.engineLoad = (a * 100) / 255;
  }
  else if (pid == "0B") {
    carData.manifoldPressure = a;
  }
  else if (pid == "2F") {
    carData.fuelLevel = (a * 100) / 255;
  }
  else if (pid == "0E") {
    carData.timingAdvance = (a / 2) - 64;
  }
}

void notifyCallback(BLERemoteCharacteristic* chr, uint8_t* data, size_t len, bool isNotify) {
  for (int i = 0; i < len; i++) {
    char c = (char)data[i];

    if (c == '\r' || c == '\n' || c == '>') {
      if (responseBuffer.length() > 0) {
        parseOBDResponse(responseBuffer);
        responseBuffer = "";
      }
    } else {
      responseBuffer += c;
    }
  }
}

void printCarData() {
  Serial.println("\n========================================");

  Serial.printf("RPM:                 %4d rpm\n", carData.rpm);
  Serial.printf("Speed:               %4d km/h\n", carData.speed);
  Serial.printf("Coolant Temp:        %4d °C\n", carData.coolantTemp);
  Serial.printf("Intake Air Temp:     %4d °C\n", carData.intakeTemp);
  Serial.printf("Throttle Position:   %4d %%\n", carData.throttle);
  Serial.printf("Engine Load:         %4d %%\n", carData.engineLoad);
  Serial.printf("Manifold Pressure:   %4d kPa", carData.manifoldPressure);

  int boost = carData.manifoldPressure - 100;
  if (boost > 0) {
    Serial.printf(" (+%d kPa BOOST!)", boost);
  }
  Serial.println();

  Serial.printf("Fuel Level:          %4d %%\n", carData.fuelLevel);
  Serial.printf("Timing Advance:      %4d degrees\n", carData.timingAdvance);

  Serial.println("========================================\n");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\nStarting TSI Telemetry...");

  BLEDevice::init("");

  BLEAddress address(obdAddress.c_str());
  client = BLEDevice::createClient();

  Serial.println("Connecting to OBDII adapter...");
  if (!client->connect(address)) {
    Serial.println("Connection failed!");
    return;
  }

  Serial.println("Connected!");

  BLERemoteService* service = client->getService(serviceUUID);
  if (service == nullptr) {
    Serial.println("Service not found!");
    return;
  }

  txChar = service->getCharacteristic(txUUID);
  rxChar = service->getCharacteristic(rxUUID);

  if (txChar == nullptr || rxChar == nullptr) {
    Serial.println("Characteristics not found!");
    return;
  }

  rxChar->registerForNotify(notifyCallback);

  Serial.println("Initializing ELM327...");
  delay(1000);

  txChar->writeValue("ATZ\r");
  delay(2000);

  txChar->writeValue("ATE0\r");
  delay(500);

  txChar->writeValue("ATL0\r");
  delay(500);

  txChar->writeValue("ATS0\r");
  delay(500);

  Serial.println("Ready! Reading telemetry...\n");
}

void loop() {
  if (!client->isConnected()) {
    Serial.println("Disconnected!");
    delay(5000);
    return;
  }

  txChar->writeValue("010C\r"); delay(100);
  txChar->writeValue("010D\r"); delay(100);
  txChar->writeValue("0105\r"); delay(100);
  txChar->writeValue("010F\r"); delay(100);
  txChar->writeValue("0111\r"); delay(100);
  txChar->writeValue("0104\r"); delay(100);
  txChar->writeValue("010B\r"); delay(100);
  txChar->writeValue("012F\r"); delay(100);
  txChar->writeValue("010E\r"); delay(100);

  delay(400);

  printCarData();

  delay(1000);
}
