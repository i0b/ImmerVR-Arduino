#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <immerVR.h>

ImmerVR immer;

void setup() {
  immer.begin();
  immer.addModule(0x41, 16, VIBRATE);
  immer.addModule(0x42,  4, TEMPERATURE);
  immer.addModule(0x40,  2, EMS);

  //immer.parseCommand("{\"id\": 0, \"mode\": \"constant\",\"intensity\": 50,
  //\"mask\":255}");
  //immer.parseCommand(
   //   "{\"id\": 0, \"mode\": \"direct\",\"values\": [ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 ] }");
  //immer.parseCommand("{\"id\": 0, \"mode\": \"direct\" }");
  //immer.parseCommand("{\"id\": 0, \"values\": [ 2, 2 ] }");
}

void loop() { immer.run(); }

/*
   // Convert IP address to String
   String ipToString(IPAddress address) {
        return String(address[0]) + "." +
               String(address[1]) + "." +
               String(address[2]) + "." +
               String(address[3]);
   }
 */
