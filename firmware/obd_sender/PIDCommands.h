#ifndef PIDCOMMANDS_H
#define PIDCOMMANDS_H

// OBD-II PID Definitions - Standard Mode 01
#define PID_RPM "010C"
#define PID_SPEED "010D"
#define PID_COOLANT_TEMP "0105"
#define PID_INTAKE_TEMP "010F"
#define PID_THROTTLE "0111"
#define PID_ENGINE_LOAD "0104"
#define PID_MANIFOLD_PRESSURE "010B"
#define PID_FUEL_LEVEL "012F"
#define PID_TIMING_ADVANCE "010E"

// New PIDs for transmission diagnosis (VW Polo GT TSI)
#define PID_ACTUAL_GEAR "01A4"           // Transmission actual gear
#define PID_BATTERY_VOLTAGE "0142"       // Control module voltage
#define PID_DEMANDED_TORQUE "0161"       // Driver demanded engine torque %
#define PID_ACTUAL_TORQUE "0162"         // Actual engine torque %
#define PID_FUEL_RATE "015E"             // Engine fuel rate

// ELM327 AT Commands
#define CMD_RESET "ATZ"
#define CMD_ECHO_OFF "ATE0"
#define CMD_LINEFEED_OFF "ATL0"
#define CMD_SPACES_OFF "ATS0"

// Command array for polling (in order)
// Critical PIDs first (RPM, Speed) for transmission diagnosis
const char* POLL_COMMANDS[] = {
  PID_RPM,
  PID_SPEED,
  PID_THROTTLE,
  PID_ENGINE_LOAD,
  PID_ACTUAL_GEAR,
  PID_DEMANDED_TORQUE,
  PID_ACTUAL_TORQUE,
  PID_COOLANT_TEMP,
  PID_INTAKE_TEMP,
  PID_MANIFOLD_PRESSURE,
  PID_FUEL_LEVEL,
  PID_TIMING_ADVANCE,
  PID_BATTERY_VOLTAGE,
  PID_FUEL_RATE
};

const int POLL_COMMANDS_COUNT = 14;

#endif
