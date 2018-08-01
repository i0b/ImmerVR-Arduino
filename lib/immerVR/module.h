#ifndef MODULE_H
#define MODULE_H

#include "hardware.h"
#include "execute.h"
#include "executeVibrate.h"
#include "executeTemperature.h"
#include "executeEMS.h"

class Module {
public:
  Module();
  bool begin(i2cAddress_t i2cAddress, uint8_t numElements, moduleType_t type,
             Hardware *hardware);
  void tick();
  void setMode(mode_t mode);
  void setIntervalMs(intervalMs_t interval);
  void setOnDurationMs(onDurationMs_t onDurationMs);
  uint8_t getNumberElements();
  moduleType_t type;
  Hardware *hardware;
  Execute* execute;

private:
  uint32_t _lastTick;
  uint16_t _timerTickInterval;
};

#endif // MODULE_H
