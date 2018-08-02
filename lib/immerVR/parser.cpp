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
}

String Parser::parseCommand(String req) {
  if (req.length() < 2) {
    return "ERROR: no valid JSON string.";
  }

  DynamicJsonBuffer jsonBuffer;

  JsonObject &root = jsonBuffer.parseObject(req);

  //Serial.println(req);

  if (!root.success()) {
#ifdef DEBUG
    Serial.println("parseObject() failed");
#endif
    return "ERROR: no valid JSON string.";
  }

  // get id from json and check if valid module id
  if (!root.containsKey("id") && !root["id"].is<unsigned short>()) {
    return "ERROR: No module specified.";
  }

  moduleId_t moduleId = (root["id"].as<String>().toInt());

  if (moduleId < 0 || moduleId >= *_numModules) {
    return "ERROR: Invalide moduleId.";
  }

  if (_modules == 0 || _modules[moduleId] == 0) {
#ifdef DEBUG
    Serial.println("module(s) not initialized");
#endif
    return "ERROR: internal error.";
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
      return "ERROR: number of given values does not match with number of actuators in module";
    }

    if (_modules[moduleId]->type == VIBRATE) {
      for (element_t element = 0; element < root["values"].size(); element++) {
        value_t newValue = root["values"][element].as<int>();
        if (newValue >= 0 && newValue <= 100) {
          _modules[moduleId]->execute->executeParameter->targetValues[element] = newValue;
          _modules[moduleId]->execute->executeParameter->updated = true;
        }
        else {
          return "ERROR: some values are not within the valid range of 0-100";
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
            return "ERROR: some temperatures are not in valid range";
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

    if (mode == "idle") {
      _modules[moduleId]->setMode(IDLE);
      _modules[moduleId]->execute->executeParameter->updated = true;
      #ifdef DEBUG
      Serial.println("new mode: idle");
      #endif
    }
    else if (mode == "continuous") {
      _modules[moduleId]->setMode(CONTINUOUS);
      _modules[moduleId]->execute->executeParameter->updated = true;
      #ifdef DEBUG
      Serial.println("new mode: continuous");
      #endif
    }
    else if (mode == "pulse") {
      _modules[moduleId]->setMode(PULSE);
      _modules[moduleId]->execute->executeParameter->updated = true;
      #ifdef DEBUG
      Serial.println("new mode: pulse");
      #endif
    }
    else if (mode == "heartbeat") {
      _modules[moduleId]->setMode(HEARTBEAT);
      _modules[moduleId]->execute->executeParameter->updated = true;
      #ifdef DEBUG
      Serial.println("new mode: heartbeat");
      #endif
    }
    else if (mode == "rain") {
      _modules[moduleId]->setMode(RAIN);
      _modules[moduleId]->execute->executeParameter->updated = true;
      #ifdef DEBUG
      Serial.println("new mode: rain");
      #endif
    }
    else if (mode == "dash") {
      _modules[moduleId]->setMode(DASH);
      _modules[moduleId]->execute->executeParameter->updated = true;
      #ifdef DEBUG
      Serial.println("new mode: dash");
      #endif
    }
    else {
      return "ERROR: Invalide mode.";
    }
  }

  return "OK";
}
