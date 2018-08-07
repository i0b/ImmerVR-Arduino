# ImmerVR-Arduino
# Immersive Virtual Reality: Hardware Platform

## Hardware
    microcontroller: lolin32 --- ESP32 based development board
    module controller: PCA9685 based 16-chanel 12-bit PWM driver breakout
    analog digital converter: ADS1115

## Software
    PlatformIO [website](https://platformio.org/)

## Dependecies
    ArduinoJson

## Initialization and execution
    The library is to be included and initialized in the *main.cpp* file and modules are added with  the *addModule(ADDR, NUM_ELEM, MODULE_TYPE)* function. Inside the *main-loop* the library's *run()* method must be added.

## Commands
### Continuous
####Description
Simple constant actuation where all actuators can be set independently.
####Accepted parameter
values - map for the actuation intensities

#### Example - Vibration
```json
{ "id": 0, "mode": "continuous", "values": [10, 20, 30, 40, 50, 60, 70, 80, 80, 70, 60, 50, 40, 30, 20, 10] }
```

#### Example - Temperature
```json
{ "id": 1, "mode": "continuous", "values": [0, 0, 0, 26]}
```

#### Example - EMS
```json
{ "id": 2, "mode": "continuous", "values": [4, 2]}
```

### Pulse
####Description
Pulsing actuation with variable on-time, time between on sets and number of repetitions.
####Accepted parameter
values - map for the actuators in the *on* condition
onDurationMs - time the actuators will be *on*
intervalMs - time between two pulses
repetitions - number of repetitions

#### Example - Vibration
```json
{ "id": 0, "mode": "pulse", "repetitions": 5, "values": [75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75], "onDurationMs": 200, "intervalMs": 500 }
```
#### Example - Temperature
```json
{ "id": 1, "mode": "pulse", "repetitions": 3, "values": [20, 30, 30, 20], "onDurationMs": 1000, "intervalMs": 5000 }
```

#### Example - EMS
```json
{ "id": 2, "mode": "pulse", "repetitions": 5, "values": [1, 1], "onDurationMs": 1000, "intervalMs": 4000 }
```


### Heartbeat
####Description
Actuation simulating the beating of a heart with two pulses and a variable pause between sets of actuations.
####Accepted parameter
values - map for the actuators in the *on* condition
onDuration - time the actuators will be *on*
intervalMs - time between two pairs of beats

#### Example - Vibration
```json
{ "id": 0, "mode": "heartbeat", "values": [80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80], "onDurationMs": 100, "intervalMs": 400 }
```

### Dash
####Description
Swiping actuation starting from a center equidistant linear motion left and right.
####Accepted parameter
values - map for the actuators in the *on* condition
onDuration - time the actuators will be *on* before continuing with the next state
repetitions - number of repetitions

#### Example - Vibration
```json
{ "id": 0, "mode": "dash", "repetitions": 10, "values": [100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100], "onDurationMs": 100 }
```


### Rain
####Description
Simulation of rain with random *drops* and variable actuation time and time between two actuations.
####Accepted parameter
values - map for the actuators in the *on* condition
onDuration - time the actuators will be *on*
intervalMs - time until a new *drop* spawns

#### Example - Vibration
```json
{ "id": 0, "mode": "rain", "values": [80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80], "onDurationMs": 100, "intervalMs": 40 }
```

#### Example - Temperature
```json
{ "id": 1, "mode": "rain", "values": [20, 0, 0, 20], "onDurationMs": 2000, "intervalMs": 5500 }
```
