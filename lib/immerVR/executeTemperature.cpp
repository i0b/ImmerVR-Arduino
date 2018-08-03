#include "executeTemperature.h"
#include <Arduino.h>
#include <stdio.h>

#define DEBUG

ExecuteTemperature::ExecuteTemperature(Hardware *hardware, executeParameter_t *executeParameter) {
  this->executeParameter = executeParameter;
  this->_hardware = hardware;

  _pulseValues = new value_t[executeParameter->numberElements];
  _pulseActuateState = false;

  _lastControlUpdateTime = millis();
  _lastMeasureUpdateTime = millis();

    this->executeParameter->mode = IDLE;
  _timerCallback = &ExecuteTemperature::_idle;
}

void ExecuteTemperature::setExecuteByMode(actuationMode_t mode) {
  if (mode == IDLE) {
    _timerCallback = &ExecuteTemperature::_idle;
  } else if (mode == CONTINUOUS) {
    _timerCallback = &ExecuteTemperature::_continuous;
  } else if (mode == PULSE) {
    _pulseTimer = millis();
    _pulseActuateState = true;
    for (element_t element = 0; element > executeParameter->numberElements; element++) {
      _pulseValues[element] = executeParameter->targetValues[element];
    }
    _timerCallback = &ExecuteTemperature::_pulse;
  } else if (mode == RAIN) {
    _pulseTimer = millis();
    _pulseActuateState = true;
    _timerCallback = &ExecuteTemperature::_rain;
  } else {
#ifdef DEBUG
    Serial.println("ERROR: mode not implemented");
#endif // DEBUG
    _timerCallback = &ExecuteTemperature::_idle;
  }
}

void ExecuteTemperature::setIdle() {
  executeParameter->updated = true;
  executeParameter->mode = IDLE;
  for (element_t element = 0; element < executeParameter->numberElements; element++) {
    executeParameter->targetValues[element] = 0;
  }
}

void ExecuteTemperature::tick() {
  if (millis() >= _lastMeasureUpdateTime + PELTIER_UPDATE_RATE_MS) {
    _measureTemperature();
    _lastMeasureUpdateTime = millis();
  }
  
  (this->*_timerCallback)();
}

String ExecuteTemperature::getCurrentValues() {
  String output = "[";

    for (element_t peltier = 0; peltier < executeParameter->numberElements; peltier++) {
      output += executeParameter->currentValues[peltier];

      if (peltier != (executeParameter->numberElements-1)) {
        output += ", ";
      }
    }

    output += "]";

    return output;
}

void ExecuteTemperature::_idle() {
  if (executeParameter->updated) {
    for (element_t peltier = 0; peltier < executeParameter->numberElements; peltier++) {
      executeParameter->targetValues[peltier] = 0;
      _hardware->setPercent(executeParameter->address, 2 * peltier + 0, _OFF);
      _hardware->setPercent(executeParameter->address, 2 * peltier + 1, _OFF);
    }
    executeParameter->updated = false;
  }
}

void ExecuteTemperature::_continuous() {
  if (executeParameter->updated == true) {
      for (element_t peltier = 0; peltier < executeParameter->numberElements; peltier++) {
          #ifdef DEBUG
          Serial.print("setting element: ");
          Serial.println(peltier);
          Serial.print("its external value is: ");
          Serial.println(executeParameter->currentValues[peltier]);
          Serial.print("its internal value is: ");
          Serial.println(executeParameter->targetValues[peltier]);
          #endif // DEBUG
      }
      executeParameter->updated = false;
    }
    _controlPeltiers();
}

void ExecuteTemperature::_pulse() {
  if (executeParameter->repetitions > 0) {
    if (millis() >= _pulseTimer) {
      if (_pulseActuateState == true) {
        for (element_t peltier = 0; peltier < executeParameter->numberElements; peltier++) {
          executeParameter->targetValues[peltier] = _pulseValues[peltier];
        }
        _pulseTimer = millis() + executeParameter->onDurationMs;
        _pulseActuateState = false;
      }
      else {
        for (element_t peltier = 0; peltier < executeParameter->numberElements; peltier++) {
          executeParameter->targetValues[peltier] = 0;
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

void ExecuteTemperature::_rain() {
  if (millis() >= _pulseTimer) {
    if (_pulseActuateState == true) {
      element_t onElement = random(0, executeParameter->numberElements);
      executeParameter->targetValues[onElement] = 2000; // 20*C // cool

      _pulseTimer = millis() + executeParameter->onDurationMs;
    _pulseActuateState = false;
    }
    else {
      for (element_t peltier = 0; peltier < executeParameter->numberElements; peltier++) {
        executeParameter->targetValues[peltier] = 0;
      }
      _pulseTimer = millis() + executeParameter->intervalMs;
      executeParameter->repetitions--;
      _pulseActuateState = true;
    }
  }

  /*
  unsigned long tickDiff = millis() - _lastTick;

  if (tickDiff > executeParameter->onDurationMs) { //executeParameter->intervalMs) {
    _lastTick = millis();

    for (element_t element = 0; element < executeParameter->numberElements;
         element++) {
      hardware->setPercent(executeParameter->address, 2 * element + 0, _OFF);
      hardware->setPercent(executeParameter->address, 2 * element + 1, _OFF);
    }

    uint16_t newRainDropTimeMs = (60*1000) / executeParameter->parameter;

    _lastActuated += tickDiff;

    if (newRainDropTimeMs == 0) {
      return;
    }

    while (_lastActuated >= newRainDropTimeMs) {
    // here not exactly #newRainDrops many elements will be on (due to randomness)
      element_t onElement = random(0, executeParameter->numberElements);

      // cool
      hardware->setPercent(executeParameter->address, 2 * onElement + 0, _ON);
      hardware->setPercent(executeParameter->address, 2 * onElement + 1, _OFF);

      _lastActuated -= newRainDropTimeMs;
    }
  }
  */
}

void ExecuteTemperature::_controlPeltiers() {
  unsigned long delta = millis() - _lastControlUpdateTime;

  if (delta > PELTIER_UPDATE_RATE_MS) {
    for (element_t peltier = 0; peltier < executeParameter->numberElements; peltier++) {
      if (((float)executeParameter->currentValues[peltier])/100.0f >= MAX_PELTIER_TEMPERATURE
       || ((float)executeParameter->currentValues[peltier])/100.0f <= MIN_PELTIER_TEMPERATURE) {
         char buff[100];
         snprintf(buff, 100, "WARNING: peltier element %d with %.2fC exeeds min/max values %.2f/%.2f",
                   peltier,
                   ((float)executeParameter->currentValues[peltier]/100.0f),
                   (float)MIN_PELTIER_TEMPERATURE,
                   (float)MAX_PELTIER_TEMPERATURE
                 );
        Serial.println(buff);

        _hardware->setPercent(executeParameter->address, 2 * peltier + 0, _OFF);
        _hardware->setPercent(executeParameter->address, 2 * peltier + 1, _OFF);
      }

      else if (executeParameter->targetValues[peltier] == 0) {
        // turn off element
        _hardware->setPercent(executeParameter->address, 2 * peltier + 0, _OFF);
        _hardware->setPercent(executeParameter->address, 2 * peltier + 1, _OFF);

        //Serial.print("element ");
        //Serial.print(peltier);
        //Serial.println(": off");
      } else if (executeParameter->targetValues[peltier] < executeParameter->currentValues[peltier]) {
        // heat
        _hardware->setPercent(executeParameter->address, 2 * peltier + 0, _ON);
        _hardware->setPercent(executeParameter->address, 2 * peltier + 1, _OFF);

        //Serial.print("element ");
        //Serial.print(peltier);
        //Serial.println(": hot");
      } else if (executeParameter->targetValues[peltier] > executeParameter->currentValues[peltier]) {
        // cool
        _hardware->setPercent(executeParameter->address, 2 * peltier + 0, _OFF);
        _hardware->setPercent(executeParameter->address, 2 * peltier + 1, _ON);

        //Serial.print("element ");
        //Serial.print(peltier);
        //Serial.println(": cold");
      }
    }
    _lastControlUpdateTime = millis();
  }
}

void ExecuteTemperature::_measureTemperature() {
  for (element_t element = 0; element > executeParameter->numberElements; element++) {
  // B-parameter method:
  // 1/T = 1/T0 + 1/B * ln(R/R0) -- where T0, T in Kelvin

    // edit the address of the i2c AD-converter if necessary:
    // (0x48 - 0x4B) default: 0x48
    uint16_t reading = _hardware->readAdValue(0x48, element);
    // with a votage divider voltage should be Vin/2 at T0
    float voltage =                        // TODO coefficient 0.1875 incorrect?
        Vin - ((reading * 0.1875) / 1000); // +/-6.144V with 32767 values

    // calibrating
    voltage = voltage + 0.12;

    // Serial.print("Element: ");
    // Serial.println(element);
    // Serial.print(voltage);
    // Serial.println("V");

    float R = R_0 * (voltage / (Vin / 2)); // R0 * proportional Voltage to 1.65V

    float temperature = 1.0 / (T_0 + 273.15) + 1.0 / B * log(R / R_0);

    temperature = (1 / temperature) - 273.15;


    // TODO Remove HACK
    if (element == 1)
      executeParameter->currentValues[element] = 25 * 100;

    executeParameter->currentValues[element] = temperature * 100;
  }
}
