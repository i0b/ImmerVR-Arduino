#include "execute.h"
#include "executeEMS.h"
#include "hardware.h"
#include <Arduino.h>

#define NUMBER_CHANNELS 5

ExecuteEMS::ExecuteEMS(Hardware *hardware,
                       executeParameter_t *executeParameter) {
  _timerCallback = &ExecuteEMS::_idle;
  executeParameter->intensity = 0;

  for (uint8_t channel = 0; channel < NUMBER_CHANNELS; channel++) {
    hardware->setPercent(executeParameter->address, channel, _OFF);
  }

  _mode = 8;
  executeParameter->parameter = 8;

  _intensity = new uint8_t[executeParameter->numberElements];
  for (uint8_t element = 0; element < executeParameter->numberElements; element++) {
    _intensity[element] = 0;
  }

  _lastKeypressTimeMs = millis();
  _keyPresses = new uint8_t[KEY_PRESS_QUEUE_MAX];
  _keyPressState = 0;
  _lastKeypressIndex = 0;
  _currentKeypressIndex = 0;

  _timerCallback = &ExecuteEMS::_direct;
}

void ExecuteEMS::setExecuteByPattern(pattern_t pattern) {
  // switch (mode)
  if (pattern == IDLE) {
    _timerCallback = &ExecuteEMS::_idle;
  } else if (pattern == DIRECT) {
    _timerCallback = &ExecuteEMS::_direct;
  } else if (pattern == RAIN) {
    _timerCallback = &ExecuteEMS::_rain;
  } else {
#ifdef DEBUG
    Serial.println("ERROR: pattern not implemented");
#endif // DEBUG
    _timerCallback = &ExecuteEMS::_idle;
  }
}

void ExecuteEMS::setIdle(Hardware *hardware,
                         executeParameter_t *executeParameter) {
  executeParameter->updated = true;
  executeParameter->pattern = IDLE;
  _timerCallback = &ExecuteEMS::_idle;
  //_idle(hardware, executeParameter);
}

String ExecuteEMS::getMeasurements(Hardware* hardware, executeParameter_t *executeParameter) {
  return "";
}

void ExecuteEMS::tick(Hardware *hardware,
                      executeParameter_t *executeParameter) {
  (this->*_timerCallback)(hardware, executeParameter);
  _executeKeypresses(hardware, executeParameter);
}

void ExecuteEMS::_idle(Hardware *hardware,
                       executeParameter_t *executeParameter) {
  if (executeParameter->updated) {
    for (uint8_t element = 0; element < executeParameter->numberElements; element++) {
      // for now only having two elements is implemented
      if (_intensity[element] < 0) {
        if(element == 0) {
          _press(RIGHT);
        }
        else if(element == 1) {
          _press(LEFT);
        }
        for (uint8_t times = 0; times < _intensity[element]; times++) {
          _press(DOWN);
        }
        _intensity[element] = 0;
      }
    }

    executeParameter->updated = false;
  }
}

void ExecuteEMS::_direct(Hardware *hardware, executeParameter_t *executeParameter) {
  if (executeParameter->updated == true) {
    if (executeParameter->parameter != _mode) {
      _setMode(hardware, executeParameter);
    }

    if (executeParameter->mask == 1) {
      Serial.println("updating EMS module");
      _increaseOnTime();
      executeParameter->mask = 0;
    }

    for (uint8_t element = 0; element < executeParameter->numberElements;
         element++) {
      if ((executeParameter->elementValues[element] - _intensity[element]) != 0) {
        #ifdef DEBUG
        Serial.print("setting element: ");
        Serial.println(element);
        Serial.print("its external value is: ");
        Serial.println(executeParameter->elementValues[element]);
        Serial.print("its internal value is: ");
        Serial.println(_intensity[element]);
        #endif // DEBUG

        _setIntensity(hardware, element, executeParameter);
      }

      #ifdef DEBUG
      Serial.println("DONE setting EMS value");
      #endif // DEBUG
    }
    executeParameter->updated = false;
  }
}

void ExecuteEMS::_increaseOnTime() {
  _press(MODE);
  _press(MODE);

  for(int time=0; time<3; time++) {
    _press(UP);
  }

  for(int time=0; time<3; time++) {
    _press(MODE);
  }
}

void ExecuteEMS::_rain(Hardware *hardware,
                      executeParameter_t *executeParameter) {
  // intensity: intensity value to set for thunder
  // interval: on time
  if (executeParameter->updated) {
    for (uint8_t element = 0; element < executeParameter->numberElements; element++) {
      executeParameter->elementValues[element] = executeParameter->intensity;
      _setIntensity(hardware, element, executeParameter);
    }

    _onTime = millis() + executeParameter->intervalMs;
    executeParameter->updated = false;
  }
  else {
    if (millis() > _onTime) {
      for (uint8_t element = 0; element < executeParameter->numberElements; element++) {
        uint8_t elementIntensityValue = executeParameter->elementValues[element];
        executeParameter->elementValues[element] = 0;
        _setIntensity(hardware, element, executeParameter);
        executeParameter->elementValues[element] = elementIntensityValue;
      }

      setIdle(hardware, executeParameter);
    }
  }
}

void ExecuteEMS::_setIntensity(Hardware *hardware, uint8_t element,
                               executeParameter_t *executeParameter) {
  #ifdef DEBUG
  Serial.print("Setting element: ");
  Serial.print(element);
  #endif //DEBUG
/*
#ifdef DISPLAY
  hardware->displayMessage(0, 0, "EMS");
  hardware->displayMessage(0, 1, "I: " + String(executeParameter->intensity));
#endif // DISPLAY
*/

  if (element == 0) {
    _press(LEFT);
    _press(LEFT);
  }
  else if (element == 1) {
    _press(RIGHT);
    _press(RIGHT);
  }

  if ((executeParameter->elementValues[element] - _intensity[element]) >= 0) {
    for (uint8_t times = 0;
      times < (executeParameter->elementValues[element] - _intensity[element]); times++) {
      _press(UP);
    }
  } else {
    for (uint8_t times = 0;
      times < (_intensity[element] - executeParameter->elementValues[element]); times++) {
      _press(DOWN);
    }
  }

  #ifdef DEBUG
  Serial.print(" to delta: ");
  Serial.println(executeParameter->elementValues[element] - _intensity[element]);
  #endif //DEBUG

  _press(MODE);

  _intensity[element] = executeParameter->elementValues[element];
  executeParameter->updated = false;
}

void ExecuteEMS::_setMode(Hardware *hardware,
                          executeParameter_t *executeParameter) {
  // set to choose P - mode
  for (uint8_t times = 0; times < 4; times++) {
    _press(MODE);
  }

  if ((executeParameter->parameter - _mode) >= 0) {
    for (uint8_t times = 0; times < (executeParameter->parameter - _mode);
         times++) {
      _press(UP);
    }
  } else {
    for (uint8_t times = 0; times < (_mode - executeParameter->parameter);
         times++) {
      _press(DOWN);
    }
  }

  _press(MODE);

  _mode = executeParameter->parameter;

  _intensity[0] = 0;
  _intensity[1] = 0;
}
void ExecuteEMS::_executeKeypresses(Hardware *hardware, executeParameter_t *executeParameter) {
  // nothing to do
  if (_currentKeypressIndex == _lastKeypressIndex) {
    return;
  }

  unsigned long deltaTime = millis() - _lastKeypressTimeMs;

  // 0ms++
  if((deltaTime < 100) && _keyPressState == 0) {
    for (uint8_t channel = 0; channel < NUMBER_CHANNELS; channel++) {
      hardware->setPercent(executeParameter->address, channel, _ON);
    }
    _keyPressState = 1;
  }
  // 100ms++
  else if((deltaTime < 200) && _keyPressState == 1) {
    hardware->setPercent(executeParameter->address, _keyPresses[_currentKeypressIndex], _OFF);
    _keyPressState = 2;
  }
  // 200ms++
  else if(deltaTime > 200) {
    hardware->setPercent(executeParameter->address, _keyPresses[_currentKeypressIndex], _ON);

    for (uint8_t channel = 0; channel < NUMBER_CHANNELS; channel++) {
      hardware->setPercent(executeParameter->address, channel, _OFF);
    }

    #ifdef DEBUG
    Serial.print("Pressed: ");
    Serial.println(_keyPresses[_currentKeypressIndex]);
    #endif // DEBUG
    //resetting for next button
    _keyPressState = 0;
    _lastKeypressTimeMs = millis();
    _currentKeypressIndex = (_currentKeypressIndex+1)%KEY_PRESS_QUEUE_MAX;
  }
}
void ExecuteEMS::_press(button_t button) {
  uint8_t newKeypressIndex = (_lastKeypressIndex+1)%KEY_PRESS_QUEUE_MAX;
  if(newKeypressIndex == _currentKeypressIndex) {
    Serial.println("ERROR: EMS --- buffer for keypresses full");
    return;
  }
  _keyPresses[_lastKeypressIndex] = button;
  _lastKeypressIndex = newKeypressIndex;
}
