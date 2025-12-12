## Quick Start Guide

### Before Hardware Arrives

#### 1. Set up InfluxDB (5 minutes)
```bash
# Using Docker (easiest)
docker run -d -p 8086:8086 --name influxdb influxdb:1.8

# Create database
docker exec influxdb influx -execute 'CREATE DATABASE telemetry'
```

#### 2. Create mock data generator (learning the backend)
```python
# simulate_car.py
import requests
import random
import time

while True:
    rpm = random.randint(800, 3000)
    speed = random.randint(0, 120)
    coolant = random.randint(70, 95)
    
    data = f'car rpm={rpm},speed={speed},coolant={coolant}'
    requests.post('http://localhost:8086/write?db=telemetry', data=data)

    print(f"Sent: RPM={rpm}, Speed={speed}, Coolant={coolant}")
    time.sleep(1)
```

#### 3. Set up Grafana
```bash
docker run -d -p 3000:3000 --name grafana grafana/grafana
# Open http://localhost:3000 (admin/admin)
# Add InfluxDB datasource: http://host.docker.internal:8086
```

**Why do this first?** When hardware arrives, you'll know if problems are in firmware or backend.

---

### Day 1: Hardware Setup

#### 1. Install ESP32 support in Arduino IDE
```
File → Preferences → Additional Board Manager URLs:
https://dl.espressif.com/dl/package_esp32_index.json

Tools → Board → Boards Manager → Search "ESP32" → Install
```

#### 2. Install required libraries
```
Sketch → Include Library → Manage Libraries:
- ELMduino (by PowerBroker2)
- WiFi (built-in)
- HTTPClient (built-in)
```

#### 3. Understanding the Firmware Structure

The TSI telemetry firmware is organized into multiple files for better maintainability:

```
firmware/tsi/
├── tsi.ino              # Main sketch (open this in Arduino IDE)
├── Config.h             # Configuration constants
├── CarData.h            # Data structures
├── PIDCommands.h        # PID definitions
├── BLEManager.h/.cpp    # BLE connection handling
├── OBDParser.h/.cpp     # OBD-II parsing
└── DisplayManager.h/.cpp # Serial output
```

Arduino IDE automatically compiles all .cpp/.h files in the sketch folder, so you just need to open `tsi.ino` and click Upload.

#### 4. Configure OBD Adapter Address

Before uploading, you may need to update the OBD adapter address in `Config.h`:

```cpp
// firmware/tsi/Config.h
#define OBD_ADDRESS "00:33:cc:4f:36:03"  // Replace with your adapter's MAC address
```

To find your adapter's MAC address, use a Bluetooth scanner app on your phone or run a BLE scan sketch on the ESP32.

#### 5. Upload and Test

1. Open `firmware/tsi/tsi.ino` in Arduino IDE
2. Select your ESP32 board (Tools → Board → ESP32 Dev Module)
3. Select the correct port (Tools → Port)
4. Click Upload
5. Open Serial Monitor (115200 baud)
6. You should see "Starting TSI Telemetry..." and connection messages

#### 6. Test in car
1. Plug OBD adapter into car's OBD-II port (usually under steering wheel)
2. Turn ignition to ON (don't start engine yet)
3. Plug ESP32 into USB car charger
4. Open Serial Monitor (115200 baud)
5. Look for: `ELM327 v1.5` or similar response

**Success criteria:** Any response from adapter means BLE works!

---

## Troubleshooting

### BLE Won't Connect
```cpp
// Try scanning for devices first
BT.begin("ESP32", true);
BTScanResults* results = BT.getScanResults();
for (int i = 0; i < results->getCount(); i++) {
    Serial.println(results->getDevice(i)->getName().c_str());
}
```

### OBD Adapter Not Responding
- Try `ATZ` (reset), `ATSP0` (auto protocol), `0100` (supported PIDs)
- Some adapters need `\r` line endings: `BT.print("ATZ\r")`
- Wait 2 seconds after reset before sending commands

### WiFi Disconnects
- Car metal body blocks signal
- Move WiFi router closer or add external antenna to ESP32
- Implement reconnection logic (don't block main loop)

### Wrong PID Values
- Double-check parsing: RPM = `(A*256 + B) / 4`
- ELM327 returns: `41 0C 1A 2B` → RPM = `(0x1A*256 + 0x2B) / 4 = 1706`
- Some PIDs are single-byte, some are two-byte

---
