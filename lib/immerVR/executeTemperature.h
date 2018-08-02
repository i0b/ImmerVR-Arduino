#ifndef EXECUTETEMPERATURE_H
#define EXECUTETEMPERATURE_H

#include "execute.h"
#include "hardware.h"

// norminal temperature usually 25°C
#define T_0 25.0
// resistance at T0
#define R_0 10000.0
// beta, usually 3000-4000
#define B 3950.0
// refference voltage applied to the analog digital converter
#define Vin 3.31

#define PELTIER_UPDATE_RATE_MS  50
#define MIN_PELTIER_TEMPERATURE 20
#define MAX_PELTIER_TEMPERATURE 40


class ExecuteTemperature : public Execute {
public:
  ExecuteTemperature(Hardware *hardware, executeParameter_t *executeParameter);
  void setExecuteByMode(actuationMode_t mode);
  void setIdle();
  void tick();
  String getCurrentValues();

private:
  unsigned long _pulseTimer;
  unsigned long _lastControlUpdateTime;
  bool _pulseActuateState;
  value_t *_pulseValues;
  void (ExecuteTemperature::*_timerCallback)();
  void _idle();
  void _continuous();
  void _pulse();
  void _rain();
  void _measureTemperature();
  void _controlPeltiers();
};

#endif // EXECUTETEMPERATURE_H
