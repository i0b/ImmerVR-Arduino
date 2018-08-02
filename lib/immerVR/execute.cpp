#include <Arduino.h>
#include "execute.h"
#include "hardware.h"

Execute::Execute() {
}

void Execute::setTargetValues(value_t *values) {
  for (element_t element = 0; element < executeParameter->numberElements; element++) {
    executeParameter->targetValues[element] = values[element];
  }
  executeParameter->updated = true;
}

void Execute::setIntervalMs(intervalMs_t intervalMs) {
  executeParameter->intervalMs = intervalMs;
}

void Execute::setOnDurationMs(onDurationMs_t onDurationMs) {
  executeParameter->onDurationMs = onDurationMs;
}

void Execute::setRepetitions(repetition_t repetitions) {
  executeParameter->repetitions = repetitions;
}
