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
├── LICENSE                   # MIT License
├── .gitignore               # Build artifacts and credentials
├── CONTRIBUTING.md          # Contribution guidelines
├── README.md                # This file
├── QUICKSTART.md            # Getting started guide
├── architecture.md          # System architecture
├── hardware.md              # Bill of materials
│
├── firmware/
│   ├── tsi/                 # Main modular firmware
│   │   ├── tsi.ino          # Main sketch (65 lines)
│   │   ├── Config.h         # Configuration constants
│   │   ├── CarData.h        # Data structures
│   │   ├── PIDCommands.h    # PID definitions
│   │   ├── BLEManager.h/.cpp      # BLE connection handling
│   │   ├── OBDParser.h/.cpp       # OBD-II response parsing
│   │   └── DisplayManager.h/.cpp  # Serial output formatting
│   └── tsi_legacy/          # Original monolithic version (206 lines)
│
├── backend/
│   ├── docker-compose.yml   # InfluxDB + Grafana
│   └── simulate_car.py      # Mock data generator
│
├── diagrams/
│   └── architecture.png
│
└── dashboards/
    └── telemetry.json       # Grafana dashboard
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

## Development Roadmap

### Milestone 1: "Hello Car" (Day 1)
- [x] ESP32 connects to ELM327 via BLE
- [x] Send `ATZ` reset command
- [x] Receive any response from OBD adapter
- [x] Print raw response to Serial Monitor

### Milestone 2: "One Clean Reading" (Day 2-3)
- [ ] Parse ELM327 response format
- [ ] Read PID `010C` (Engine RPM)
- [ ] Convert hex response to readable integer
- [ ] Print "RPM: 1706" in Serial Monitor

### Milestone 3: "Multiple PIDs" (Week 1)
- [ ] Read RPM, Speed, Coolant Temp
- [ ] Implement basic error handling
- [ ] Handle BLE disconnections gracefully
- [ ] Poll at 1Hz successfully

### Milestone 4: "WiFi Pipeline" (Week 2)
- [ ] Connect ESP32 to home WiFi
- [ ] Send JSON payload to InfluxDB
- [ ] Basic Grafana dashboard shows live RPM
- [ ] System survives 10-minute test drive

### Milestone 5: "Production Ready" (Month 2+)
- [ ] Auto-reconnect BLE/WiFi with exponential backoff
- [ ] Local buffering during connection loss
- [ ] 5+ PIDs with priority-based polling
- [ ] Runs full commute without manual intervention
- [ ] VW-specific PIDs (boost pressure, fuel trims)

---

## Known Limitations

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

## Contributing

This is a learning project—contributions and shared knowledge are welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

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
