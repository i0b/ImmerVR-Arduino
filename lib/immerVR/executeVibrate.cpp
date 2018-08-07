#include "executeVibrate.h"
#include <Arduino.h>

ExecuteVibrate::ExecuteVibrate(Hardware *hardware, executeParameter_t *executeParameter) {
  this->executeParameter = executeParameter;
  this->_hardware = hardware;
  _pulseActuateState = false;

  this->executeParameter->mode = CONTINUOUS;
  _timerCallback = &ExecuteVibrate::_continuous;
}

void ExecuteVibrate::setExecuteByMode(actuationMode_t mode) {
  // switch (mode)
  if (mode == IDLE) {
    _timerCallback = &ExecuteVibrate::_idle;
  } else if (mode == CONTINUOUS) {
    _timerCallback = &ExecuteVibrate::_continuous;
  } else if (mode == PULSE) {
    _pulseTimer = millis();
    _pulseActuateState = true;
    _timerCallback = &ExecuteVibrate::_pulse;
  } else if (mode == HEARTBEAT) {
    _state = HEARTBEAT_IDLE;
    _pulseTimer = millis();
    _timerCallback = &ExecuteVibrate::_heartbeat;
  } else if (mode == DASH) {
    _state = 0;
    _pulseTimer = millis();
    _timerCallback = &ExecuteVibrate::_dash;
  } else if (mode == RAIN) {
    _pulseTimer = millis();
    _pulseActuateState = true;
    _timerCallback = &ExecuteVibrate::_rain;
  } else {
    Serial.println("{ \"event\": \"set-mode\", \"status\": \"error\", \"message\": \"mode not implemented\" }");
    _timerCallback = &ExecuteVibrate::_idle;
  }
}

void ExecuteVibrate::setIdle() {
  executeParameter->updated = true;
  executeParameter->mode = IDLE;
  _timerCallback = &ExecuteVibrate::_idle;
}

String ExecuteVibrate::getCurrentValues() {
  return "";
}

void ExecuteVibrate::tick() {
  (this->*_timerCallback)();
}

void ExecuteVibrate::_idle() {
  if (executeParameter->updated) {
    for (uint8_t element = 0; element < executeParameter->numberElements;
         element++) {
      _hardware->setPercent(executeParameter->address, element, _OFF);
      executeParameter->currentValues[element] = 0;
      executeParameter->targetValues[element] = 0;
    }
    executeParameter->updated = false;
  }
}

void ExecuteVibrate::_continuous() {
  if (executeParameter->updated) {
    for (element_t element = 0; element < executeParameter->numberElements; element++) {
      _hardware->setPercent(executeParameter->address, element, executeParameter->targetValues[element]);
      executeParameter->currentValues[element] = executeParameter->targetValues[element];
    }
    executeParameter->updated = false;
  }
}

void ExecuteVibrate::_pulse() {
  if (executeParameter->repetitions > 0) {
    if (millis() >= _pulseTimer) {
      if (_pulseActuateState == true) {
        for (element_t element = 0; element < executeParameter->numberElements; element++) {
          _hardware->setPercent(executeParameter->address, element, executeParameter->targetValues[element]);
          executeParameter->currentValues[element] = executeParameter->targetValues[element];
        }
        _pulseTimer = millis() + executeParameter->onDurationMs;
        _pulseActuateState = false;
      }
      else {
        for (element_t element = 0; element < executeParameter->numberElements; element++) {
          _hardware->setPercent(executeParameter->address, element, _OFF);
          executeParameter->currentValues[element] = _OFF;
        }
        _pulseTimer = millis() + executeParameter->intervalMs;
        executeParameter->repetitions--;
        _pulseActuateState = true;
      }
    }
  }

  else {
    executeParameter->updated = false;
    setIdle();
  }
}


void ExecuteVibrate::_rain() {
  // targetValues: intensity of actuations
  // onDurationMs:  on time in ms for impact

  // simple algorithm: only change state every intervalMs milliseconds
  //                    - turn off all actuators
  //                    - turn on as many as needed

  if (millis() >= _pulseTimer + executeParameter->onDurationMs) {
    // time for new raindrops
    for (element_t element = 0; element < executeParameter->numberElements; element++) {
      executeParameter->currentValues[element] = _OFF;
    }

    unsigned long timeSinceLastDrops = millis() - _pulseTimer;
    element_t numberNewDrops = timeSinceLastDrops/executeParameter->intervalMs;

    if (numberNewDrops > 0) {
      for (element_t drops = 0; drops < numberNewDrops; drops++) {
        element_t onElement = random(0, executeParameter->numberElements);
        executeParameter->currentValues[onElement] = executeParameter->targetValues[onElement];
      }
      _pulseTimer = millis();
    }

    for (element_t element = 0; element < executeParameter->numberElements; element++) {
      _hardware->setPercent(executeParameter->address, element, executeParameter->currentValues[element]);
    }
  }
  executeParameter->updated = false;
}

void ExecuteVibrate::_heartbeat() {
  unsigned long tickDiff = millis() - _pulseTimer;

  if (tickDiff > ((EXECUTE_REFRESH_RATE + executeParameter->onDurationMs) +
                        HEARTBEAT_INTERVAL_BETWEEN +
                        executeParameter->onDurationMs) +
                        executeParameter->intervalMs) {
    _pulseTimer = millis();
    _state = HEARTBEAT_IDLE;

  } else if (tickDiff <= (EXECUTE_REFRESH_RATE + executeParameter->onDurationMs)) {
  // first beat
    _state = HEARTBEAT_START;
  } else if (tickDiff <= ((EXECUTE_REFRESH_RATE + executeParameter->onDurationMs) +
                        HEARTBEAT_INTERVAL_BETWEEN)) {
    // first beat break
    _state = HEARTBEAT_END;
  }
  else if (tickDiff <= ((EXECUTE_REFRESH_RATE + executeParameter->onDurationMs) +
                        HEARTBEAT_INTERVAL_BETWEEN +
                        executeParameter->onDurationMs)) {
                          _state = HEARTBEAT_START;
    // second beat
    _state = HEARTBEAT_START;
  }
  else if (tickDiff <= ((EXECUTE_REFRESH_RATE + executeParameter->onDurationMs) +
                        HEARTBEAT_INTERVAL_BETWEEN +
                        executeParameter->onDurationMs) +
                        executeParameter->intervalMs) {
    // second beat break
    _state = HEARTBEAT_END;
  }

  switch (_state) {
    case HEARTBEAT_START:
      #ifdef DEBUG
          Serial.print("heartbeat: start [");
          Serial.print(tickDiff);
          Serial.println("ms]");
      #endif // DEBUG
      for (element_t element = 0; element < executeParameter->numberElements; element++) {
        executeParameter->currentValues[element] = executeParameter->targetValues[element];
        _hardware->setPercent(executeParameter->address, element, executeParameter->targetValues[element]);
      }
      _state = HEARTBEAT_IDLE;
      break;
    case HEARTBEAT_END:
      #ifdef DEBUG
          Serial.print("heartbeat: end [");
          Serial.print(tickDiff);
          Serial.println("ms]");
      #endif // DEBUG
      for (element_t element = 0; element < executeParameter->numberElements; element++) {
        executeParameter->currentValues[element] = _OFF;
        _hardware->setPercent(executeParameter->address, element, _OFF);
      }
      _state = HEARTBEAT_IDLE;
      break;
    default:
      break;
  }
}

void ExecuteVibrate::_dash() {
    if (executeParameter->repetitions > 0) {
    if (millis() > _pulseTimer + executeParameter->onDurationMs) {
      for (element_t element = 0; element < executeParameter->numberElements; element++) {
        executeParameter->currentValues[element] = _OFF;
      }

      switch (_state) {
        case 0:
          executeParameter->currentValues[0] = executeParameter->targetValues[0];
          executeParameter->currentValues[7] = executeParameter->targetValues[7];
          executeParameter->currentValues[8] = executeParameter->targetValues[8];
          executeParameter->currentValues[15] = executeParameter->targetValues[15];
          break;
        case 1:
          executeParameter->currentValues[1] = executeParameter->targetValues[1];
          executeParameter->currentValues[6] = executeParameter->targetValues[6];
          executeParameter->currentValues[9] = executeParameter->targetValues[9];
          executeParameter->currentValues[14] = executeParameter->targetValues[14];
          break;
        case 2:
          executeParameter->currentValues[2] = executeParameter->targetValues[2];
          executeParameter->currentValues[5] = executeParameter->targetValues[5];
          executeParameter->currentValues[10] = executeParameter->targetValues[10];
          executeParameter->currentValues[13] = executeParameter->targetValues[13];
          break;
        case 3:
          executeParameter->currentValues[3] = executeParameter->targetValues[3];
          executeParameter->currentValues[4] = executeParameter->targetValues[4];
          executeParameter->currentValues[11] = executeParameter->targetValues[11];
          executeParameter->currentValues[12] = executeParameter->targetValues[12];
          break;
        case 4:
          executeParameter->repetitions--;
      }

      for (element_t element = 0; element < executeParameter->numberElements; element++) {
        _hardware->setPercent(executeParameter->address, element, executeParameter->currentValues[element]);
      }

      if (_state == 4) {
        _state = 0;
      }
      else {
        _state++;
      }

      _pulseTimer = millis();
    }
  }
  else {
    setIdle();
  }
}
