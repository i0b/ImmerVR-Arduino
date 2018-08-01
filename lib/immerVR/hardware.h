#ifndef HARDWARE_H
#define HARDWARE_H

#include <Wire.h>
#include <stddef.h>
#include <stdint.h>
/* for display: <U8g2lib.h> */

typedef enum { VIBRATE, TEMPERATURE, EMS } moduleType_t;
typedef enum {
  IDLE,
  CONTINUOUS,
  PULSE,
  HEARTBEAT,
  RAIN
  //ROTATION,
  //SWIPE_LEFT_RIGHT,
} mode_t;

// zero at uint8_t/2, then in 0.5 degree steps up / down
// zero: 37°C at 1000 0000b = 128d
// e.g.  37.5°C at 129d, 36°C at 126d
//#define TEMPERATURE_TO_BIT(x) (128 + ((x - 37) * 2))
//#define BIT_TO_TEMPERATURE(x) (((x - 128) / 2) + 37)

#define _ON (100)
#define _OFF (0)
#define PCA9685_SUBADR1 0x2
#define PCA9685_SUBADR2 0x3
#define PCA9685_SUBADR3 0x4

#define PCA9685_MODE1 0x0
#define PCA9685_PRESCALE 0xFE

#define LED0_ON_L 0x6
#define LED0_ON_H 0x7
#define LED0_OFF_L 0x8
#define LED0_OFF_H 0x9

#define ALLLED_ON_L 0xFA
#define ALLLED_ON_H 0xFB
#define ALLLED_OFF_L 0xFC
#define ALLLED_OFF_H 0xFD

#define ADS1015_CONVERSIONDELAY (1)
#define ADS1115_CONVERSIONDELAY (8)

#define ADS1015_REG_POINTER_CONVERT (0x00)
#define ADS1015_REG_POINTER_CONFIG (0x01)

#define ADS1015_REG_CONFIG_CQUE_NONE                                           \
  (0x0003) // Disable the comparator and put ALERT/RDY in high state (default)
#define ADS1015_REG_CONFIG_CLAT_NONLAT                                         \
  (0x0000) // Non-latching comparator (default)
#define ADS1015_REG_CONFIG_CPOL_ACTVLOW                                        \
  (0x0000) // ALERT/RDY pin is low when active (default)
#define ADS1015_REG_CONFIG_CMODE_TRAD                                          \
  (0x0000) // Traditional comparator with hysteresis (default)
#define ADS1015_REG_CONFIG_DR_1600SPS                                          \
  (0x0080) // 1600 samples per second (default)
#define ADS1015_REG_CONFIG_MODE_SINGLE                                         \
  (0x0100) // Power-down single-shot mode (default)

#define ADS1015_REG_CONFIG_MUX_SINGLE_0 (0x4000) // Single-ended AIN0
#define ADS1015_REG_CONFIG_MUX_SINGLE_1 (0x5000) // Single-ended AIN1
#define ADS1015_REG_CONFIG_MUX_SINGLE_2 (0x6000) // Single-ended AIN2
#define ADS1015_REG_CONFIG_MUX_SINGLE_3 (0x7000) // Single-ended AIN3

#define ADS1015_REG_CONFIG_PGA_MASK (0x0E00)
#define ADS1015_REG_CONFIG_PGA_6_144V (0x0000) // +/-6.144V range = Gain 2/3
#define ADS1015_REG_CONFIG_PGA_4_096V (0x0200) // +/-4.096V range = Gain 1
#define ADS1015_REG_CONFIG_PGA_2_048V                                          \
  (0x0400) // +/-2.048V range = Gain 2 (default)
#define ADS1015_REG_CONFIG_PGA_1_024V (0x0600) // +/-1.024V range = Gain 4
#define ADS1015_REG_CONFIG_PGA_0_512V (0x0800) // +/-0.512V range = Gain 8
#define ADS1015_REG_CONFIG_PGA_0_256V (0x0A00) // +/-0.256V range = Gain 16

#define ADS1015_REG_CONFIG_OS_SINGLE                                           \
  (0x8000) // Write: Set to start a single-conversion

class Hardware {
public:
  void begin(void);
  int checkActuatorActive(uint8_t i2cAddress);
  void setPercent(uint8_t i2cAddress, uint8_t channel, uint8_t percent);
  void setPwm(uint8_t i2cAddress, uint8_t num, uint16_t on, uint16_t off);
  void setPwmFrequency(uint8_t i2cAddress, float frequency);
  void resetPwm(uint8_t i2cAddress);
  uint16_t readAdValue(uint8_t i2cAddress, uint8_t channel);
  /*
  #ifdef DISPLAY
  void displayMessage(uint8_t x, uint8_t y, String message);
  #endif
  */

private:
  uint8_t _read8(uint8_t i2cAddress, uint8_t channel);
  void _write8(uint8_t i2cAddress, uint8_t channel, uint8_t d);
  uint16_t _readRegister(uint8_t i2cAddress, uint8_t reg);
  void _writeRegister(uint8_t i2cAddress, uint8_t reg, uint16_t value);
  uint8_t _conversionDelay;
  uint8_t _bitShift;
  /*
  #ifdef DISPLAY
  // U8G2_SSD1306_64X48_ER_1_SW_I2C* _u8g2;
  U8X8_SSD1306_64X48_ER_HW_I2C *_u8x8;
  #endif
  */

  typedef enum {
    GAIN_TWOTHIRDS = ADS1015_REG_CONFIG_PGA_6_144V,
    GAIN_ONE = ADS1015_REG_CONFIG_PGA_4_096V,
    GAIN_TWO = ADS1015_REG_CONFIG_PGA_2_048V,
    GAIN_FOUR = ADS1015_REG_CONFIG_PGA_1_024V,
    GAIN_EIGHT = ADS1015_REG_CONFIG_PGA_0_512V,
    GAIN_SIXTEEN = ADS1015_REG_CONFIG_PGA_0_256V
  } _adsGain_t;

  _adsGain_t _gain;
};

#endif // HARDWARE_H
