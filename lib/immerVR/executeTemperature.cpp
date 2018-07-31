#include "execute.h"
#include "executeTemperature.h"
#include "hardware.h"
#include <Arduino.h>
#include <stdio.h>
#define DEBUG

ExecuteTemperature::ExecuteTemperature(Hardware *hardware,
                                       executeParameter_t *executeParameter) {
  _timerCallback = &ExecuteTemperature::_idle;
  _lastControlUpdateTime = millis();

  _setPeltierTemperature = new float[executeParameter->numberElements];
  for (uint8_t peltier = 0; peltier < executeParameter->numberElements;
       peltier++) {
    _setPeltierTemperature[peltier] = 0;
    executeParameter->elementValues[peltier] = TEMPERATURE_TO_BIT(0);
  }
  _timerCallback = &ExecuteTemperature::_direct;
}

void ExecuteTemperature::setExecuteByPattern(pattern_t pattern) {
  if (pattern == IDLE) {
    _timerCallback = &ExecuteTemperature::_idle;
  }
  else if (pattern == DIRECT) {
    _timerCallback = &ExecuteTemperature::_direct;
  }
  else if (pattern == CONSTANT) {
    _timerCallback = &ExecuteTemperature::_constant;
  } else if (pattern == RAIN) {
    _lastTick = millis();
    _lastActuated = millis();
    _timerCallback = &ExecuteTemperature::_rain;
  }
  else {
#ifdef DEBUG
    Serial.println("ERROR: pattern not implemented");
#endif // DEBUG
    _timerCallback = &ExecuteTemperature::_idle;
  }
}

void ExecuteTemperature::setIdle(Hardware *hardware,
                                 executeParameter_t *executeParameter) {
  executeParameter->updated = true;
  //_idle(hardware, executeParameter);
}

void ExecuteTemperature::tick(Hardware *hardware,
                              executeParameter_t *executeParameter) {
  // TODO check for min/max temperature here, probably disable peltier element(s)
  (this->*_timerCallback)(hardware, executeParameter);
}

String ExecuteTemperature::getMeasurements(Hardware* hardware, executeParameter_t *executeParameter) {
  String output = "[";

    for (uint8_t peltier = 0; peltier < executeParameter->numberElements; peltier++) {
      output += _getTemperature(hardware, peltier);

      if(peltier != (executeParameter->numberElements-1)) {
        output += ", ";
      }
    }

    output += "]";

    return output;
}

void ExecuteTemperature::_idle(Hardware *hardware,
                               executeParameter_t *executeParameter) {
  if (executeParameter->updated) {
    for (uint8_t peltier = 0; peltier < executeParameter->numberElements; peltier++) {
        hardware->setPercent(executeParameter->address, 2 * peltier + 0, _OFF);
        hardware->setPercent(executeParameter->address, 2 * peltier + 1, _OFF);
    }
    executeParameter->updated = false;
  }
}

void ExecuteTemperature::_constant(Hardware *hardware,
                                   executeParameter_t *executeParameter) {
  for (uint8_t peltier = 0; peltier < executeParameter->numberElements;
       peltier++) {
    // check if peltier element #peltier should be on
    uint8_t currentDeltaTemperature =
        TEMPERATURE_TO_BIT(_getTemperature(hardware, peltier));
    if (executeParameter->elementValues[peltier] == 0) {
      // turn off element
      hardware->setPercent(executeParameter->address, 2 * peltier + 0, _OFF);
      hardware->setPercent(executeParameter->address, 2 * peltier + 1, _OFF);
      // Serial.print("element ");
      // Serial.print(peltier);
      // Serial.println(": off");
    } else if (executeParameter->elementValues[peltier] <
               currentDeltaTemperature) {
      // heat
      hardware->setPercent(executeParameter->address, 2 * peltier + 0, _ON);
      hardware->setPercent(executeParameter->address, 2 * peltier + 1, _OFF);
      // Serial.print("element ");
      // Serial.print(peltier);
      // Serial.println(": hot");
    } else if (executeParameter->elementValues[peltier] >
               currentDeltaTemperature) {
      // cool
      hardware->setPercent(executeParameter->address, 2 * peltier + 0, _OFF);
      hardware->setPercent(executeParameter->address, 2 * peltier + 1, _ON);
      // Serial.print("element ");
      // Serial.print(peltier);
      // Serial.println(": cold");
    }
  }
  executeParameter->updated = false;
}

void ExecuteTemperature::_direct(Hardware *hardware, executeParameter_t *executeParameter) {
  if (executeParameter->updated) {
    for (uint8_t peltier = 0; peltier < executeParameter->numberElements; peltier++) {
      _setPeltierTemperature[peltier] = BIT_TO_TEMPERATURE(executeParameter->elementValues[peltier]);
    }
    executeParameter->updated = false;
  }
  _controlPeltiers(hardware, executeParameter);
}

void ExecuteTemperature::_rain(Hardware *hardware, executeParameter_t *executeParameter) {
  // parameter: raindrops per minute
  // interval:  on time

  // simple algorithm: only change state every IMPACT_ON_DURATION milliseconds
  //                    - turn off all actuators
  //                    - turn on as many as needed

  unsigned long tickDiff = millis() - _lastTick;

  if (tickDiff > 2000){ //executeParameter->intervalMs) {
    _lastTick = millis();

    for (uint8_t element = 0; element < executeParameter->numberElements;
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
      uint8_t onElement = random(0, executeParameter->numberElements);

      // cool
      hardware->setPercent(executeParameter->address, 2 * onElement + 0, _ON);
      hardware->setPercent(executeParameter->address, 2 * onElement + 1, _OFF);

      _lastActuated -= newRainDropTimeMs;
    }
  }
}

void ExecuteTemperature::_controlPeltiers(Hardware *hardware, executeParameter_t *executeParameter) {
  unsigned long delta = millis() - _lastControlUpdateTime;
  // every 200ms

  if (delta > PELTIER_UPDATE_RATE_MS) {
    for (uint8_t peltier = 0; peltier < executeParameter->numberElements; peltier++) {
      float currentTemperature = _getTemperature(hardware, peltier);

      if (currentTemperature >= MAX_PELTIER_TEMPERATURE
       || currentTemperature <= MIN_PELTIER_TEMPERATURE) {
         char buff[100];
         snprintf(buff, 100, "WARNING: peltier element %d with %.2fC exeeds min/max values %d/%d",
                   peltier,
                   currentTemperature,
                   (int)MIN_PELTIER_TEMPERATURE,
                   (int)MAX_PELTIER_TEMPERATURE
                 );
        Serial.println(buff);

        hardware->setPercent(executeParameter->address, 2 * peltier + 0, _OFF);
        hardware->setPercent(executeParameter->address, 2 * peltier + 1, _OFF);
      }

      else if (_setPeltierTemperature[peltier] == 0) {
        // turn off element
        hardware->setPercent(executeParameter->address, 2 * peltier + 0, _OFF);
        hardware->setPercent(executeParameter->address, 2 * peltier + 1, _OFF);

        //Serial.print("element ");
        //Serial.print(peltier);
        //Serial.println(": off");

      } else if (_setPeltierTemperature[peltier] < currentTemperature) {
        // heat
        hardware->setPercent(executeParameter->address, 2 * peltier + 0, _ON);
        hardware->setPercent(executeParameter->address, 2 * peltier + 1, _OFF);

        //Serial.print("element ");
        //Serial.print(peltier);
        //Serial.println(": hot");
      } else if (_setPeltierTemperature[peltier] > currentTemperature) {
        // cool
        hardware->setPercent(executeParameter->address, 2 * peltier + 0, _OFF);
        hardware->setPercent(executeParameter->address, 2 * peltier + 1, _ON);

        //Serial.print("element ");
        //Serial.print(peltier);
        //Serial.println(": cold");
      }
    }
    _lastControlUpdateTime = millis();
  }
}

float ExecuteTemperature::_getTemperature(Hardware *hardware, uint8_t element) {
// B-parameter method:
// 1/T = 1/T0 + 1/B * ln(R/R0) -- where T0, T in Kelvin

  // edit the address of the i2c AD-converter if necessary:
  // (0x48 - 0x4B) default: 0x48
  uint16_t reading = hardware->readAdValue(0x48, element);
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
    return 25;

  return temperature;
}
