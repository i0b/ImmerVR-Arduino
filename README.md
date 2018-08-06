# ImmerVR-Arduino
# Immersive Virtual Reality: Hardware Platform

### hardware
    microcontroller: lolin32 --- ESP32 based development board
    module controller: PCA9685 based 16-chanel 12-bit PWM driver breakout
    analog digital converter: ADS1115

### software
    PlatformIO [website](https://platformio.org/)

### dependecies
    ArduinoJson

## example commands
### vibration
```json
{ "id": 0, "mode": "heartbeat", "values": [80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80], "onDurationMs": 100, "intervalMs": 400 }
```

```json
{ "id": 0, "mode": "pulse", "repetitions": 5, "values": [80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80], "onDurationMs": 200, "intervalMs": 500 }
```

```json
{ "id": 0, "mode": "dash", "repetitions": 10, "values": [100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100], "onDurationMs": 100 }
```

### temperature
```json
{ "id": 1, "mode": "continuous", "values": [0, 0, 0, 26]}
```
