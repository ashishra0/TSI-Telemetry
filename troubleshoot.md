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
