#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <Arduino.h>
#include "CarData.h"

class DisplayManager {
public:
  DisplayManager();

  // Print formatted car data
  void printCarData(const CarData& data);

  // Print status messages
  void printStatus(const String& message);
  void printError(const String& error);

  // Print startup banner
  void printBanner();

private:
  // Calculate boost pressure
  int calculateBoost(int manifoldPressure);
};

#endif
