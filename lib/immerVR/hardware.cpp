#include "hardware.h"
#include <Arduino.h>

void Hardware::begin() {
  _gain = GAIN_TWOTHIRDS; // +/-6.144V
  _conversionDelay = ADS1115_CONVERSIONDELAY;
  _bitShift = 0;

  // CAREFUL: if esp8266 / wemos, then change to Wire.begin(D2, D1)!
  //Wire.begin(D2, D1);
  // D2: GPIO4, D1: GPIO5
  #define SCL 22
  #define SDA 4
  Wire.begin(SDA, SCL);
  //Wire.begin(4, 5);
  /*
  #ifdef DISPLAY
  // WeMOS sclPin: D2, sdaPin: D1
  // Wire.begin(D2, D1);
  //_u8g2 = new U8G2_SSD1306_64X48_ER_1_SW_I2C(U8G2_R0, D1, D2, 0);
  _u8x8 = new U8X8_SSD1306_64X48_ER_HW_I2C(0);

  _u8x8->begin();
  _u8x8->setFont(u8x8_font_chroma48medium8_r);
  // see https://github.com/olikraus/u8g2/wiki/u8g2reference#setfont
  //_u8g2->setFont(u8g2_font_6x10_tf);
  #endif
  */
}

/*
#ifdef DISPLAY
// in the current setup the display has a size of 8x6 characters
void Hardware::displayMessage(uint8_t x, uint8_t y, String message) {
*/
  /*
     //_u8g2->clearBuffer();
     _u8g2->setFont(u8g2_font_6x10_tf);
     _u8g2->setFontRefHeightAll();
     _u8g2->userInterfaceMessage("Title1", "Title2", "Title3", " Ok \n Cancel
     ");
     //_u8g2->sendBuffer();
   */
/*
  _u8x8->clearLine(y);
  _u8x8->drawString(x, y, message.c_str());
}
#endif
*/

int Hardware::checkActuatorActive(uint8_t i2cAddress) {
  /*
     uint8_t mode = _read8(i2cAddress, PCA9685_MODE1);
     char buffer[100];
     snprintf(buffer, sizeof buffer, "DEBUG: read8(%x): %x", i2cAddress, mode);
     Serial.println(buffer);
   */
  Wire.beginTransmission(i2cAddress);
  uint8_t error = Wire.endTransmission();

  if (error == 0) {
    return 0;
  } else {
    return -1;
  }
  /*
      if (_read8(i2cAddress, PCA9685_MODE1) != 0xFF) {
              return 0;
      }
      else {
              return -1;
      }
   */
}

void Hardware::resetPwm(uint8_t i2cAddress) {
  _write8(i2cAddress, PCA9685_MODE1, 0x0);
}

void Hardware::setPwmFrequency(uint8_t i2cAddress, float frequency) {
  float prescaleval = 25000000;
  prescaleval /= 4096;
  prescaleval /= frequency;
  prescaleval -= 1;
  uint8_t prescale = floor(prescaleval + 0.5);

  uint8_t oldmode = _read8(i2cAddress, PCA9685_MODE1);
  uint8_t newmode = (oldmode & 0x7F) | 0x10;       // sleep
  _write8(i2cAddress, PCA9685_MODE1, newmode);     // go to sleep
  _write8(i2cAddress, PCA9685_PRESCALE, prescale); // set the prescaler
  _write8(i2cAddress, PCA9685_MODE1, oldmode);
  delay(5);
  _write8(i2cAddress, PCA9685_MODE1, oldmode | 0xa1);
  // This sets the MODE1 register to turn on auto increment.
  // This is why the beginTransmission below was not working.
  // Serial.print("Mode now 0x"); Serial.println(read8(PCA9685_MODE1), HEX);
}

void Hardware::setPwm(uint8_t i2cAddress, uint8_t num, uint16_t on,
                      uint16_t off) {
  Wire.beginTransmission(i2cAddress);
  Wire.write(LED0_ON_L + 4 * num);
  Wire.write(on);
  Wire.write(on >> 8);
  Wire.write(off);
  Wire.write(off >> 8);
  Wire.endTransmission();
}

void Hardware::setPercent(uint8_t i2cAddress, uint8_t channel,
                          uint8_t percent) {
  // Low value equals high ratio
  // uint16_t val = map ( percent, 0, 100, 4095, 0 );
  uint16_t val = map(percent, 100, 0, 4095, 0);

  if (val == 0) {
    // Special value for signal fully on.
    setPwm(i2cAddress, channel, 0, 4096);
  } else if (val == 4095) {
    // Special value for signal fully off.
    setPwm(i2cAddress, channel, 4096, 0);
  } else {
    setPwm(i2cAddress, channel, 0, val);
  }
}

uint8_t Hardware::_read8(uint8_t i2cAddress, uint8_t channel) {
  Wire.beginTransmission(i2cAddress);
  Wire.write(channel);
  Wire.endTransmission();

  Wire.requestFrom((uint8_t)i2cAddress, (uint8_t)1);
  return Wire.read();
}

void Hardware::_write8(uint8_t i2cAddress, uint8_t channel, uint8_t d) {
  Wire.beginTransmission(i2cAddress);
  Wire.write(channel);
  Wire.write(d);
  Wire.endTransmission();
}

// ------------------------------------------------------------------------------------
// -------------------------       AD - Methods
// ---------------------------------
// ------------------------------------------------------------------------------------

void Hardware::_writeRegister(uint8_t i2cAddress, uint8_t reg, uint16_t value) {
  Wire.beginTransmission(i2cAddress);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)(value >> 8));
  Wire.write((uint8_t)(value & 0xFF));
  Wire.endTransmission();
}

uint16_t Hardware::_readRegister(uint8_t i2cAddress, uint8_t reg) {
  Wire.beginTransmission(i2cAddress);
  Wire.write(ADS1015_REG_POINTER_CONVERT);
  Wire.endTransmission();
  Wire.requestFrom(i2cAddress, (uint8_t)2);
  return ((Wire.read() << 8) | Wire.read());
}

uint16_t Hardware::readAdValue(uint8_t i2cAddress, uint8_t channel) {
  if (channel > 3) {
    return 0;
  }

  uint16_t config =
      ADS1015_REG_CONFIG_CQUE_NONE |    // Disable the comparator (default val)
      ADS1015_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
      ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
      ADS1015_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
      ADS1015_REG_CONFIG_DR_1600SPS |   // 1600 samples per second (default)
      ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  config |= _gain;
  switch (channel) {
  case (0):
    config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
    break;
  case (1):
    config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
    break;
  case (2):
    config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
    break;
  case (3):
    config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
    break;
  }

  config |= ADS1015_REG_CONFIG_OS_SINGLE;
  _writeRegister(i2cAddress, ADS1015_REG_POINTER_CONFIG, config);
  delay(_conversionDelay);
  return _readRegister(i2cAddress, ADS1015_REG_POINTER_CONVERT) >> _bitShift;
}
