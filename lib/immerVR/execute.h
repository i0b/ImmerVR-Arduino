#ifndef EXECUTE_H
#define EXECUTE_H

#include <stdint.h>
#include "hardware.h"

typedef uint16_t intensity_t;
typedef uint8_t i2cAddress_t;
typedef uint16_t intervalMs_t;
typedef uint16_t onDurationMs_t;
typedef uint16_t value_t;
typedef uint8_t element_t;
typedef uint16_t repetition_t;

typedef struct {
  i2cAddress_t address;
  element_t numberElements;
  mode_t mode;
  value_t *currentValues;
  value_t *targetValues;
  intervalMs_t intervalMs;
  onDurationMs_t onDurationMs;
  repetition_t repetitions;
  bool updated;
} executeParameter_t;

class Execute {
public:
  executeParameter_t *executeParameter;

  Execute();
  void setTargetValues(value_t *values);
  void setIntervalMs(intervalMs_t intervalMs);
  void setOnDurationMs(onDurationMs_t onDurationMs);
  void setRepetitions(repetition_t repetitions);
  virtual void setExecuteByMode(mode_t mode) = 0;
  virtual void setIdle() = 0;
  virtual void tick() = 0;
  virtual String getCurrentValues() = 0;

protected:
  Hardware *_hardware;
};

#endif // EXECUTE_H
