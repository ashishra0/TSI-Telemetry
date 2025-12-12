#ifndef CARDATA_H
#define CARDATA_H

struct CarData {
  int rpm = 0;
  int speed = 0;
  int coolantTemp = 0;
  int intakeTemp = 0;
  int throttle = 0;
  int engineLoad = 0;
  int manifoldPressure = 0;
  int fuelLevel = 0;
  int timingAdvance = 0;

  void reset() {
    rpm = 0;
    speed = 0;
    coolantTemp = 0;
    intakeTemp = 0;
    throttle = 0;
    engineLoad = 0;
    manifoldPressure = 0;
    fuelLevel = 0;
    timingAdvance = 0;
  }
};

#endif
