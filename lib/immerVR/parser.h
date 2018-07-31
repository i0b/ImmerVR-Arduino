#ifndef PARSER_H
#define PARSER_H

#include <Arduino.h>
#include "module.h"

class Parser {
public:
  Parser(Module **modules, uint8_t *numModules);
  String parseCommand(String req);

private:
  Module **_modules;
  uint8_t *_numModules;
  void parseModuleCommand(JsonObject& root);
};

#endif // PARSER_H
