#ifndef IMMERVR_H
#define IMMERVR_H

#include "module.h"
#include "hardware.h"
#include "parser.h"
//#include <ESP8266WiFi.h>
// The port to listen for incoming TCP connections
//#define LISTEN_PORT         2323
#define BAUDRATE            115200
#define DEBUG_INTERVAL      1000
#define SERIAL_TIMEOUT      10


class ImmerVR {

public:
ImmerVR();
void begin();
void run();
void addModule(i2cAddress_t i2cAddress, element_t numElements, moduleType_t type);
void parseCommand(String command);

private:
// Create an instance of the server
//WiFiServer* _server;
// WiFi parameters
//const char* _ssid = "adventure-time";
//const char* _password = "";
//const char* _apSsid = "esp8266";
//const char* _apPassword = "esp8266ap";

Hardware* _hardware;
Parser* _parser;
Module* _modules[MAX_NUM_MODULES];
moduleId_t _numModules;
uint32_t _debugHeartbeatMillis;
String _serialInput;
void _printModuleStates();
};

#endif //IMMERVR_H
