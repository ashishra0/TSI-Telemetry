#ifndef CONFIG_H
#define CONFIG_H

// BLE Configuration (ELM327 OBD Adapter)
#define OBD_ADDRESS "00:33:cc:4f:36:03"
#define SERVICE_UUID "0000fff0-0000-1000-8000-00805f9b34fb"
#define TX_UUID "0000fff2-0000-1000-8000-00805f9b34fb"
#define RX_UUID "0000fff1-0000-1000-8000-00805f9b34fb"

// ESP-NOW Configuration
// MAC address of the receiver ESP32 (mqtt_receiver)
#define RECEIVER_MAC {0xD4, 0xE9, 0xF4, 0xB3, 0xAC, 0x64}

// Serial Configuration
#define SERIAL_BAUD 115200

// Polling Configuration
#define POLL_DELAY_MS 100
#define DATA_REFRESH_MS 1000
#define RESPONSE_DELAY_MS 400

// ESP-NOW Channel (must match receiver)
#define ESPNOW_CHANNEL 1

#endif
