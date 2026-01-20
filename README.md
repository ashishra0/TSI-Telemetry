# TSI-Telemetry

**Real-time vehicle telemetry system using dual ESP32s, ESP-NOW, HiveMQ Cloud, TimescaleDB, and Grafana.**

A distributed IoT pipeline that streams OBD-II data from your car to a cloud dashboard in real-time. Built for my **Volkswagen Polo GT TSI**, but works with any OBD-II compatible vehicle.

> **Project Status:** Core pipeline working! Real-time data flowing from car to Grafana.

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                              TSI-Telemetry Architecture                          │
└─────────────────────────────────────────────────────────────────────────────────┘

    IN-CAR                                              HOME SERVER / CLOUD
    ══════                                              ═══════════════════

┌─────────────┐      ┌─────────────────┐                ┌─────────────────┐
│   Car ECU   │      │   OBD Sender    │    ESP-NOW     │  MQTT Receiver  │
│             │─────►│    (ESP32)      │───────────────►│    (ESP32)      │
│  OBD-II     │ BLE  │                 │   2.4GHz       │                 │
│  Port       │      │  - BLE to OBD   │   Wireless     │  - ESP-NOW Rx   │
└─────────────┘      │  - Parse PIDs   │                │  - WiFi Client  │
                     │  - ESP-NOW Tx   │                │  - MQTT Publish │
                     └─────────────────┘                └────────┬────────┘
                            │                                    │
                            │                                    │ TLS/MQTT
                            │                                    ▼
                     ┌──────┴──────┐                     ┌─────────────────┐
                     │  ELM327     │                     │  HiveMQ Cloud   │
                     │  BLE OBD    │                     │                 │
                     │  Adapter    │                     │  MQTT Broker    │
                     └─────────────┘                     └────────┬────────┘
                                                                  │
                                                                  │ Subscribe
                                                                  ▼
                                                         ┌─────────────────┐
                                                         │  Python Bridge  │
                                                         │                 │
                                                         │  - MQTT Sub     │
                                                         │  - JSON Parse   │
                                                         │  - SQL Insert   │
                                                         └────────┬────────┘
                                                                  │
                                                                  │ SQL
                                                                  ▼
                                                         ┌─────────────────┐
                                                         │  TimescaleDB    │
                                                         │                 │
                                                         │  Time-series    │
                                                         │  Database       │
                                                         └────────┬────────┘
                                                                  │
                                                                  │ Query
                                                                  ▼
                                                         ┌─────────────────┐
                                                         │    Grafana      │
                                                         │                 │
                                                         │  Real-time      │
                                                         │  Dashboard      │
                                                         └─────────────────┘
```

---

## Data Flow

```
1. Car ECU ──────► OBD-II Port
                      │
                      │ OBD-II Protocol (CAN bus)
                      ▼
2. ELM327 Adapter ──► Translates to serial commands
                      │
                      │ Bluetooth Low Energy (BLE)
                      ▼
3. ESP32 #1 ────────► Reads PIDs, parses responses
   (OBD Sender)       │
                      │ ESP-NOW (peer-to-peer, no WiFi needed)
                      ▼
4. ESP32 #2 ────────► Receives telemetry data
   (MQTT Receiver)    │
                      │ WiFi + TLS
                      ▼
5. HiveMQ Cloud ────► MQTT message broker
                      │
                      │ MQTT Subscribe
                      ▼
6. Python Bridge ───► Transforms and inserts data
                      │
                      │ SQL
                      ▼
7. TimescaleDB ─────► Stores time-series data
                      │
                      │ Query
                      ▼
8. Grafana ─────────► Visualizes real-time dashboard
```

---

## Why Two ESP32s?

Running Bluetooth and WiFi simultaneously on a single ESP32 causes conflicts:
- Memory contention between BLE and WiFi stacks
- RC=-2 MQTT connection failures
- Unstable connections and random disconnects

**Solution:** Separate concerns across two dedicated microcontrollers:

| ESP32 #1 (Sender) | ESP32 #2 (Receiver) |
|-------------------|---------------------|
| BLE only | WiFi only |
| Connects to OBD adapter | Connects to HiveMQ Cloud |
| Parses OBD-II responses | Publishes MQTT messages |
| Sends via ESP-NOW | Receives via ESP-NOW |

ESP-NOW provides reliable, low-latency peer-to-peer communication without requiring WiFi infrastructure.

---

## Telemetry Data

### Supported PIDs

| Metric | PID | Unit | Status |
|--------|-----|------|--------|
| Engine RPM | 010C | rpm | ✅ Working |
| Vehicle Speed | 010D | km/h | ✅ Working |
| Coolant Temperature | 0105 | °C | ✅ Working |
| Intake Air Temperature | 010F | °C | ✅ Working |
| Throttle Position | 0111 | % | ✅ Working |
| Engine Load | 0104 | % | ✅ Working |
| Manifold Pressure | 010B | kPa | ✅ Working |
| Fuel Level | 012F | % | ⚠️ Car-dependent |
| Timing Advance | 010E | ° | ✅ Working |

### Example JSON Payload

```json
{
  "rpm": 2200,
  "speed": 48,
  "coolant_temp": 86,
  "intake_temp": 32,
  "throttle": 18,
  "engine_load": 24,
  "manifold_pressure": 101,
  "fuel_level": 65,
  "timing_advance": 12,
  "sender_timestamp": 123456,
  "receiver_timestamp": 123789
}
```

---

## Quick Start

### Prerequisites

- 2x ESP32 development boards
- ELM327 Bluetooth OBD-II adapter
- Arduino CLI installed
- HiveMQ Cloud account (free tier works)
- TimescaleDB instance (Docker or managed)
- Grafana instance

### 1. Get Receiver MAC Address

```bash
make flash-getmac-receiver
make monitor-receiver
# Note the MAC address shown (e.g., D4:E9:F4:B3:AC:64)
```

### 2. Configure Sender

Edit `firmware/obd_sender/Config.h`:
```c
#define RECEIVER_MAC {0xD4, 0xE9, 0xF4, 0xB3, 0xAC, 0x64}
#define OBD_ADDRESS "00:33:cc:4f:36:03"  // Your ELM327 MAC
```

### 3. Configure Receiver

Copy and edit credentials:
```bash
cp firmware/mqtt_receiver/Credentials.h.example firmware/mqtt_receiver/Credentials.h
```

```c
#define WIFI_SSID "your-wifi"
#define WIFI_PASSWORD "your-password"
#define MQTT_HOST "your-cluster.s1.eu.hivemq.cloud"
#define MQTT_USERNAME "your-username"
#define MQTT_PASSWORD "your-password"
```

### 4. Flash Both Boards

```bash
make flash-sender      # Flash OBD reader
make flash-receiver    # Flash MQTT publisher
```

### 5. Set Up Database

```bash
docker exec -it timescaledb psql -U postgres -d telemetry -f setup_db.sql
```

### 6. Run Python Bridge

```bash
cd bridge
cp .env.example .env
# Edit .env with your credentials
pip install -r requirements.txt
python mqtt_bridge.py
```

### 7. Configure Grafana

Add TimescaleDB as a PostgreSQL data source and query `car_metrics` table.

---

## Makefile Commands

```bash
# Sender (ESP32 #1)
make compile-sender     # Compile only
make flash-sender       # Compile + upload
make monitor-sender     # Serial monitor

# Receiver (ESP32 #2)
make compile-receiver   # Compile only
make flash-receiver     # Compile + upload
make monitor-receiver   # Serial monitor

# Both boards
make compile-all        # Compile both
make flash-all          # Flash both

# Utilities
make ports              # List serial ports
make flash-getmac-receiver  # Get receiver MAC address
make clean              # Remove build artifacts
```

---

## Hardware

| Component | Purpose | Notes |
|-----------|---------|-------|
| ESP32 DevKit (x2) | Microcontrollers | One for BLE, one for WiFi |
| ELM327 BLE Adapter | OBD-II interface | Cheap clones work for testing |
| USB cables | Power + programming | |
| 12V to 5V converter | Car power (optional) | For permanent install |

---

## Troubleshooting

### ESP-NOW "send failed" but data still arrives
- Normal behavior; ACK timing issues don't prevent delivery

### MQTT disconnects (state=-3)
- Check WiFi signal strength
- Verify HiveMQ credentials
- Try `setInsecure()` to rule out TLS certificate issues

### All OBD values are 0
- Car ignition must be ON (not just accessory mode)
- Check BLE connection to ELM327 adapter

### Fuel Level always 0
- Some cars don't support PID 012F
- Check with a phone OBD app (like Torque) to verify

---

## Future Enhancements

- [ ] Re-enable TLS certificate verification
- [ ] ESP8266 supervisor for cluster health monitoring
- [ ] VW-specific PIDs (boost pressure via Mode 0x22)
- [ ] Local SD card buffering for offline drives
- [ ] GSM/LTE version for remote logging
- [ ] Custom PCB for permanent installation
