#include "execute.h"
#include "executeVibrate.h"
#include "hardware.h"
#include <Arduino.h>

ExecuteVibrate::ExecuteVibrate(Hardware *hardware,
                               executeParameter_t *executeParameter) {
  _timerCallback = &ExecuteVibrate::_idle;

  _lastElement = 0;

  _timerCallback = &ExecuteVibrate::_direct;
}
// these funcitons are called in tinterval timed timeslots (e.g. 200ms)

void ExecuteVibrate::setExecuteByMode(mode_t mode) {
  // switch (mode)
  if (pattern == IDLE) {
    _timerCallback = &ExecuteVibrate::_idle;
  } else if (pattern == DIRECT) {
    _timerCallback = &ExecuteVibrate::_direct;
  } else if (pattern == CONSTANT) {
    _timerCallback = &ExecuteVibrate::_constant;
  } else if (pattern == HEARTBEAT) {
    _lastTick = millis();
    _timerCallback = &ExecuteVibrate::_heartbeat;
  } else if (pattern == ROTATION) {
    _lastTick = millis();
    _timerCallback = &ExecuteVibrate::_rotation;
  } else if (pattern == LEFTRIGHT) {
    _lastTick = millis();
    _timerCallback = &ExecuteVibrate::_leftRight;
  } else if (pattern == RAIN) {
    _lastActuated = millis();
    _lastTick = millis();
    _timerCallback = &ExecuteVibrate::_rain;
  } else {
#ifdef DEBUG
    Serial.println("ERROR: pattern not implemented");
#endif // DEBUG

    _timerCallback = &ExecuteVibrate::_idle;
  }
}

void ExecuteVibrate::setIdle(Hardware *hardware,
                             executeParameter_t *executeParameter) {
  executeParameter->updated = true;
  _idle(hardware, executeParameter);
}

String ExecuteVibrate::getMeasurements(Hardware* hardware, executeParameter_t *executeParameter) {
  return "";
}

void ExecuteVibrate::tick(Hardware *hardware,
                          executeParameter_t *executeParameter) {
  (this->*_timerCallback)(hardware, executeParameter);
}

void ExecuteVibrate::_idle(Hardware *hardware,
                           executeParameter_t *executeParameter) {
  if (executeParameter->updated) {
#ifdef DEBUG
    Serial.print("module address: 0x");
    Serial.print(executeParameter->address, HEX);
    Serial.println(" off");
#endif // DEBUG

    for (uint8_t element = 0; element < executeParameter->numberElements;
         element++) {
      hardware->setPercent(executeParameter->address, element, _OFF);
    }
    executeParameter->updated = false;
  }
}

void ExecuteVibrate::_direct(Hardware *hardware,
                             executeParameter_t *executeParameter) {
  if (executeParameter->updated) {
    for (uint8_t element = 0; element < executeParameter->numberElements;
         element++) {
      hardware->setPercent(executeParameter->address, element,
                           executeParameter->elementValues[element]);
    }
    executeParameter->updated = false;
  }
}

void ExecuteVibrate::_constant(Hardware *hardware,
                               executeParameter_t *executeParameter) {
  if (executeParameter->updated) {
#ifdef DEBUG
    Serial.println("constant: updated");
    Serial.print("hardware address: 0x");
    Serial.println(executeParameter->address, HEX);
    Serial.print("number elements: ");
    Serial.println(executeParameter->numberElements);
    Serial.print("mask: ");
    Serial.println(executeParameter->mask);
    Serial.print("intensity: ");
    Serial.println(executeParameter->intensity);
#endif // DEBUG
/*
#ifdef DISPLAY
    hardware->displayMessage(0, 0, "VIBRO");
    hardware->displayMessage(0, 1, "constant");
    hardware->displayMessage(0, 2, "I: " + String(executeParameter->intensity));
    hardware->displayMessage(0, 3, "M: " + String(executeParameter->mask));
#endif // DISPLAY
*/

    mask_t mask = executeParameter->mask;
    for (uint8_t element = 0; element < executeParameter->numberElements;
         element++) {
      if (mask & 0x01) {
        hardware->setPercent(executeParameter->address, element,
                             executeParameter->intensity);
      } else {
        hardware->setPercent(executeParameter->address, element, _OFF);
      }
      mask = mask >> 1;
    }
    executeParameter->updated = false;
  }
}

void ExecuteVibrate::_rotation(Hardware *hardware,
                               executeParameter_t *executeParameter) {
  if (millis() >= _lastTick + executeParameter->intervalMs) {
    _lastTick = millis();
    hardware->setPercent(executeParameter->address, _lastElement, 0);
    // parameter = 0: rotate right
    if (executeParameter->parameter == 0) {
      _lastElement = (_lastElement + 1) % executeParameter->numberElements;
      hardware->setPercent(executeParameter->address, _lastElement,
                           executeParameter->intensity);
    }
    // parameter = 1: rotate left
    else if (executeParameter->parameter == 1) {
      if (_lastElement == 0 ||
          _lastElement >= executeParameter->numberElements) {
        _lastElement = executeParameter->numberElements - 1;
      } else {
        _lastElement = _lastElement - 1;
      }
      hardware->setPercent(executeParameter->address, _lastElement,
                           executeParameter->intensity);
    }
#ifdef DEBUG
    Serial.print("rotate: set element ");
    Serial.print(_lastElement);
    Serial.print(" to ");
    Serial.print(executeParameter->intensity);
    Serial.print(" percent at ");
    Serial.print(_lastTick);
    Serial.println(" ms");
#endif // DEBUG
  }
}
void ExecuteVibrate::_heartbeat(Hardware *hardware,
                                executeParameter_t *executeParameter) {
  unsigned long tickDiff = millis() - _lastTick;
  if (tickDiff >
      executeParameter->intervalMs + 3 * HEARTBEAT_VIBRATOR_ON_TIME) {
    _lastTick = millis();
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
    for (uint8_t element = 0; element < executeParameter->numberElements;
         element++) {
      hardware->setPercent(executeParameter->address, element,
                           executeParameter->intensity);
    }
  }
  // first beat break
  else if (tickDiff <= 2 * HEARTBEAT_VIBRATOR_ON_TIME) {
#ifdef DEBUG
    Serial.print("heartbeat: first pause [");
    Serial.print(tickDiff);
    Serial.println("ms]");
#endif // DEBUG
    for (uint8_t element = 0; element < executeParameter->numberElements;
         element++) {
      hardware->setPercent(executeParameter->address, element, _OFF);
    }
  }
  // second beat
  else if (tickDiff <= 3 * HEARTBEAT_VIBRATOR_ON_TIME) {
#ifdef DEBUG
    Serial.print("heartbeat: second beat [");
    Serial.print(tickDiff);
    Serial.println("ms]");
#endif // DEBUG
    for (uint8_t element = 0; element < executeParameter->numberElements;
         element++) {
      hardware->setPercent(executeParameter->address, element,
                           executeParameter->intensity);
    }
  }
  // second beat break
  else if (tickDiff <=
           executeParameter->intervalMs + 3 * HEARTBEAT_VIBRATOR_ON_TIME) {
#ifdef DEBUG
    Serial.print("heartbeat: second pause [");
    Serial.print(tickDiff);
    Serial.println("ms]");
#endif // DEBUG
    for (uint8_t element = 0; element < executeParameter->numberElements;
         element++) {
      hardware->setPercent(executeParameter->address, element, _OFF);
    }
  }
}

void ExecuteVibrate::_leftRight(Hardware *hardware,
                                executeParameter_t *executeParameter) {
  if (millis() >= _lastTick + executeParameter->intervalMs) {
    _lastTick = millis();
    // parameter = 0: left to right
    if (executeParameter->parameter == 0) {
      if (_lastElement > 7) {
        _lastElement = 0;
      } else {
        _lastElement++;
      }

      switch (_lastElement) {
      case 0:
        hardware->setPercent(executeParameter->address, 11, 0);
        hardware->setPercent(executeParameter->address, 12, 0);
        hardware->setPercent(executeParameter->address, 3,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 4,
                             executeParameter->intensity);
        break;
      case 1:
        hardware->setPercent(executeParameter->address, 3, 0);
        hardware->setPercent(executeParameter->address, 4, 0);
        hardware->setPercent(executeParameter->address, 2,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 5,
                             executeParameter->intensity);
        break;
      case 2:
        hardware->setPercent(executeParameter->address, 2, 0);
        hardware->setPercent(executeParameter->address, 5, 0);
        hardware->setPercent(executeParameter->address, 1,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 6,
                             executeParameter->intensity);
        break;
      case 3:
        hardware->setPercent(executeParameter->address, 1, 0);
        hardware->setPercent(executeParameter->address, 6, 0);
        hardware->setPercent(executeParameter->address, 0,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 7,
                             executeParameter->intensity);
        break;
      case 4:
        hardware->setPercent(executeParameter->address, 0, 0);
        hardware->setPercent(executeParameter->address, 7, 0);
        hardware->setPercent(executeParameter->address, 8,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 15,
                             executeParameter->intensity);
        break;
      case 5:
        hardware->setPercent(executeParameter->address, 8, 0);
        hardware->setPercent(executeParameter->address, 15, 0);
        hardware->setPercent(executeParameter->address, 9,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 14,
                             executeParameter->intensity);
        break;
      case 6:
        hardware->setPercent(executeParameter->address, 9, 0);
        hardware->setPercent(executeParameter->address, 14, 0);
        hardware->setPercent(executeParameter->address, 10,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 13,
                             executeParameter->intensity);
        break;
      case 7:
        hardware->setPercent(executeParameter->address, 10, 0);
        hardware->setPercent(executeParameter->address, 13, 0);
        hardware->setPercent(executeParameter->address, 11,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 12,
                             executeParameter->intensity);
        break;
      }
#ifdef DEBUG
      Serial.print("left-right: setting column ");
#endif // DEBUG
    }
    // parameter = 1: down to up
    else if (executeParameter->parameter == 1) {
      if (_lastElement > 7) {
        _lastElement = 0;
      } else {
        _lastElement++;
      }

      switch (_lastElement) {
      case 0:
        hardware->setPercent(executeParameter->address, 3, 0);
        hardware->setPercent(executeParameter->address, 4, 0);
        hardware->setPercent(executeParameter->address, 11,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 12,
                             executeParameter->intensity);
        break;
      case 1:
        hardware->setPercent(executeParameter->address, 11, 0);
        hardware->setPercent(executeParameter->address, 12, 0);
        hardware->setPercent(executeParameter->address, 10,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 13,
                             executeParameter->intensity);
        break;
      case 2:
        hardware->setPercent(executeParameter->address, 10, 0);
        hardware->setPercent(executeParameter->address, 13, 0);
        hardware->setPercent(executeParameter->address, 9,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 14,
                             executeParameter->intensity);
        break;
      case 3:
        hardware->setPercent(executeParameter->address, 9, 0);
        hardware->setPercent(executeParameter->address, 14, 0);
        hardware->setPercent(executeParameter->address, 8,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 15,
                             executeParameter->intensity);
        break;
      case 4:
        hardware->setPercent(executeParameter->address, 8, 0);
        hardware->setPercent(executeParameter->address, 15, 0);
        hardware->setPercent(executeParameter->address, 0,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 7,
                             executeParameter->intensity);
        break;
      case 5:
        hardware->setPercent(executeParameter->address, 0, 0);
        hardware->setPercent(executeParameter->address, 7, 0);
        hardware->setPercent(executeParameter->address, 1,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 6,
                             executeParameter->intensity);
        break;
      case 6:
        hardware->setPercent(executeParameter->address, 1, 0);
        hardware->setPercent(executeParameter->address, 6, 0);
        hardware->setPercent(executeParameter->address, 2,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 5,
                             executeParameter->intensity);
        break;
      case 7:
        hardware->setPercent(executeParameter->address, 2, 0);
        hardware->setPercent(executeParameter->address, 5, 0);
        hardware->setPercent(executeParameter->address, 3,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 4,
                             executeParameter->intensity);
        break;
      }
#ifdef DEBUG
      Serial.print("right-left: setting column ");
#endif // DEBUG
    }
    // parameter = 2: right to left to right ...
    else if (executeParameter->parameter == 2) {
      if (_lastElement > 13) {
        _lastElement = 0;
      } else {
        _lastElement++;
      }

      switch (_lastElement) {
      case 0:
        hardware->setPercent(executeParameter->address, 2, 0);
        hardware->setPercent(executeParameter->address, 5, 0);
        hardware->setPercent(executeParameter->address, 3,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 4,
                             executeParameter->intensity);
        break;
      case 1:
        hardware->setPercent(executeParameter->address, 3, 0);
        hardware->setPercent(executeParameter->address, 4, 0);
        hardware->setPercent(executeParameter->address, 2,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 5,
                             executeParameter->intensity);
        break;
      case 2:
        hardware->setPercent(executeParameter->address, 2, 0);
        hardware->setPercent(executeParameter->address, 5, 0);
        hardware->setPercent(executeParameter->address, 1,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 6,
                             executeParameter->intensity);
        break;
      case 3:
        hardware->setPercent(executeParameter->address, 1, 0);
        hardware->setPercent(executeParameter->address, 6, 0);
        hardware->setPercent(executeParameter->address, 0,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 7,
                             executeParameter->intensity);
        break;
      case 4:
        hardware->setPercent(executeParameter->address, 0, 0);
        hardware->setPercent(executeParameter->address, 7, 0);
        hardware->setPercent(executeParameter->address, 8,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 15,
                             executeParameter->intensity);
        break;
      case 5:
        hardware->setPercent(executeParameter->address, 8, 0);
        hardware->setPercent(executeParameter->address, 15, 0);
        hardware->setPercent(executeParameter->address, 9,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 14,
                             executeParameter->intensity);
        break;
      case 6:
        hardware->setPercent(executeParameter->address, 9, 0);
        hardware->setPercent(executeParameter->address, 14, 0);
        hardware->setPercent(executeParameter->address, 10,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 13,
                             executeParameter->intensity);
        break;
      case 7:
        hardware->setPercent(executeParameter->address, 10, 0);
        hardware->setPercent(executeParameter->address, 13, 0);
        hardware->setPercent(executeParameter->address, 11,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 12,
                             executeParameter->intensity);
        break;
      case 8:
        hardware->setPercent(executeParameter->address, 11, 0);
        hardware->setPercent(executeParameter->address, 12, 0);
        hardware->setPercent(executeParameter->address, 10,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 13,
                             executeParameter->intensity);
        break;
      case 9:
        hardware->setPercent(executeParameter->address, 10, 0);
        hardware->setPercent(executeParameter->address, 13, 0);
        hardware->setPercent(executeParameter->address, 9,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 14,
                             executeParameter->intensity);
        break;
      case 10:
        hardware->setPercent(executeParameter->address, 9, 0);
        hardware->setPercent(executeParameter->address, 14, 0);
        hardware->setPercent(executeParameter->address, 8,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 15,
                             executeParameter->intensity);
        break;
      case 11:
        hardware->setPercent(executeParameter->address, 8, 0);
        hardware->setPercent(executeParameter->address, 15, 0);
        hardware->setPercent(executeParameter->address, 0,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 7,
                             executeParameter->intensity);
        break;
      case 12:
        hardware->setPercent(executeParameter->address, 0, 0);
        hardware->setPercent(executeParameter->address, 7, 0);
        hardware->setPercent(executeParameter->address, 1,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 6,
                             executeParameter->intensity);
        break;
      case 13:
        hardware->setPercent(executeParameter->address, 1, 0);
        hardware->setPercent(executeParameter->address, 6, 0);
        hardware->setPercent(executeParameter->address, 2,
                             executeParameter->intensity);
        hardware->setPercent(executeParameter->address, 5,
                             executeParameter->intensity);
        break;
      }
#ifdef DEBUG
      Serial.print("left-right-left-right: setting column ");
#endif // DEBUG
    }
#ifdef DEBUG
    Serial.print(_lastElement);
    Serial.print(" to ");
    Serial.print(executeParameter->intensity);
    Serial.print(" percent at ");
    Serial.print(_lastTick);
    Serial.println(" ms");
#endif // DEBUG
  }
}

void ExecuteVibrate::_rain(Hardware *hardware, executeParameter_t *executeParameter) {
  // intensity: intensity of actuation
  // interval:  on time in ms for impact
  // parameter: raindrops per minute

  // simple algorithm: only change state every IMPACT_ON_DURATION milliseconds
  //                    - turn off all actuators
  //                    - turn on as many as needed

  unsigned long tickDiff = millis() - _lastTick;

  //if (tickDiff > IMPACT_ON_DURATION) {
  if (tickDiff > executeParameter->intervalMs) {
    _lastTick = millis();

    for (uint8_t element = 0; element < executeParameter->numberElements;
         element++) {
      hardware->setPercent(executeParameter->address, element, _OFF);
    }

    uint16_t newRainDropTimeMs = (60*1000) / executeParameter->parameter;

    _lastActuated += tickDiff;

    while (_lastActuated >= newRainDropTimeMs) {
    // here not exactly #newRainDrops many elements will be on (due to randomness)
      uint8_t onElement = random(0, executeParameter->numberElements);

      hardware->setPercent(executeParameter->address, onElement,
                           executeParameter->intensity);

      _lastActuated -= newRainDropTimeMs;
    }
  }
}
