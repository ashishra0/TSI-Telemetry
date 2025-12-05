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
