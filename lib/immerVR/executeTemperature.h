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

#define PELTIER_UPDATE_RATE_MS 200
#define MIN_PELTIER_TEMPERATURE 10
#define MAX_PELTIER_TEMPERATURE 40


class ExecuteTemperature : public Execute {
public:
  ExecuteTemperature(Hardware *hardware, executeParameter_t *executeParameter);
  void setExecuteByPattern(pattern_t pattern);
  void setIdle(Hardware *hardware, executeParameter_t *executeParameter);
  void tick(Hardware *hardware, executeParameter_t *executeParameter);
  String getMeasurements(Hardware *hardware, executeParameter_t *executeParameter);

private:
  unsigned long _lastTick;
  unsigned long _lastActuated;
  unsigned long _lastControlUpdateTime;
  float *_setPeltierTemperature;
  void (ExecuteTemperature::*_timerCallback)(
      Hardware *hardware, executeParameter_t *executeParameter);
  void _idle(Hardware *hardware, executeParameter_t *executeParameter);
  void _constant(Hardware *hardware, executeParameter_t *executeParameter);
  void _direct(Hardware *hardware, executeParameter_t *executeParameter);
  void _rain(Hardware *hardware, executeParameter_t *executeParameter);
  float _getTemperature(Hardware *hardware, uint8_t element);
  void _controlPeltiers(Hardware *hardware, executeParameter_t *executeParameter);
};

#endif // EXECUTETEMPERATURE_H
