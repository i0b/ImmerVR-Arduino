#ifndef EXECUTE_H
#define EXECUTE_H

#include <stdint.h>
#include "hardware.h"

typedef uint16_t parameter_t;
typedef uint16_t intensity_t;
typedef uint16_t mask_t;
typedef uint8_t i2cAddress_t;
typedef uint16_t intervalMs_t;

typedef struct {
  i2cAddress_t address;
  uint8_t numberElements;
  uint8_t *elementValues;
  parameter_t parameter;
  intensity_t intensity;
  mask_t mask;
  pattern_t pattern;
  intervalMs_t intervalMs;
  bool updated;
} executeParameter_t;

class Execute {
public:
  Execute();
  virtual void setExecuteByPattern(pattern_t pattern);
  virtual void setIdle(Hardware *hardware,
                       executeParameter_t *executeParameter);
  virtual void tick(Hardware *hardware, executeParameter_t *executeParameter);
  virtual String getMeasurements(Hardware *hardware, executeParameter_t *executeParameter);

private:
};

#endif // EXECUTE_H
