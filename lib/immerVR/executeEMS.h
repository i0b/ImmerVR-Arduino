#ifndef EXECUTEEMS_H
#define EXECUTEEMS_H

#define KEY_PRESS_QUEUE_MAX 64

#include "execute.h"
#include "hardware.h"

/*
 #define  EMS_DOWN 0
 #define EMS_RIGHT 1
 #define  EMS_MODE 2
 #define    EMS_UP 3
 #define  EMS_LEFT 4
 */

//typedef enum { UP, RIGHT, LEFT, DOWN, MODE } button_t; // old mapping
typedef enum { DOWN, RIGHT, MODE, UP, LEFT } button_t;

class ExecuteEMS : public Execute {
public:
ExecuteEMS(Hardware* hardware, executeParameter_t* executeParameter);
void setExecuteByPattern(pattern_t pattern);
void setIdle(Hardware* hardware, executeParameter_t* executeParameter);
void tick(Hardware* hardware, executeParameter_t* executeParameter);
String getMeasurements(Hardware *hardware, executeParameter_t *executeParameter);

private:
uint8_t *_intensity;
uint8_t *_keyPresses;
uint8_t _keyPressState;
uint8_t _lastKeypressIndex;
uint8_t _currentKeypressIndex;
unsigned long _lastKeypressTimeMs;
unsigned long _onTime;
uint8_t _mode;
void (ExecuteEMS::*_timerCallback)(Hardware* hardware, executeParameter_t* executeParameter);
void _idle(Hardware* hardware, executeParameter_t* executeParameter);
void _direct(Hardware* hardware, executeParameter_t* executeParameter);
void _rain(Hardware* hardware, executeParameter_t* executeParameter);
void _setIntensity(Hardware* hardware, uint8_t element, executeParameter_t* executeParameter);
void _setMode(Hardware* hardware, executeParameter_t* executeParameter);
void _executeKeypresses(Hardware* hardware, executeParameter_t* executeParameter);
void _press(button_t button);
void _increaseOnTime();
};

#endif // EXECUTEEMS_H
