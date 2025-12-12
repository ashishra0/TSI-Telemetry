#include "DisplayManager.h"

DisplayManager::DisplayManager() {}

void DisplayManager::printCarData(const CarData& data) {
  Serial.println("\n========================================");

  Serial.printf("RPM:                 %4d rpm\n", data.rpm);
  Serial.printf("Speed:               %4d km/h\n", data.speed);
  Serial.printf("Coolant Temp:        %4d °C\n", data.coolantTemp);
  Serial.printf("Intake Air Temp:     %4d °C\n", data.intakeTemp);
  Serial.printf("Throttle Position:   %4d %%\n", data.throttle);
  Serial.printf("Engine Load:         %4d %%\n", data.engineLoad);
  Serial.printf("Manifold Pressure:   %4d kPa", data.manifoldPressure);

  int boost = calculateBoost(data.manifoldPressure);
  if (boost > 0) {
    Serial.printf(" (+%d kPa BOOST!)", boost);
  }
  Serial.println();

  Serial.printf("Fuel Level:          %4d %%\n", data.fuelLevel);
  Serial.printf("Timing Advance:      %4d degrees\n", data.timingAdvance);

  Serial.println("========================================\n");
}

void DisplayManager::printStatus(const String& message) {
  Serial.println(message);
}

void DisplayManager::printError(const String& error) {
  Serial.print("ERROR: ");
  Serial.println(error);
}

void DisplayManager::printBanner() {
  Serial.println("\n\nStarting TSI Telemetry...");
}

int DisplayManager::calculateBoost(int manifoldPressure) {
  return manifoldPressure - 100;
}
