#ifndef PARSER_H
#define PARSER_H

#include <Arduino.h>
#include "module.h"

class Parser {
public:
  Parser(Module **modules, moduleId_t *numModules);
  String parseCommand(String req);

private:
  Module **_modules;
  moduleId_t *_numModules;
  actuationMode_t _readMode;
};

#endif // PARSER_H
