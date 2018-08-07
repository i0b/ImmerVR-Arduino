#include <Arduino.h>
#include "execute.h"
#include "module.h"

Module::Module() {}

bool Module::begin(i2cAddress_t i2cAddress, uint8_t numElements,
                   moduleType_t type, Hardware *hardware) {
  this->type = type;
  this->hardware = hardware;
  this->hardware->resetPwm(i2cAddress);

  if (this->hardware->checkActuatorActive(i2cAddress) == -1) {
    Serial.print("no module using address 0x");
    Serial.print(i2cAddress, HEX);
    Serial.println(" was found.");

    return false;
  }

  this->hardware->setPwmFrequency(i2cAddress, 400);
  for (uint8_t channel = 0; channel < numElements; channel++) {
    this->hardware->setPercent(i2cAddress, channel, _OFF);
  }

  executeParameter_t *executeParameter = new executeParameter_t;
  executeParameter->address = i2cAddress;
  executeParameter->numberElements = numElements;
  executeParameter->mode = IDLE;
  executeParameter->currentValues = new value_t[numElements];
  executeParameter->targetValues = new value_t[numElements];
  executeParameter->updated = false;

  for (element_t element = 0; element < numElements; element++) {
    executeParameter->currentValues[element] = 0;
  }

  for (element_t element = 0; element < numElements; element++) {
    executeParameter->targetValues[element] = 0;
  }

  if (type == VIBRATE) {
    execute = new ExecuteVibrate(hardware, executeParameter);
  } else if (type == TEMPERATURE) {
    execute = new ExecuteTemperature(hardware, executeParameter);
  } else if (type == EMS) {
    execute = new ExecuteEMS(hardware, executeParameter);
  }

  return true;
}

void Module::tick() {
  if ((millis() - _lastTick) >= EXECUTE_REFRESH_RATE) {
    _lastTick = millis();
    execute->tick();
  }
}

void Module::setRepetitions(repetition_t repetitions) {
    if (execute->executeParameter->repetitions != repetitions) {
      execute->executeParameter->updated = true;
      execute->executeParameter->repetitions = repetitions;
    }
}

void Module::setIntervalMs(intervalMs_t intervalMs) {
  if (execute->executeParameter->intervalMs != intervalMs) {
    execute->executeParameter->updated = true;
    execute->executeParameter->intervalMs = intervalMs;
  }
}

void Module::setOnDurationMs(onDurationMs_t onDurationMs) {
  if (execute->executeParameter->onDurationMs != onDurationMs) {
    execute->executeParameter->updated = true;
    execute->executeParameter->onDurationMs = onDurationMs;
  }
}

void Module::setMode(actuationMode_t mode) {
  // set to idle mode before continue -- this turns all actuators of first
  if (execute->executeParameter->mode != mode) {
    if (type != TEMPERATURE) {
      execute->setIdle();
    }
    execute->executeParameter->updated = true;
    execute->executeParameter->mode = mode;
    execute->setExecuteByMode(mode);
  }
}
