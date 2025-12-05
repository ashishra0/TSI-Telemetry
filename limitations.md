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
