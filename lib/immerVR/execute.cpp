#include <Arduino.h>
#include "execute.h"
#include "hardware.h"

Execute::Execute() {
}

void Execute::setTargetValues(Module* module, value_t *values) {
  for (numberElements_t element; element < module->executeParameter->numberElements; element++) {
    module->executeParameter->targetValues[element] = values[element];
  }
  executeParameter->updated = true;
}

void Execute::setIntervalMs(Module* module, intervalMs_t intervalMs) {
  module->executeParameter->intervalMs = intervalMs;
}

void Execute::setOnDurationMs(Module* module, onDurationMs_t onDurationMs) {
  module->executeParameter->onDurationMs = onDurationMs;
}

void Execute::setRepetitions(Module *module, repetition_t repetitions) {
  module->executeParameter->repetitions = repetitions;
}
