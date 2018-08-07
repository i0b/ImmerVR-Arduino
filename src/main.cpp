#include <immerVR.h>

ImmerVR immer;

void setup() {
  immer.begin();
  immer.addModule(0x41, 16, VIBRATE);
  immer.addModule(0x42,  4, TEMPERATURE);
  immer.addModule(0x40,  2, EMS);
}

void loop() { immer.run(); }
