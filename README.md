# TSI-Telemetry
**An open-source, ESP32-powered OBD-II telemetry pipeline for live car diagnostics, analytics, and Grafana dashboards.**

TSI-Telemetry is a lightweight, modular, hardware + firmware project that streams real-time vehicle metrics (RPM, speed, coolant temp, throttle position, fuel trims, boost, etc.) from any OBD-II compatible car to your home server or cloud backend.

Building for my **Volkswagen Polo GT TSI**, but should work with any car that supports standard OBD-II PID commands.

> **Project Status:** Hardware in transit, firmware in development. This is a learning project, expect iteration and experimentation.

---

## Why?
Modern cars produce tons of real-time data, but none of it is available to the driver beyond the dashboard. This project turns your car into an open telemetry source you can log, analyze, visualize, or extend.

This project is ideal for:
- Hardware learners  
- Car enthusiasts  
- Data nerds (Grafana / InfluxDB / Prometheus)  
- DIY automotive hackers  
- People who want to understand engine health better  

**What you'll learn:**
- Embedded systems (ESP32 programming)
- Wireless protocols (BLE, WiFi)
- Automotive electronics (OBD-II, CAN bus)
- Time-series databases (InfluxDB)
- Data visualization (Grafana)
- Real-time error handling and reconnection logic

---

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

## Hardware Bill of Materials

### Phase 1: Proof of Concept (~$30)
| Component | Estimated Cost | Purpose | Notes |
|----------|---------------|---------|-------|
| **ESP32 Dev Board** | $5-8 | BLE + WiFi controller | Any ESP32 works |
| **ELM327 BLE Adapter** | $8-12 | OBD-II interface | Budget for unreliability |
| **USB Car Charger (5V)** | $5 | Power source | Use existing one |
| **Micro USB Cable** | $3 | Power + programming | |
| **Breadboard + Jumpers** | $5 | Prototyping | Optional |

**Total: ~$25-35**

### Phase 2: Production Upgrade (~$50 additional)
| Component | Cost | Why Upgrade? |
|----------|------|--------------|
| **Veepeak/Viecar BLE Adapter** | $25-30 | 10x more stable connection |
| **Automotive Buck Converter** | $8-12 | Voltage protection, cleaner power |
| **ABS Enclosure** | $5-8 | Professional mounting |
| **External WiFi Antenna** | $3-5 | Better signal in metal car body |

**Upgrade only after validating the concept with cheap hardware.**

---

## Development Roadmap

### Milestone 1: "Hello Car" (Day 1)
- [ ] ESP32 connects to ELM327 via BLE
- [ ] Send `ATZ` reset command
- [ ] Receive any response from OBD adapter
- [ ] Print raw response to Serial Monitor

**Success = seeing garbage characters means communication works**

### Milestone 2: "One Clean Reading" (Day 2-3)
- [ ] Parse ELM327 response format
- [ ] Read PID `010C` (Engine RPM)
- [ ] Convert hex response to readable integer
- [ ] Print "RPM: 1706" in Serial Monitor

**Success = one accurate RPM reading**

### Milestone 3: "Multiple PIDs" (Week 1)
- [ ] Read RPM, Speed, Coolant Temp
- [ ] Implement basic error handling
- [ ] Handle BLE disconnections gracefully
- [ ] Poll at 1Hz successfully

**Success = 3 PIDs for 5 minutes without crash**

### Milestone 4: "WiFi Pipeline" (Week 2)
- [ ] Connect ESP32 to home WiFi
- [ ] Send JSON payload to InfluxDB
- [ ] Basic Grafana dashboard shows live RPM
- [ ] System survives 10-minute test drive

**Success = seeing car data on Grafana in real-time**

### Milestone 5: "Production Ready" (Month 2+)
- [ ] Auto-reconnect BLE/WiFi with exponential backoff
- [ ] Local buffering during connection loss
- [ ] 5+ PIDs with priority-based polling
- [ ] Runs full commute without manual intervention
- [ ] VW-specific PIDs (boost pressure, fuel trims)

**Success = reliable daily-driver quality**

---

## Known Limitations & Gotchas

### Hardware Reality Check
**ELM327 BLE Adapters:**
- Cheap clones ($5-10) disconnect randomly every 2-10 minutes
- Firmware quality varies wildly between units
- Start cheap for learning, upgrade to Veepeak/Viecar later
- WiFi OBD adapters more stable (but ESP32 becomes WiFi client)

**ESP32 Quirks:**
- Metal car body acts as Faraday cage (WiFi range issues)
- Alternator + ignition coils create electrical noise
- USB car chargers cut power when ignition off
- Cold starts cause voltage spikes (add capacitor to VIN)

### OBD-II Protocol Limitations
**Polling Speed:**
- Each PID request takes 50-200ms
- Reading 5 PIDs = 250ms-1s latency minimum
- CAN bus congestion during high ECU load
- Start with 1Hz sampling, optimize later

**PID Availability (VW Polo GT TSI specific):**
- Standard PIDs (Mode 0x01): RPM, Speed, Coolant, Throttle
- Boost Pressure: NOT a standard PID, requires Mode 0x22 (manufacturer-specific)
- Fuel Trims: May be blocked during real-time driving on some ECUs
- Unknown: Your car's ECU must be tested to discover supported PIDs

**Safety & Legal:**
- OBD-II port is always powered (drains battery if left plugged in)
- Unplugging during engine operation is safe
- Don't flash firmware while driving
- Check local laws on OBD-II logging (usually legal for personal use)

### Data Volume
At 1Hz sampling with 6 PIDs:
- ~100 bytes per reading
- ~360KB per hour of driving
- ~8.6MB per day (24h continuous)

**Recommendation:** InfluxDB retention policy (7 days raw, downsampled for history)

---

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
// hello_car.ino - Simplest possible test
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

## Current Features

### Working
- Basic ESP32 BLE connection framework
- Mock data backend (InfluxDB + Grafana)
- Architecture documentation

### In Development (Post-Hardware Arrival)
- ELM327 command parser
- PID reader with error handling
- WiFi telemetry sender
- Reconnection logic
- Multi-PID polling

### Planned (After Validation)
- VW-specific PID support (boost, advanced fuel trims)
- Priority-based polling (fast PIDs vs slow PIDs)
- Local SD card buffering
- NTP time sync
- DTC (error code) reader
- GSM/LTE version for cloud logging
- PCB design for permanent install
- Web UI for configuration

---

## Example JSON Payload

```json
{
  "rpm": 2200,
  "speed": 48,
  "coolant": 86,
  "iat": 32,
  "throttle": 18,
  "timestamp": 1712170184
}
```

---

## Project Structure

```
TSI-Telemetry/
│
├── firmware/
│   ├── hello_car/          # Day 1 BLE test
│   ├── pid_reader/         # PID parsing logic
│   └── main/               # Full telemetry system
│
├── backend/
│   ├── docker-compose.yml  # InfluxDB + Grafana
│   └── simulate_car.py     # Mock data generator
│
├── diagrams/
│   └── architecture.png
│
├── dashboards/
│   └── telemetry.json      # Grafana dashboard
│
└── docs/
    ├── vw_pids.md          # VW-specific PID codes
    └── troubleshooting.md  # Common issues
```

---

## Decision Points

### When to Invest in Quality Hardware

**Invest when you hit these milestones:**
- Successfully read 3+ PIDs consistently
- System runs for 30-minute drive
- You check the dashboard 5+ times
- Cheap adapter fails predictably

**Don't invest yet if:**
- Can't get BLE working after 2 weeks of troubleshooting
- Your car doesn't support enough interesting PIDs
- Project loses novelty after initial excitement
- Realize phone apps like Torque do enough

**The cheap hardware teaches you if this project is worth pursuing.**

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

## Contributing

This is a learning project—contributions and shared knowledge are welcome!

**Ways to contribute:**
- Share your car's supported PID list
- Document manufacturer-specific PIDs
- Report adapter compatibility (which brands work/fail)
- Improve error handling code
- Add new backend integrations (Prometheus, Home Assistant)

**Future contribution ideas:**
- Custom PCB design
- GSM/LTE module support
- Android/iOS companion app
- 3D-printed enclosure designs

---

## Resources

**OBD-II Documentation:**
- [Wikipedia: OBD-II PIDs](https://en.wikipedia.org/wiki/OBD-II_PIDs)
- [ELM327 Datasheet](https://www.elmelectronics.com/wp-content/uploads/2017/01/ELM327DS.pdf)

**Forums & Communities:**
- [ESP32 Subreddit](https://reddit.com/r/esp32)
- [VWVortex (VW-specific)](https://forums.vwvortex.com/)
- [ELMduino GitHub](https://github.com/PowerBroker2/ELMduino)

**Learning Resources:**
- [How OBD-II Works (Video)](https://www.youtube.com/watch?v=1NiJ-0rWHis)
- [InfluxDB Quickstart](https://docs.influxdata.com/influxdb/v1.8/introduction/get-started/)
- [Grafana Tutorial](https://grafana.com/tutorials/grafana-fundamentals/)

---

## License

MIT License — completely open-source. Use, modify, share freely.

---

## Acknowledgements

- ESP32 community for excellent documentation
- Open-source OBD-II tooling authors (ELMduino, pyOBD)
- Car hacking community for reverse-engineering protocols
- Everyone tinkering with automotive data

---
