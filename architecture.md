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
