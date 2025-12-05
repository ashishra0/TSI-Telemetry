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
