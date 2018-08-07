#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "hardware.h"
#include "module.h"
#include "parser.h"
#include "executeTemperature.h"
#include "executeEMS.h"

//#define DEBUG

Parser::Parser(Module **modules, moduleId_t *numModules) {
  _modules = modules;
  _numModules = numModules;
  _returnStringBuffer = new char[RETURN_STRING_BUFFER_SIZE];
}

char* Parser::parseCommand(String req) {
  if (req.length() < 2) {
    snprintf(_returnStringBuffer, RETURN_STRING_BUFFER_SIZE, "{ \"event\": \"parse-command\", \"status\": \"error\", \"message\": \"no valid JSON string\" }");
    return _returnStringBuffer;
  }

  DynamicJsonBuffer jsonBuffer;

  JsonObject &root = jsonBuffer.parseObject(req);

  //Serial.println(req);

  if (!root.success()) {
    #ifdef DEBUG
    Serial.println("parseObject() failed");
    #endif

    snprintf(_returnStringBuffer, RETURN_STRING_BUFFER_SIZE, "{ \"event\": \"parse-command\", \"status\": \"error\", \"message\": \"no valid JSON string\" }");
    return _returnStringBuffer;
  }

  // get id from json and check if valid module id
  if (!root.containsKey("id") && !root["id"].is<unsigned short>()) {
    snprintf(_returnStringBuffer, RETURN_STRING_BUFFER_SIZE, "{ \"event\": \"parse-command\", \"status\": \"error\", \"message\": \"no module specified\" }");
    return _returnStringBuffer;
  }

  moduleId_t moduleId = (root["id"].as<String>().toInt());

  if (moduleId < 0 || moduleId >= *_numModules) {
    snprintf(_returnStringBuffer, RETURN_STRING_BUFFER_SIZE, "{ \"event\": \"parse-command\", \"status\": \"error\", \"message\": \"module identifier must be in range 0-%d\" }", (*_numModules)-1);
    return _returnStringBuffer;
  }

  if (_modules == 0 || _modules[moduleId] == 0) {
#ifdef DEBUG
    Serial.println("module(s) not initialized");
#endif
    snprintf(_returnStringBuffer, RETURN_STRING_BUFFER_SIZE, "{ \"event\": \"parse-command\", \"status\": \"error\", \"message\": \"internal error\" }");
    return _returnStringBuffer;
  }

// TODO
/*
  if (root.containsKey("update") && _modules[moduleId]->getModuleType() == EMS) {
    _modules[moduleId]->executeParameter->mask = 1;
    _modules[moduleId]->executeParameter->updated = true;
  }

*/

  if (root.containsKey("values") && root["values"].is<JsonArray &>()) {
    if (root["values"].size() != _modules[moduleId]->execute->executeParameter->numberElements) {
      snprintf(_returnStringBuffer, RETURN_STRING_BUFFER_SIZE, "{ \"event\": \"parse-command\", \"status\": \"error\", \"message\": \"number of given values for module %d must be %d\" }", moduleId, _modules[moduleId]->execute->executeParameter->numberElements);
      return _returnStringBuffer;
    }

    if (_modules[moduleId]->type == VIBRATE) {
      for (element_t element = 0; element < root["values"].size(); element++) {
        value_t newValue = root["values"][element].as<int>();
        if (newValue >= 0 && newValue <= 100) {
          _modules[moduleId]->execute->executeParameter->targetValues[element] = newValue;
          _modules[moduleId]->execute->executeParameter->updated = true;
        }
        else {
          snprintf(_returnStringBuffer, RETURN_STRING_BUFFER_SIZE, "{ \"event\": \"parse-command\", \"status\": \"error\", \"message\": \"some values are not within the valid range of 0-100\" }");
          return _returnStringBuffer;
        }
      }
    } else if (_modules[moduleId]->type == TEMPERATURE) {
      for (element_t element = 0; element < root["values"].size(); element++) {
        if (root["values"][element].as<int>() == 0) {
          _modules[moduleId]->execute->executeParameter->targetValues[element] = 0;
          _modules[moduleId]->execute->executeParameter->updated = true;
        } else {
          float newValue = root["values"][element].as<float>();
          // not less than 20 degree celsius and not more than 40 degree celsius
          if (newValue >= MIN_PELTIER_TEMPERATURE && newValue <= MAX_PELTIER_TEMPERATURE) {
            _modules[moduleId]->execute->executeParameter->targetValues[element] = newValue * 100;
            _modules[moduleId]->execute->executeParameter->updated = true;
          }
          else {
            snprintf(_returnStringBuffer, RETURN_STRING_BUFFER_SIZE, "{ \"event\": \"parse-command\", \"status\": \"error\", \"message\": \"some temperatures are not in valid range of %d-%d°C\" }", MIN_PELTIER_TEMPERATURE, MAX_PELTIER_TEMPERATURE);
            return _returnStringBuffer;
          }
        }
      }
    }
    else if (_modules[moduleId]->type == EMS) {
      for (uint8_t element = 0; element < root["values"].size(); element++) {
        uint8_t newValue = root["values"][element].as<int>();
        if (newValue >= 0 && newValue <= MAX_EMS_VALUE) {
          _modules[moduleId]->execute->executeParameter->targetValues[element] = newValue;
          _modules[moduleId]->execute->executeParameter->updated = true;
        }
      }
    }
  }

  if (root.containsKey("intervalMs") && root["intervalMs"].is<unsigned int>()) {
    intervalMs_t interval = root["intervalMs"].as<String>().toInt();
    _modules[moduleId]->setIntervalMs(interval);
    _modules[moduleId]->execute->executeParameter->updated = true;
  }
  if (root.containsKey("onDurationMs") && root["onDurationMs"].is<unsigned int>()) {
    onDurationMs_t onDurationMs = root["onDurationMs"].as<String>().toInt();
    _modules[moduleId]->setOnDurationMs(onDurationMs);
    _modules[moduleId]->execute->executeParameter->updated = true;
  }
  if (root.containsKey("repetitions") && root["repetitions"].is<unsigned int>()) {
    repetition_t repetitions = root["repetitions"].as<String>().toInt();
    _modules[moduleId]->setRepetitions(repetitions);
    _modules[moduleId]->execute->executeParameter->updated = true;
  }
  if (root.containsKey("mode") && root["mode"].is<const char *>()) {
    String mode = root["mode"].as<String>();

    if (mode == actuatorModeStrings[IDLE]) {
      _readMode = IDLE;
    }
    else if (mode == actuatorModeStrings[CONTINUOUS]) {
      _readMode = CONTINUOUS;
    }
    else if (mode == actuatorModeStrings[PULSE]) {
      _readMode = PULSE;
    }
    else if (mode == actuatorModeStrings[HEARTBEAT]) {
      _readMode = HEARTBEAT;
    }
    else if (mode == actuatorModeStrings[RAIN]) {
      _readMode = RAIN;
    }
    else if (mode == actuatorModeStrings[DASH]) {
      _readMode = DASH;
    }

    // ADD NEW MODE HERE!!!
    //----------------------------------------


    //----------------------------------------
    // ADD NEW MODE HERE!!!

    else {
      snprintf(_returnStringBuffer, RETURN_STRING_BUFFER_SIZE, "{ \"event\": \"parse-command\", \"status\": \"error\", \"message\": \"invalide mode\" }");
      return _returnStringBuffer;
    }

    _modules[moduleId]->setMode(_readMode);
    _modules[moduleId]->execute->executeParameter->updated = true;
    #ifdef DEBUG
    Serial.print("new mode: "); Serial.println(actuatorModeStrings[_readMode]);
    #endif
  }

  snprintf(_returnStringBuffer, RETURN_STRING_BUFFER_SIZE, "{ \"event\": \"parse-command\", \"status\": \"ok\", \"message\": \"module %d updated\" }", moduleId);
  return _returnStringBuffer;
}
