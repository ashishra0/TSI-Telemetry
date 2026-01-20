#ifndef CONFIG_H
#define CONFIG_H

// Serial Configuration
#define SERIAL_BAUD 115200

// ESP-NOW Channel (must match sender)
#define ESPNOW_CHANNEL 1

// MQTT Configuration
#define MQTT_TOPIC "car/telemetry"
#define MQTT_CLIENT_ID "tsi-receiver"

// HiveMQ Cloud uses port 8883 for TLS
#define MQTT_PORT 8883

// Data timeout - if no data received for this long, mark as stale
#define DATA_TIMEOUT_MS 10000

#endif
