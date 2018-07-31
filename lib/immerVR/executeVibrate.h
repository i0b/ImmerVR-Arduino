#ifndef EXECUTEVIBRATE_H
#define EXECUTEVIBRATE_H

#include "execute.h"
#include "hardware.h"

#define HEARTBEAT_VIBRATOR_ON_TIME 100

class ExecuteVibrate : public Execute {
public:
ExecuteVibrate(Hardware* hardware, executeParameter_t* executeParameter);
void setExecuteByPattern(pattern_t pattern);
void setIdle(Hardware* hardware, executeParameter_t* executeParameter);
void tick(Hardware* hardware, executeParameter_t* executeParameter);
String getMeasurements(Hardware *hardware, executeParameter_t *executeParameter);

private:
unsigned long _lastTick;
unsigned long _lastActuated;
uint8_t _lastElement;
void (ExecuteVibrate::*_timerCallback)(Hardware* hardware, executeParameter_t* executeParameter);
void _idle(Hardware* hardware, executeParameter_t* executeParameter);
void _direct(Hardware* hardware, executeParameter_t* executeParameter);
void _constant(Hardware* hardware, executeParameter_t* executeParameter);
void _heartbeat(Hardware* hardware, executeParameter_t* executeParameter);
void _rotation(Hardware* hardware, executeParameter_t* executeParameter);
void _leftRight(Hardware* hardware, executeParameter_t* executeParameter);
void _rain(Hardware* hardware, executeParameter_t* executeParameter);
};

#endif // EXECUTEVIBRATE_H
