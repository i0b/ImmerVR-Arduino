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

typedef enum { TENS1, TENS2, TENS3 } emsMode_t;

class ExecuteEMS : public Execute {
public:
ExecuteEMS(Hardware *hardware, executeParameter_t *executeParameter);
void setExecuteByMode(mode_t mode);
void setIdle();
void tick();
String getCurrentValues();

private:
unsigned long _pulseTimer;
uint8_t *_keyPresses;
uint8_t _keyPressState;
uint8_t _lastKeypressIndex;
uint8_t _currentKeypressIndex;
unsigned long _lastKeypressTimeMs;
emsMode_t _emsMode;
void (ExecuteEMS::*_timerCallback)();
void _idle();
void _continuous();
void _pulse();
void _setValues(value_t* values);
void _setMode(emsMode_t emsMode);
void _setIntensity(value_t *values);
void _executeKeypresses();
void _press(button_t button);
void _increaseOnTime();
};

#endif // EXECUTEEMS_H
