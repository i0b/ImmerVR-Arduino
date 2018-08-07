#ifndef PARSER_H
#define PARSER_H

#include <Arduino.h>
#include "module.h"

#define RETURN_STRING_BUFFER_SIZE 300

class Parser {
public:
  Parser(Module **modules, moduleId_t *numModules);
  char* parseCommand(String req);

private:
  Module **_modules;
  moduleId_t *_numModules;
  actuationMode_t _readMode;
  char *_returnStringBuffer;
};

#endif // PARSER_H
