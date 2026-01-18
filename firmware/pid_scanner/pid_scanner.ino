/*
 * OBD-II PID Scanner for VW Polo GT TSI
 *
 * This utility connects to your ELM327 BLE adapter and discovers
 * which OBD-II PIDs your car supports.
 *
 * Flash this to your ESP32, open Serial Monitor at 115200 baud,
 * and it will scan and report all supported PIDs.
 */

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>

// Copy these from your obd_sender/Config.h
#define OBD_ADDRESS "00:33:cc:4f:36:03"
#define SERVICE_UUID "0000fff0-0000-1000-8000-00805f9b34fb"
#define TX_UUID "0000fff2-0000-1000-8000-00805f9b34fb"
#define RX_UUID "0000fff1-0000-1000-8000-00805f9b34fb"

BLEClient* pClient = nullptr;
BLERemoteCharacteristic* pTxChar = nullptr;
BLERemoteCharacteristic* pRxChar = nullptr;

String responseBuffer = "";
bool responseComplete = false;

// PID names for Mode 01
const char* PID_NAMES[] = {
  /* 00 */ "Supported PIDs 01-20",
  /* 01 */ "Monitor status since DTCs cleared",
  /* 02 */ "Freeze DTC",
  /* 03 */ "Fuel system status",
  /* 04 */ "Calculated engine load",
  /* 05 */ "Engine coolant temperature",
  /* 06 */ "Short term fuel trim - Bank 1",
  /* 07 */ "Long term fuel trim - Bank 1",
  /* 08 */ "Short term fuel trim - Bank 2",
  /* 09 */ "Long term fuel trim - Bank 2",
  /* 0A */ "Fuel pressure",
  /* 0B */ "Intake manifold pressure",
  /* 0C */ "Engine RPM",
  /* 0D */ "Vehicle speed",
  /* 0E */ "Timing advance",
  /* 0F */ "Intake air temperature",
  /* 10 */ "MAF air flow rate",
  /* 11 */ "Throttle position",
  /* 12 */ "Commanded secondary air status",
  /* 13 */ "Oxygen sensors present (2 banks)",
  /* 14 */ "Oxygen Sensor 1 - Voltage",
  /* 15 */ "Oxygen Sensor 2 - Voltage",
  /* 16 */ "Oxygen Sensor 3 - Voltage",
  /* 17 */ "Oxygen Sensor 4 - Voltage",
  /* 18 */ "Oxygen Sensor 5 - Voltage",
  /* 19 */ "Oxygen Sensor 6 - Voltage",
  /* 1A */ "Oxygen Sensor 7 - Voltage",
  /* 1B */ "Oxygen Sensor 8 - Voltage",
  /* 1C */ "OBD standards this vehicle conforms to",
  /* 1D */ "Oxygen sensors present (4 banks)",
  /* 1E */ "Auxiliary input status",
  /* 1F */ "Run time since engine start",
  /* 20 */ "Supported PIDs 21-40",
  /* 21 */ "Distance traveled with MIL on",
  /* 22 */ "Fuel Rail Pressure (relative)",
  /* 23 */ "Fuel Rail Gauge Pressure (diesel)",
  /* 24 */ "O2 Sensor 1 - Equivalence Ratio",
  /* 25 */ "O2 Sensor 2 - Equivalence Ratio",
  /* 26 */ "O2 Sensor 3 - Equivalence Ratio",
  /* 27 */ "O2 Sensor 4 - Equivalence Ratio",
  /* 28 */ "O2 Sensor 5 - Equivalence Ratio",
  /* 29 */ "O2 Sensor 6 - Equivalence Ratio",
  /* 2A */ "O2 Sensor 7 - Equivalence Ratio",
  /* 2B */ "O2 Sensor 8 - Equivalence Ratio",
  /* 2C */ "Commanded EGR",
  /* 2D */ "EGR Error",
  /* 2E */ "Commanded evaporative purge",
  /* 2F */ "Fuel tank level input",
  /* 30 */ "Warm-ups since codes cleared",
  /* 31 */ "Distance traveled since codes cleared",
  /* 32 */ "Evap system vapor pressure",
  /* 33 */ "Absolute barometric pressure",
  /* 34 */ "O2 Sensor 1 - Current",
  /* 35 */ "O2 Sensor 2 - Current",
  /* 36 */ "O2 Sensor 3 - Current",
  /* 37 */ "O2 Sensor 4 - Current",
  /* 38 */ "O2 Sensor 5 - Current",
  /* 39 */ "O2 Sensor 6 - Current",
  /* 3A */ "O2 Sensor 7 - Current",
  /* 3B */ "O2 Sensor 8 - Current",
  /* 3C */ "Catalyst Temperature: Bank 1, Sensor 1",
  /* 3D */ "Catalyst Temperature: Bank 2, Sensor 1",
  /* 3E */ "Catalyst Temperature: Bank 1, Sensor 2",
  /* 3F */ "Catalyst Temperature: Bank 2, Sensor 2",
  /* 40 */ "Supported PIDs 41-60",
  /* 41 */ "Monitor status this drive cycle",
  /* 42 */ "Control module voltage",
  /* 43 */ "Absolute load value",
  /* 44 */ "Commanded air-fuel ratio",
  /* 45 */ "Relative throttle position",
  /* 46 */ "Ambient air temperature",
  /* 47 */ "Absolute throttle position B",
  /* 48 */ "Absolute throttle position C",
  /* 49 */ "Accelerator pedal position D",
  /* 4A */ "Accelerator pedal position E",
  /* 4B */ "Accelerator pedal position F",
  /* 4C */ "Commanded throttle actuator",
  /* 4D */ "Time run with MIL on",
  /* 4E */ "Time since trouble codes cleared",
  /* 4F */ "Max values - fuel-air ratio, O2 voltage, current, intake MAP",
  /* 50 */ "Max value for air flow rate from MAF",
  /* 51 */ "Fuel type",
  /* 52 */ "Ethanol fuel %",
  /* 53 */ "Absolute evap system vapor pressure",
  /* 54 */ "Evap system vapor pressure",
  /* 55 */ "Short term secondary O2 sensor trim - Banks 1/3",
  /* 56 */ "Long term secondary O2 sensor trim - Banks 1/3",
  /* 57 */ "Short term secondary O2 sensor trim - Banks 2/4",
  /* 58 */ "Long term secondary O2 sensor trim - Banks 2/4",
  /* 59 */ "Fuel rail absolute pressure",
  /* 5A */ "Relative accelerator pedal position",
  /* 5B */ "Hybrid battery pack remaining life",
  /* 5C */ "Engine oil temperature",
  /* 5D */ "Fuel injection timing",
  /* 5E */ "Engine fuel rate",
  /* 5F */ "Emission requirements",
  /* 60 */ "Supported PIDs 61-80",
  /* 61 */ "Driver's demand engine torque",
  /* 62 */ "Actual engine torque",
  /* 63 */ "Engine reference torque",
  /* 64 */ "Engine percent torque data",
  /* 65 */ "Auxiliary input/output supported",
  /* 66 */ "Mass air flow sensor",
  /* 67 */ "Engine coolant temperature",
  /* 68 */ "Intake air temperature sensor",
  /* 69 */ "Commanded EGR and EGR Error",
  /* 6A */ "Commanded Diesel intake air flow control",
  /* 6B */ "Exhaust gas recirculation temperature",
  /* 6C */ "Commanded throttle actuator control",
  /* 6D */ "Fuel pressure control system",
  /* 6E */ "Injection pressure control system",
  /* 6F */ "Turbocharger compressor inlet pressure",
  /* 70 */ "Boost pressure control",
  /* 71 */ "Variable Geometry turbo control",
  /* 72 */ "Wastegate control",
  /* 73 */ "Exhaust pressure",
  /* 74 */ "Turbocharger RPM",
  /* 75 */ "Turbocharger temperature",
  /* 76 */ "Turbocharger temperature",
  /* 77 */ "Charge air cooler temperature",
  /* 78 */ "Exhaust Gas temperature Bank 1",
  /* 79 */ "Exhaust Gas temperature Bank 2",
  /* 7A */ "Diesel particulate filter",
  /* 7B */ "Diesel particulate filter",
  /* 7C */ "Diesel particulate filter temperature",
  /* 7D */ "NOx NTE control area status",
  /* 7E */ "PM NTE control area status",
  /* 7F */ "Engine run time",
  /* 80 */ "Supported PIDs 81-A0",
  /* 81-9F */ "Reserved / Various",
  /* A0 */ "Supported PIDs A1-C0",
  /* A1 */ "NOx sensor corrected data",
  /* A2 */ "Cylinder fuel rate",
  /* A3 */ "Evap system vapor pressure",
  /* A4 */ "Transmission actual gear",  // <-- The one we want!
  /* A5 */ "Diesel exhaust fluid dosing",
  /* A6 */ "Odometer"
};

// Notification callback
void notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
  for (size_t i = 0; i < length; i++) {
    char c = (char)pData[i];
    if (c == '>') {
      responseComplete = true;
    } else if (c != '\r' && c != '\n') {
      responseBuffer += c;
    }
  }
}

bool connectToOBD() {
  Serial.println("Scanning for OBD adapter...");

  BLEDevice::init("PID_Scanner");

  BLEAddress targetAddress(OBD_ADDRESS);
  pClient = BLEDevice::createClient();

  Serial.printf("Connecting to %s...\n", OBD_ADDRESS);

  if (!pClient->connect(targetAddress)) {
    Serial.println("Failed to connect!");
    return false;
  }

  Serial.println("Connected! Getting service...");

  BLERemoteService* pService = pClient->getService(BLEUUID(SERVICE_UUID));
  if (pService == nullptr) {
    Serial.println("Failed to find service!");
    pClient->disconnect();
    return false;
  }

  pTxChar = pService->getCharacteristic(BLEUUID(TX_UUID));
  pRxChar = pService->getCharacteristic(BLEUUID(RX_UUID));

  if (pTxChar == nullptr || pRxChar == nullptr) {
    Serial.println("Failed to find characteristics!");
    pClient->disconnect();
    return false;
  }

  // Register for notifications
  pRxChar->registerForNotify(notifyCallback);

  Serial.println("BLE setup complete!");
  return true;
}

String sendCommand(const char* cmd, int timeout = 2000) {
  responseBuffer = "";
  responseComplete = false;

  String cmdStr = String(cmd) + "\r";
  pTxChar->writeValue((uint8_t*)cmdStr.c_str(), cmdStr.length());

  unsigned long start = millis();
  while (!responseComplete && (millis() - start) < timeout) {
    delay(10);
  }

  return responseBuffer;
}

void initELM327() {
  Serial.println("\nInitializing ELM327...");

  sendCommand("ATZ", 3000);  // Reset
  delay(1000);
  sendCommand("ATE0");       // Echo off
  sendCommand("ATL0");       // Linefeeds off
  sendCommand("ATS0");       // Spaces off
  sendCommand("ATH0");       // Headers off
  sendCommand("ATSP0");      // Auto protocol

  Serial.println("ELM327 ready!\n");
}

// Parse supported PIDs bitmask
void parseSupportedPIDs(const String& response, int baseOffset) {
  // Response format: "4100XXXXXXXX" where XXXXXXXX is the bitmask
  if (response.length() < 12) {
    Serial.printf("Invalid response for range %02X: %s\n", baseOffset, response.c_str());
    return;
  }

  // Extract the 4-byte hex bitmask
  String hexMask = response.substring(4, 12);

  // Convert to 32-bit value
  uint32_t mask = 0;
  for (int i = 0; i < 8; i++) {
    char c = hexMask[i];
    int val = 0;
    if (c >= '0' && c <= '9') val = c - '0';
    else if (c >= 'A' && c <= 'F') val = c - 'A' + 10;
    else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
    mask = (mask << 4) | val;
  }

  Serial.printf("\n=== PIDs %02X-%02X (mask: %08X) ===\n",
                baseOffset + 1, baseOffset + 0x20, mask);

  // Check each bit (MSB first)
  for (int i = 0; i < 32; i++) {
    if (mask & (1 << (31 - i))) {
      int pid = baseOffset + i + 1;
      const char* name = (pid < 167) ? PID_NAMES[pid] : "Unknown";
      Serial.printf("  [%02X] %s\n", pid, name);
    }
  }
}

void scanAllPIDs() {
  Serial.println("\n");
  Serial.println("╔════════════════════════════════════════════════════════════╗");
  Serial.println("║           OBD-II PID SCANNER - VW Polo GT TSI              ║");
  Serial.println("╚════════════════════════════════════════════════════════════╝");

  // Query each supported PIDs range
  const char* supportedPIDQueries[] = {
    "0100",  // PIDs 01-20
    "0120",  // PIDs 21-40
    "0140",  // PIDs 41-60
    "0160",  // PIDs 61-80
    "0180",  // PIDs 81-A0
    "01A0",  // PIDs A1-C0
    "01C0"   // PIDs C1-E0
  };

  int offsets[] = {0x00, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xC0};

  for (int i = 0; i < 7; i++) {
    Serial.printf("\nQuerying %s...", supportedPIDQueries[i]);
    String response = sendCommand(supportedPIDQueries[i], 3000);

    if (response.startsWith("41")) {
      parseSupportedPIDs(response, offsets[i]);
    } else if (response.indexOf("NO DATA") >= 0 || response.indexOf("NODATA") >= 0) {
      Serial.printf(" No data (range not supported)\n");
      break;  // If a range isn't supported, higher ranges won't be either
    } else {
      Serial.printf(" Unexpected response: %s\n", response.c_str());
    }

    delay(500);
  }

  // Specifically check the PIDs we added
  Serial.println("\n");
  Serial.println("╔════════════════════════════════════════════════════════════╗");
  Serial.println("║         CHECKING NEW TRANSMISSION DIAGNOSIS PIDs           ║");
  Serial.println("╚════════════════════════════════════════════════════════════╝");

  struct TestPID {
    const char* cmd;
    const char* name;
  };

  TestPID testPIDs[] = {
    {"01A4", "Actual Gear"},
    {"0142", "Control Module Voltage"},
    {"0161", "Driver Demand Torque"},
    {"0162", "Actual Engine Torque"},
    {"015E", "Engine Fuel Rate"},
    {"0163", "Engine Reference Torque"}
  };

  Serial.println("\nTesting individual PIDs:\n");

  for (int i = 0; i < 6; i++) {
    Serial.printf("  %s (%s): ", testPIDs[i].cmd, testPIDs[i].name);
    String response = sendCommand(testPIDs[i].cmd, 2000);

    if (response.startsWith("41")) {
      Serial.printf("SUPPORTED - Response: %s\n", response.c_str());
    } else if (response.indexOf("NO DATA") >= 0 || response.indexOf("NODATA") >= 0) {
      Serial.println("NOT SUPPORTED");
    } else {
      Serial.printf("UNKNOWN - Response: %s\n", response.c_str());
    }

    delay(300);
  }

  Serial.println("\n");
  Serial.println("════════════════════════════════════════════════════════════");
  Serial.println("                     SCAN COMPLETE!");
  Serial.println("════════════════════════════════════════════════════════════");
  Serial.println("\nNote: If a PID shows 'NOT SUPPORTED', the firmware will");
  Serial.println("still work - that field will just remain at 0.");
  Serial.println("\nFor VW DSG transmission data, you may need VCDS/OBDeleven");
  Serial.println("which uses UDS protocol (not standard OBD-II).");
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n\n");
  Serial.println("════════════════════════════════════════════════════════════");
  Serial.println("          TSI-Telemetry PID Scanner Starting...");
  Serial.println("════════════════════════════════════════════════════════════");
  Serial.println("\nMake sure:");
  Serial.println("  1. Car ignition is ON (engine can be off)");
  Serial.println("  2. ELM327 adapter is plugged into OBD port");
  Serial.println("  3. ELM327 MAC address is correct in this sketch\n");

  if (!connectToOBD()) {
    Serial.println("\nFailed to connect to OBD adapter!");
    Serial.println("Check the MAC address and try again.");
    while (1) delay(1000);
  }

  initELM327();
  scanAllPIDs();
}

void loop() {
  // Nothing to do - scan runs once in setup()
  delay(10000);
}
