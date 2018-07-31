#ifndef MODULE_H
#define MODULE_H

#include <ArduinoJson.h>
#include "execute.h"
#include "executeVibrate.h"
#include "executeTemperature.h"
#include "executeEMS.h"
#include "hardware.h"

class Module {
public:
  Module();
  bool begin(i2cAddress_t i2cAddress, uint8_t numElements, moduleType_t type,
             Hardware *hardware);
  void tick();
  void setParameter(parameter_t parameter);
  void setMode(pattern_t mode);
  void setIntensity(intensity_t intensity);
  void setMask(mask_t mask);
  void setIntervalMs(intervalMs_t interval);
  // void setDirect(JsonArray& elementValueArray);
  uint8_t getNumberElements();
  moduleType_t getModuleType();
  executeParameter_t *executeParameter;
  Execute *getExecute();

private:
  Hardware *_hardware;
  Execute *_execute;
  uint32_t _lastTick;
  uint16_t _timerTickInterval;
  moduleType_t _type;
};

#endif // MODULE_H
