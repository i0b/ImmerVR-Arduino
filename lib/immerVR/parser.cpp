#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "hardware.h"
#include "module.h"
#include "parser.h"

#define DEBUG

Parser::Parser(Module **modules, uint8_t *numModules) {
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

  if (root.containsKey("modules") && root["modules"].is<JsonArray &>()) {
    for(uint8_t index=0; index<root["modules"].size(); index++) {
      parseModuleCommand(root["modules"][index]);
    }
    return "OK";
  }

  // get id from json and check if valid module id
  if (!root.containsKey("id") && !root["id"].is<unsigned short>()) {
    return "ERROR: No module specified.";
  }

  int8_t moduleId = (root["id"].as<String>().toInt());

  if (moduleId < 0 || moduleId >= *_numModules) {
    return "ERROR: Invalide moduleId.";
  }

  if (_modules == 0 || _modules[moduleId] == 0) {
#ifdef DEBUG
    Serial.println("module(s) not initialized");
#endif
    return "ERROR: internal error.";
  }

  if (root.containsKey("update") && _modules[moduleId]->getModuleType() == EMS) {
    _modules[moduleId]->executeParameter->mask = 1;
    _modules[moduleId]->executeParameter->updated = true;
  }

  if (root.containsKey("values") && root["values"].is<JsonArray &>()) {
    if (root["values"].size() >
        _modules[moduleId]->executeParameter->numberElements) {
      Serial.println("ERROR: value array size too large");
    }

    if (_modules[moduleId]->getModuleType() == VIBRATE) {
      for (uint8_t element = 0; element < root["values"].size(); element++) {
        uint8_t newValue = root["values"][element].as<int>();
        if (newValue >= 0 && newValue <= 100) {
          _modules[moduleId]->executeParameter->elementValues[element] =
              newValue;
          _modules[moduleId]->executeParameter->updated = true;
        }
      }
    } else if (_modules[moduleId]->getModuleType() == TEMPERATURE) {
      for (uint8_t element = 0; element < root["values"].size(); element++) {
        if (root["values"][element].as<int>() == 0) {
          _modules[moduleId]->executeParameter->elementValues[element] =
              TEMPERATURE_TO_BIT(0);
          _modules[moduleId]->executeParameter->updated = true;
        } else {
          float newValue = root["values"][element].as<float>();
          // not less than 20 degree celsius and not more than 40 degree celsius
          if (newValue >= 20 && newValue <= 40) {
            _modules[moduleId]->executeParameter->elementValues[element] =
                TEMPERATURE_TO_BIT(newValue);
            _modules[moduleId]->executeParameter->updated = true;
          }
          else {
            Serial.print("ERROR: set temperature ");
            Serial.print(newValue);
            Serial.println(" not in range 20-40 degree Celsius.");
          }
        }
      }
    }
    else if (_modules[moduleId]->getModuleType() == EMS) {
      for (uint8_t element = 0; element < root["values"].size(); element++) {
        uint8_t newValue = root["values"][element].as<int>();
        if (newValue >= 0 && newValue <= 10) {
          _modules[moduleId]->executeParameter->elementValues[element] =
              newValue;
          _modules[moduleId]->executeParameter->updated = true;
        }
      }
    }
  }

  if (root.containsKey("parameter") && root["parameter"].is<unsigned short>()) {
    // change parameter variables
    // then update actuators
    parameter_t parameter = root["parameter"].as<String>().toInt();
    _modules[moduleId]->setParameter(parameter);

#ifdef DEBUG
    Serial.println("new parameter: " + String(parameter));
#endif
  }

  if (root.containsKey("mask") && root["mask"].is<unsigned int>()) {
    mask_t mask = (root["mask"].as<String>().toInt());

    _modules[moduleId]->setMask(mask);

#ifdef DEBUG
    Serial.println("new mask: " + String(mask));
// Serial.println("new masking: " + String(mask, HEX));
#endif
  }

  if (root.containsKey("interval") && root["interval"].is<unsigned int>()) {
    intervalMs_t interval = root["interval"].as<String>().toInt();
    _modules[moduleId]->setIntervalMs(interval);

#ifdef DEBUG
    Serial.println("new interval: " + String(interval)) + "ms";
#endif
  }

  if (root.containsKey("mode") && root["mode"].is<const char *>()) {
    String mode = root["mode"].as<String>();

    if (mode == "idle") {
      _modules[moduleId]->setMode(IDLE);
#ifdef DEBUG
      Serial.println("new mode: idle");
#endif
    } else if (mode == "direct") {
      _modules[moduleId]->setMode(DIRECT);
#ifdef DEBUG
      Serial.println("new mode: direct");
#endif
    } else if (mode == "constant") {
      _modules[moduleId]->setMode(CONSTANT);
#ifdef DEBUG
      Serial.println("new mode: constant");
#endif
    } else if (mode == "rotate") {
      _modules[moduleId]->setMode(ROTATION);
#ifdef DEBUG
      Serial.println("new mode: rotate");
#endif
    } else if (mode == "leftRight") {
      _modules[moduleId]->setMode(LEFTRIGHT);
#ifdef DEBUG
      Serial.println("new mode: left-right");
#endif
    } else if (mode == "heartbeat") {
      _modules[moduleId]->setMode(HEARTBEAT);
#ifdef DEBUG
      Serial.println("new mode: heartbeat");
#endif
    } else if (mode == "rain") {
      _modules[moduleId]->setMode(RAIN);
#ifdef DEBUG
      Serial.println("new mode: rain");
#endif

    } else {
      return "ERROR: Invalide mode.";
    }

    if (root.containsKey("intensity") &&
        root["intensity"].is<unsigned short>()) {
      // TODO check
      intensity_t intensity = root["intensity"].as<String>().toInt();
      _modules[moduleId]->setIntensity(intensity);

#ifdef DEBUG
      Serial.println("new intensity: " + String(intensity));
#endif
    }
  }

  return "OK";
}

void Parser::parseModuleCommand(JsonObject& root) {
  if (!root.containsKey("id") && !root["id"].is<unsigned short>()) {
    Serial.println("ERROR: No module specified.");
    return;
  }

  int8_t moduleId = (root["id"].as<String>().toInt());

  if (moduleId < 0 || moduleId >= *_numModules) {
    Serial.println("ERROR: Invalide moduleId.");
    return;
  }

  if (_modules == 0 || _modules[moduleId] == 0) {
    Serial.println("ERROR: Module(s) not initialized");
    return;
  }

  if (root.containsKey("values") && root["values"].is<JsonArray &>()) {
    if (root["values"].size() >
        _modules[moduleId]->executeParameter->numberElements) {
      Serial.println("ERROR: value array size too large");
    }

    if (_modules[moduleId]->getModuleType() == VIBRATE) {
      for (uint8_t element = 0; element < root["values"].size(); element++) {
        uint8_t newValue = root["values"][element].as<int>();
        if (newValue >= 0 && newValue <= 100) {
          _modules[moduleId]->executeParameter->elementValues[element] =
              newValue;
          _modules[moduleId]->executeParameter->updated = true;
        }
      }
    } else if (_modules[moduleId]->getModuleType() == TEMPERATURE) {
      for (uint8_t element = 0; element < root["values"].size(); element++) {
        if (root["values"][element].as<int>() == 0) {
          _modules[moduleId]->executeParameter->elementValues[element] =
              TEMPERATURE_TO_BIT(0);
          _modules[moduleId]->executeParameter->updated = true;
        } else {
          float newValue = root["values"][element].as<float>();
          // not less than 20 degree celsius and not more than 40 degree celsius
          if (newValue >= 20 && newValue <= 40) {
            _modules[moduleId]->executeParameter->elementValues[element] =
                TEMPERATURE_TO_BIT(newValue);
            _modules[moduleId]->executeParameter->updated = true;
          }
          else {
            Serial.print("ERROR: set temperature ");
            Serial.print(newValue);
            Serial.println(" not in range 20-40 degree Celsius.");
          }
        }
      }
    }
    else if (_modules[moduleId]->getModuleType() == EMS) {
      for (uint8_t element = 0; element < root["values"].size(); element++) {
        uint8_t newValue = root["values"][element].as<int>();
        if (newValue >= 0 && newValue <= 10) {
          _modules[moduleId]->executeParameter->elementValues[element] =
              newValue;
          _modules[moduleId]->executeParameter->updated = true;
        }
      }
    }
  }
}
