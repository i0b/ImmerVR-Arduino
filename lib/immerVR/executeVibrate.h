#ifndef EXECUTEVIBRATE_H
#define EXECUTEVIBRATE_H

#include "execute.h"
#include "hardware.h"

#define HEARTBEAT_INTERVAL_BETWEEN 150

#define HEARTBEAT_IDLE  0
#define HEARTBEAT_START 1
#define HEARTBEAT_END   2

class ExecuteVibrate : public Execute {
public:
ExecuteVibrate(Hardware* hardware, executeParameter_t* executeParameter);
void setExecuteByMode(actuationMode_t mode);
void setIdle();
void tick();
String getCurrentValues();

private:
unsigned long _pulseTimer;
uint8_t _state;
bool _pulseActuateState;
void (ExecuteVibrate::*_timerCallback)();
void _idle();
void _continuous();
void _pulse();
void _rain();
// TODO heartbeat repetitions
void _heartbeat();
void _dash();
};

#endif // EXECUTEVIBRATE_H
