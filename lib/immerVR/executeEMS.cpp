#include "execute.h"
#include "executeEMS.h"
#include "hardware.h"
#include <Arduino.h>

#define NUMBER_CHANNELS 5

ExecuteEMS::ExecuteEMS(Hardware *hardware, executeParameter_t *executeParameter) {
  this->executeParameter = executeParameter;
  this->_hardware = hardware;

  for (uint8_t channel = 0; channel < NUMBER_CHANNELS; channel++) {
    _hardware->setPercent(executeParameter->address, channel, _OFF);
  }

  _emsMode = TENS1;

  _lastKeypressTimeMs = millis();
  _keyPresses = new uint8_t[KEY_PRESS_QUEUE_MAX];
  _keyPressState = 0;
  _lastKeypressIndex = 0;
  _currentKeypressIndex = 0;

  _timerCallback = &ExecuteEMS::_continuous;
}

void ExecuteEMS::setExecuteByMode(mode_t mode) {
  // switch (mode)
  if (mode == IDLE) {
    _timerCallback = &ExecuteEMS::_idle;
  } else if (mode == CONTINUOUS) {
    _timerCallback = &ExecuteEMS::_continuous;
  } else if (mode == PULSE) {
    _timerCallback = &ExecuteEMS::_pulse;
  } else {
#ifdef DEBUG
    Serial.println("ERROR: mode not implemented");
#endif // DEBUG
    _timerCallback = &ExecuteEMS::_idle;
  }

  executeParameter->mode = mode;
}

void ExecuteEMS::setIdle() {
  executeParameter->updated = true;
  executeParameter->mode = IDLE;
  _timerCallback = &ExecuteEMS::_idle;
  //_idle(hardware, executeParameter);
}

String ExecuteEMS::getCurrentValues() {
  // TODO
  return "";
}

void ExecuteEMS::tick() {
  (this->*_timerCallback)();
  _executeKeypresses();
}


void ExecuteEMS::_idle() {
  if (executeParameter->updated) {
    for (element_t element; element < executeParameter->numberElements; element++) {
      executeParameter->targetValues[element] = 0;

      // for now only having two elements is implemented
      if (executeParameter->currentValues[element] < 0) {
        if(element == 0) {
          _press(RIGHT);
        }
        else if(element == 1) {
          _press(LEFT);
        }
        for (uint8_t times = 0; times < executeParameter->currentValues[element]; times++) {
          _press(DOWN);
        }
        executeParameter->currentValues[element] = 0;
      }
    }

    executeParameter->updated = false;
  }
}

void ExecuteEMS::_continuous() {
  if (executeParameter->updated == true) {
/*
TODO

    if (module->executeParameter->parameter != _mode) {
      _setMode(hardware, executeParameter);
    }
    if (executeParameter->mask == 1) {
      Serial.println("updating EMS module");
      _increaseOnTime();
      executeParameter->mask = 0;
    }
*/

    for (element_t element; element < executeParameter->numberElements; element++) {
      if (executeParameter->currentValues[element] != executeParameter->targetValues[element]) {
        #ifdef DEBUG
        Serial.print("setting element: ");
        Serial.println(element);
        Serial.print("its external value is: ");
        Serial.println(executeParameter->currentValues[element]);
        Serial.print("its internal value is: ");
        Serial.println(executeParameter->targetValues[element]);
        #endif // DEBUG
      }
    }


    _setIntensity(executeParameter->targetValues);
    executeParameter->updated = false;
  }
}

void ExecuteEMS::_pulse() {
  if (executeParameter->repetitions > 0) {
    if (millis() >= _pulseTimer) {
      if (executeParameter->updated == true) {
        _setIntensity(executeParameter->targetValues);
        _pulseTimer = millis() + executeParameter->onDurationMs;
        executeParameter->updated = false;
      }
      else {
        _setIntensity(NULL);
        _pulseTimer = millis() + executeParameter->intervalMs;
        executeParameter->repetitions--;
        executeParameter->updated = true;
      }
    }
  }

  else {
    executeParameter->updated = false;
    setIdle();
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

void ExecuteEMS::_setIntensity(value_t *values) {
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

  bool changed = false;

  for (element_t element = 0; element < executeParameter->numberElements; element++) {
    value_t value = 0;
    if (values != NULL) {
      value = values[element];
    }
    if ((value - executeParameter->currentValues[element]) > 0) {
      changed = true;
      if (element == 0) {
        _press(LEFT);
        _press(LEFT);
      }
      else if (element == 1) {
        _press(RIGHT);
        _press(RIGHT);
      }

      for (uint8_t times = 0; times < (value - executeParameter->currentValues[element]); times++) {
        _press(UP);
      }
    }
    else if ((value - executeParameter->currentValues[element]) < 0) {
      changed = true;
      if (element == 0) {
        _press(LEFT);
        _press(LEFT);
      }
      else if (element == 1) {
        _press(RIGHT);
        _press(RIGHT);
      }

      for (uint8_t times = 0; times < (executeParameter->currentValues[element] - value); times++) {
        _press(DOWN);
      }
    }


    #ifdef DEBUG
    Serial.print(" to delta: ");
    Serial.println(value - executeParameter->currentValues[element]);
    #endif //DEBUG

    executeParameter->currentValues[element] = value;
  }

  if (changed == true) {
    _press(MODE);
  }
  executeParameter->updated = false;
}

void ExecuteEMS::_setMode(emsMode_t emsMode) {
  // set to choose P - mode
  for (uint8_t times = 0; times < 4; times++) {
    _press(MODE);
  }

  if ((emsMode - _emsMode) >= 0) {
    for (uint8_t times = 0; times < (emsMode - _emsMode);
         times++) {
      _press(UP);
    }
  } else {
    for (uint8_t times = 0; times < (_emsMode - emsMode);
         times++) {
      _press(DOWN);
    }
  }

  _press(MODE);

  _emsMode = emsMode;

  for (element_t element = 0; element < executeParameter->numberElements; element++) {
    executeParameter->currentValues[element] = 0;
  }
}

void ExecuteEMS::_executeKeypresses() {
  // nothing to do
  if (_currentKeypressIndex == _lastKeypressIndex) {
    return;
  }

  unsigned long deltaTime = millis() - _lastKeypressTimeMs;

  // 0ms++
  if((deltaTime < 100) && _keyPressState == 0) {
    for (uint8_t channel = 0; channel < NUMBER_CHANNELS; channel++) {
      _hardware->setPercent(executeParameter->address, channel, _ON);
    }
    _keyPressState = 1;
  }
  // 100ms++
  else if((deltaTime < 200) && _keyPressState == 1) {
    _hardware->setPercent(executeParameter->address, _keyPresses[_currentKeypressIndex], _OFF);
    _keyPressState = 2;
  }
  // 200ms++
  else if(deltaTime > 200) {
    _hardware->setPercent(executeParameter->address, _keyPresses[_currentKeypressIndex], _ON);

    for (uint8_t channel = 0; channel < NUMBER_CHANNELS; channel++) {
      _hardware->setPercent(executeParameter->address, channel, _OFF);
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
