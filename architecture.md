## Architecture Overview

### High-Level Flow
```
Car ECU → OBD-II Port → BLE OBD Adapter → ESP32 → WiFi → InfluxDB/MQTT → Grafana Dashboard
```

### Detailed Architecture Diagram
```
                        +----------------------+
                        |      Car ECU         |
                        | (Engine, Sensors...) |
                        +----------+-----------+
                                   |
                                   |  OBD-II (ISO9141/ISO15765)
                                   |
                        +----------v-----------+
                        |   BLE OBD-II Dongle  |
                        |  (ELM327-compatible) |
                        +----------+-----------+
                                   |
                                   | BLE
                                   |
                        +----------v-----------+
                        |       ESP32          |
                        |  - BLE PID Reader    |
                        |  - WiFi Publisher    |
                        |  - JSON Encoder      |
                        +----------+-----------+
                                   |
                                   | WiFi / HTTP / MQTT
                                   |
                    +--------------v----------------+
                    |   Telemetry Backend           |
                    | - InfluxDB / Prometheus       |
                    | - MQTT Broker (optional)      |
                    +--------------+----------------+
                                   |
                                   | SQL / Queries
                                   |
                    +--------------v----------------+
                    |          Grafana              |
                    |   Real-Time Dashboards        |
                    +-------------------------------+
```

---

## Firmware Architecture

The ESP32 firmware follows a modular architecture for maintainability and extensibility.

### Module Organization

```
firmware/tsi/
├── tsi.ino              # Orchestration (setup/loop)
├── Config.h             # Constants (UUIDs, addresses, timing)
├── CarData.h            # Vehicle data structure
├── PIDCommands.h        # OBD-II PID definitions
├── BLEManager           # Bluetooth connection handling
├── OBDParser            # Response parsing & hex conversion
└── DisplayManager       # Serial output formatting
```

### Module Responsibilities

**Config.h**
- BLE UUIDs and OBD adapter address
- Serial baud rate
- Polling delays and timing constants
- Single source of truth for configuration

**CarData.h**
- `CarData` struct with 9 telemetry fields
- rpm, speed, coolantTemp, intakeTemp, throttle, engineLoad, manifoldPressure, fuelLevel, timingAdvance
- Foundation for future JSON serialization

**PIDCommands.h**
- Standard OBD-II PID definitions (010C, 010D, etc.)
- ELM327 AT command strings
- Polling command array
- Easy to add/remove PIDs

**BLEManager**
- BLE device initialization
- Connection to OBD adapter
- Service/characteristic discovery
- ELM327 initialization sequence (ATZ, ATE0, ATL0, ATS0)
- Command transmission
- Connection status monitoring

**OBDParser**
- Hex-to-integer conversion (hexCharToInt, hexByteToInt)
- Character-by-character stream processing
- OBD-II response parsing (41 XX format)
- PID-specific conversion formulas
- Updates CarData structure
- Handles "NO DATA" responses

**DisplayManager**
- Formatted serial output
- Boost pressure calculation
- Status/error message printing
- Startup banner
- Easy to extend (WiFi logging, LCD, JSON output)

**tsi.ino (Main Sketch)**
- Global manager instances
- C-style callback bridge (notifyCallback)
- setup(): Initialize serial, BLE, connect, configure
- loop(): Poll PIDs, display data, handle disconnections
- Clean orchestration only - no business logic

### Data Flow

1. **Startup**: tsi.ino initializes all managers
2. **Connection**: BLEManager connects to OBD adapter, sends AT commands
3. **Registration**: BLEManager registers notifyCallback with BLE stack
4. **Polling**: loop() sends PID commands via BLEManager
5. **Reception**: BLE callback invokes OBDParser.processChar() for each byte
6. **Parsing**: OBDParser buffers characters, detects delimiters, parses responses
7. **Update**: OBDParser updates CarData fields
8. **Display**: DisplayManager formats and prints CarData

### Why Modular?

**Maintainability**: Each module has single responsibility, easy to locate bugs

**Extensibility**: Adding WiFi is simple - create WifiManager, call from loop()

**Testability**: Can unit test OBDParser with mock data, no hardware needed

**Reusability**: OBDParser can be reused in other ESP32 OBD projects

**Arduino Compatible**: Arduino IDE automatically compiles .cpp/.h files in sketch folder

---
