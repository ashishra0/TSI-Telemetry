#ifndef CARDATA_H
#define CARDATA_H

#include <stdint.h>

// Packed structure for ESP-NOW transmission
// Must be identical in sender and receiver
// Size: 14 x int16_t (28 bytes) + 1 x uint32_t (4 bytes) = 32 bytes
typedef struct __attribute__((packed)) {
  // Core metrics
  int16_t rpm;
  int16_t speed;
  int16_t coolantTemp;
  int16_t intakeTemp;
  int16_t throttle;
  int16_t engineLoad;
  int16_t manifoldPressure;
  int16_t fuelLevel;
  int16_t timingAdvance;

  // New metrics for transmission diagnosis
  int16_t actualGear;         // Transmission actual gear (0=N, 1-7)
  int16_t batteryVoltage;     // Control module voltage (mV / 100, e.g., 125 = 12.5V)
  int16_t demandedTorque;     // Driver demanded torque % (-125 to +125)
  int16_t actualTorque;       // Actual engine torque % (-125 to +125)
  int16_t fuelRate;           // Fuel rate (L/h * 10, e.g., 55 = 5.5 L/h)

  uint32_t timestamp;
} CarData;

inline void resetCarData(CarData* data) {
  data->rpm = 0;
  data->speed = 0;
  data->coolantTemp = 0;
  data->intakeTemp = 0;
  data->throttle = 0;
  data->engineLoad = 0;
  data->manifoldPressure = 0;
  data->fuelLevel = 0;
  data->timingAdvance = 0;
  data->actualGear = 0;
  data->batteryVoltage = 0;
  data->demandedTorque = 0;
  data->actualTorque = 0;
  data->fuelRate = 0;
  data->timestamp = 0;
}

#endif
