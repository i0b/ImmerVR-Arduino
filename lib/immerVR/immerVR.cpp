//#define DEBUG
#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include "immerVR.h"
#include "hardware.h"

ImmerVR::ImmerVR() {
}

void ImmerVR::begin() {
        Serial.begin(BAUDRATE);
        Serial.setTimeout(SERIAL_TIMEOUT);

        _numModules = 0;

        //_server = new WiFiServer(LISTEN_PORT);
        _hardware = new Hardware();
        _parser = new Parser(_modules, &_numModules);

        /*
           // Connect to WiFi
           WiFi.begin(_ssid, _password);
           for (int i=0; (WiFi.status() != WL_CONNECTED) && i<10; i++) {
                delay(500);
         #ifdef DEBUG
                Serial.print(".");
         #endif
           }

           delay(500);

         #ifdef DEBUG
           if (WiFi.status() != WL_CONNECTED) {
                Serial.print("Setting soft-AP...");
                Serial.println(WiFi.softAP(_apSsid, _apPassword) ? "Ready" : "Failed!");
           }
           else {
                Serial.println("connected to WiFi as client");
           }
         #endif

         #ifndef DEBUG
           if (WiFi.status() != WL_CONNECTED) {
                WiFi.softAP("esp8266", "esp8266ap");
           }
         #endif

           // Start the server
           _server->begin();
           _server->setNoDelay(true);
         */

/*
        // Set up mDNS responder:
        // - first argument is the domain name, in this example
        //   the fully-qualified domain name is "esp8266.local"
        // - second argument is the IP address to advertise
        //   we send our IP address on the WiFi network
        if (!MDNS.begin("esp8266")) {
                Serial.println("Error setting up MDNS responder!");
        }
        else {
                Serial.println("mDNS responder started");
        }

        // Add service to MDNS-SD
        MDNS.addService("http", "tcp", 2323);
 */

        _hardware->begin();

        #ifdef DEBUG
        _debugHeartbeatMillis = 0;
        /*
           // Print the IP address
           Serial.println("Local server started on IP:");
           Serial.println(WiFi.localIP());
         */
        #endif

        /*
        #ifdef DISPLAY
        _hardware->displayMessage(0,0, "setup");
        _hardware->displayMessage(0,1, "done");
        #endif
        */
}

void ImmerVR::addModule(i2cAddress_t i2cAddress, uint8_t numElements, moduleType_t type /* ADDRESS, ELEMENTS */){
        _modules[_numModules] = new Module();
        Serial.println("new module added");
        bool initialized = _modules[_numModules]->begin(i2cAddress, numElements, type, _hardware);

        if (initialized == true) {
          _numModules++;
          Serial.println("new module initiallized");
        }
        else {
          delete _modules[_numModules];
        }
}

void ImmerVR::parseCommand(String command) {
  Serial.println(_parser->parseCommand(command));
}

void ImmerVR::_printModuleStates() {
  /*
  { "modules": [{"id": 0, "type": "vibration", "values": [0,0,...,0]},
    {"id": 1, "type": "temperature", "values": [25,25,25,25], "measurments": [25,25,25,25]},
    {"id": 3, "type": "ems", "values": [1,1]}] }
  */
        String output = "{ \"modules\": [";

        for (int module = 0; module < _numModules; module++) {
          // id, type
          output += "{ \"id\": ";
          output += module;
          output += ", \"type\": ";

          if (_modules[module]->type == VIBRATE) {
            output += "\"vibration\"";
          }
          else if (_modules[module]->type == TEMPERATURE) {
            output += "\"temperature\"";
          }
          else if (_modules[module]->type == EMS) {
            output += "\"ems\"";
          }

          // values
          output += ", \"values\": [";

          for (uint8_t value = 0; value < _modules[module]->execute->executeParameter->numberElements; value++) {
            output += _modules[module]->execute->executeParameter->currentValues[value];

            if (value < _modules[module]->execute->executeParameter->numberElements-1) {
              output += ", ";
            }
            else {
              output += "]";
            }
          }

          // mode

          output += ", \"mode\": \"" + actuatorModeStrings[_modules[module]->execute->executeParameter->mode];

          output += "\" }";
          if (module < _numModules-1) {
            output += ", ";
          }
        }

        // all displayed
        output += "] }";

        Serial.println(output);
}

void ImmerVR::run() {
        // debug message every 1s
        unsigned long delta = millis() - _debugHeartbeatMillis;
        if (delta >= DEBUG_INTERVAL) {
          _printModuleStates();
          _debugHeartbeatMillis = millis();
        }

        for (int module = 0; module < _numModules; module++) {
          _modules[module]->tick();
        }

        /*
           // Check if a client has connected
           WiFiClient client = _server->available();
           if (!client) {
                return;
           }

           // Wait until the client sends some data
         #ifdef DEBUG
           Serial.println("new client");
         #endif
           while(!client.available()) {
                delay(1);
           }

           // Read the first line of the request
           String req = client.readStringUntil('\r');
         #ifdef DEBUG
           Serial.print("client request: ");
           Serial.println(req);
         #endif

           client.print(_parser->parseCommand(req));
           client.flush();

           delay(1);

           client.stop();
         #ifdef DEBUG
           Serial.println("Client disonnected");
         #endif

           // The client will actually be disconnected
           // when the function returns and 'client' object is detroyed
         */
        if (Serial.available() > 0) {
                /*
                      //Serial.print(Serial.available());
                      char c = Serial.read();
                      Serial.print(c);

                      if (c == '\n') {
                              _parser->parseCommand(_serialInput);
                              _serialInput = "";
                      }
                      else {
                              _serialInput += c;
                      }
                 */
                _parser->parseCommand(Serial.readStringUntil('\n'));
        }
}
