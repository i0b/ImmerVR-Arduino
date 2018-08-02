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
    _pulseTimer = millis();
    _timerCallback = &ExecuteVibrate::_heartbeat;
  } else if (mode == DASH) {
    _dashState = 0;
    _pulseTimer = millis();
    _timerCallback = &ExecuteVibrate::_dash;
  } else if (mode == RAIN) {
    _pulseTimer = millis();
    _pulseActuateState = true;
    _timerCallback = &ExecuteVibrate::_rain;
  } else {
#ifdef DEBUG
    Serial.println("ERROR: pattern not implemented");
#endif // DEBUG

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
          executeParameter->currentValues[element] = 0;
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
  unsigned long currentTime = millis();

  if (currentTime >= _pulseTimer) {
    // time for new raindrops
    if (_pulseActuateState == true) {
      for (element_t element = 0; element < executeParameter->numberElements; element++) {
        _hardware->setPercent(executeParameter->address, element, _OFF);
        executeParameter->currentValues[element] = 0;
      }
      _pulseActuateState = false;
    }
    else {
      unsigned long timeSinceLastDrops = currentTime - (_pulseTimer+executeParameter->onDurationMs);

      element_t numberNewDrops = timeSinceLastDrops/executeParameter->intervalMs;

      if (numberNewDrops > 0) {
        for (element_t drops = 0; drops < numberNewDrops; drops++) {
          element_t onElement = random(0, executeParameter->numberElements);
          executeParameter->currentValues[onElement] = executeParameter->targetValues[onElement];
        }
        _pulseActuateState = true;
      }

      _pulseTimer = _pulseTimer + executeParameter->onDurationMs;
    }
  }
  executeParameter->updated = false;
}


void ExecuteVibrate::_heartbeat() {
  unsigned long tickDiff = millis() - _pulseTimer;
  if (tickDiff > executeParameter->intervalMs + 3 * HEARTBEAT_VIBRATOR_ON_TIME) {
    _pulseTimer = millis();
#ifdef DEBUG
    Serial.print("heartbeat: reset [");
    Serial.print(tickDiff);
    Serial.println("ms]");
#endif // DEBUG
  } else if (tickDiff <= HEARTBEAT_VIBRATOR_ON_TIME) {
  // first beat
#ifdef DEBUG
    Serial.print("heartbeat: first beat [");
    Serial.print(tickDiff);
    Serial.println("ms]");
#endif // DEBUG
    for (element_t element = 0; element < executeParameter->numberElements; element++) {
      _hardware->setPercent(executeParameter->address, element, executeParameter->targetValues[element]);
    }
  }
  // first beat break
  else if (tickDiff <= 2 * HEARTBEAT_VIBRATOR_ON_TIME) {
#ifdef DEBUG
    Serial.print("heartbeat: first pause [");
    Serial.print(tickDiff);
    Serial.println("ms]");
#endif // DEBUG
    for (element_t element = 0; element < executeParameter->numberElements; element++) {
      _hardware->setPercent(executeParameter->address, element, _OFF);
    }
  }
  // second beat
  else if (tickDiff <= 3 * HEARTBEAT_VIBRATOR_ON_TIME) {
#ifdef DEBUG
    Serial.print("heartbeat: second beat [");
    Serial.print(tickDiff);
    Serial.println("ms]");
#endif // DEBUG
    for (element_t element = 0; element < executeParameter->numberElements; element++) {
      _hardware->setPercent(executeParameter->address, element, executeParameter->targetValues[element]);
    }
  }
  // second beat break
  else if (tickDiff <= executeParameter->intervalMs + 3 * HEARTBEAT_VIBRATOR_ON_TIME) {
#ifdef DEBUG
    Serial.print("heartbeat: second pause [");
    Serial.print(tickDiff);
    Serial.println("ms]");
#endif // DEBUG
    for (element_t element = 0; element < executeParameter->numberElements; element++) {
      _hardware->setPercent(executeParameter->address, element, _OFF);
    }
  }
}


void ExecuteVibrate::_dash() {
  // NEEDED: 5 states
  if (_pulseTimer > executeParameter->onDurationMs) {
    for (element_t element = 0; element < executeParameter->numberElements; element++) {
      _hardware->setPercent(executeParameter->address, element, _OFF);
    }

    switch (_dashState) {
      case 1:
        _hardware->setPercent(executeParameter->address, 0, executeParameter->targetValues[0]);
        _hardware->setPercent(executeParameter->address, 7, executeParameter->targetValues[7]);
        _hardware->setPercent(executeParameter->address, 8, executeParameter->targetValues[8]);
        _hardware->setPercent(executeParameter->address, 15, executeParameter->targetValues[15]);
        break;
      case 2:
        _hardware->setPercent(executeParameter->address, 0, executeParameter->targetValues[1]);
        _hardware->setPercent(executeParameter->address, 6, executeParameter->targetValues[6]);
        _hardware->setPercent(executeParameter->address, 9, executeParameter->targetValues[9]);
        _hardware->setPercent(executeParameter->address, 14, executeParameter->targetValues[14]);
        break;
      case 3:
        _hardware->setPercent(executeParameter->address, 2, executeParameter->targetValues[2]);
        _hardware->setPercent(executeParameter->address, 5, executeParameter->targetValues[5]);
        _hardware->setPercent(executeParameter->address, 10, executeParameter->targetValues[10]);
        _hardware->setPercent(executeParameter->address, 13, executeParameter->targetValues[13]);
        break;
      case 4:
        _hardware->setPercent(executeParameter->address, 3, executeParameter->targetValues[3]);
        _hardware->setPercent(executeParameter->address, 4, executeParameter->targetValues[4]);
        _hardware->setPercent(executeParameter->address, 11, executeParameter->targetValues[11]);
        _hardware->setPercent(executeParameter->address, 12, executeParameter->targetValues[12]);
        break;
      case 5:
        setIdle();
        break;
    }

    _dashState++;
  }
  executeParameter->updated = false;
}
