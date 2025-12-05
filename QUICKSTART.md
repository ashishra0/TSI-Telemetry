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

#### 3. Flash "Hello Car" test code
```cpp
// hello_car.ino
#include <BluetoothSerial.h>

BluetoothSerial BT;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE...");
    
    BT.begin("ESP32", true); // Master mode
    
    // Replace "OBDII" with your adapter's name
    // Common names: "OBDII", "OBD2", "ELM327", "V-LINK"
    bool connected = BT.connect("OBDII");
    
    if (connected) {
        Serial.println("Connected to OBD adapter!");
    } else {
        Serial.println("Failed to connect. Check adapter name.");
    }
}

void loop() {
    if (BT.connected()) {
        Serial.println("Sending ATZ (reset)...");
        BT.println("ATZ");
        delay(2000);
        
        // Read any response
        while (BT.available()) {
            Serial.write(BT.read());
        }
        Serial.println();
    }
    
    delay(5000);
}
```

#### 4. Test in car
1. Plug OBD adapter into car's OBD-II port (usually under steering wheel)
2. Turn ignition to ON (don't start engine yet)
3. Plug ESP32 into USB car charger
4. Open Serial Monitor (115200 baud)
5. Look for: `ELM327 v1.5` or similar response

**Success criteria:** Any response from adapter means BLE works!

---
