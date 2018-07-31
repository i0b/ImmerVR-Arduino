#include <Arduino.h>
#include "execute.h"
#include "module.h"

Module::Module() {}

bool Module::begin(i2cAddress_t i2cAddress, uint8_t numElements,
                   moduleType_t type, Hardware *hardware) {
  _type = type;
  _hardware = hardware;
  _hardware->resetPwm(i2cAddress);
  if (_hardware->checkActuatorActive(i2cAddress) == -1) {
    Serial.print("no module using address 0x");
    Serial.print(i2cAddress, HEX);
    Serial.println(" was found.");

    return false;
  }

  _hardware->setPwmFrequency(i2cAddress, 400);
  for (uint8_t channel = 0; channel < numElements; channel++) {
    _hardware->setPercent(i2cAddress, channel, _OFF);
  }

  executeParameter = new executeParameter_t;
  executeParameter->address = i2cAddress;
  executeParameter->numberElements = numElements;
  executeParameter->elementValues = new uint8_t[numElements];

  for (uint8_t element = 0; element < numElements; element++) {
    executeParameter->elementValues[element] = 0;
  }

  executeParameter->updated = false;
  _timerTickInterval = 50;

  if (type == VIBRATE) {
    _execute = new ExecuteVibrate(_hardware, executeParameter);
  } else if (type == TEMPERATURE) {
    _execute = new ExecuteTemperature(_hardware, executeParameter);
  } else if (type == EMS) {
    _execute = new ExecuteEMS(_hardware, executeParameter);
  }

  return true;
}

void Module::tick() {
  if ((millis() - _lastTick) >= _timerTickInterval) {
    _lastTick = millis();
    _execute->tick(_hardware, executeParameter);
  }
}

void Module::setParameter(parameter_t parameter) {
  if (executeParameter->parameter != parameter) {
    executeParameter->updated = true;
    executeParameter->parameter = parameter;
  }
}

void Module::setMode(pattern_t pattern) {
  // set to idle mode before continue -- this turns all actuators of first
  if (executeParameter->pattern != pattern) {
    _execute->setIdle(_hardware, executeParameter);
    executeParameter->updated = true;
    executeParameter->pattern = pattern;
    _execute->setExecuteByPattern(pattern);
  }
}

void Module::setIntensity(intensity_t intensity) {
  if (executeParameter->intensity != intensity) {
    executeParameter->updated = true;
    executeParameter->intensity = intensity;
  }
}

void Module::setMask(mask_t mask) {
  if (executeParameter->mask != mask) {
    executeParameter->updated = true;
    executeParameter->mask = mask;
  }
}

void Module::setIntervalMs(intervalMs_t interval) {
  if (executeParameter->intervalMs != interval) {
    executeParameter->updated = true;
    executeParameter->intervalMs = interval;
  }
}

moduleType_t Module::getModuleType() { return _type; }
/*
   void Module::setDirect(JsonArray& elementValueArray) {
        for (uint8_t element = 0; element < elementValueArray.size(); element++)
   { uint8_t newValue = elementValueArray[element].as<int>(); if (newValue >= 0
   && newValue <= 100) { _executeParameter->elementValues[element] = newValue;
                        _executeParameter->updated = true;
                }
        }
   }
 */

 Execute* Module::getExecute() {
   return _execute;
 }
