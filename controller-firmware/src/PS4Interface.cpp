#include "PS4Interface.h"
#include <Wire.h>

PS4Interface::PS4Interface(uint8_t i2cAddr) 
    : _addr(i2cAddr), _throttle(500), _steering(0) {}

void PS4Interface::begin() {
  Wire.begin();
}

void PS4Interface::update() {
  // Skeleton: Request 8 bytes from USB Host
  Wire.requestFrom(_addr, (uint8_t)8);
  if (Wire.available() >= 8) {
      // Logic to parse PS4 report over I2C
      // (Simplified placeholder)
      _throttle = Wire.read() * 4; 
      _steering = (int)Wire.read() - 127;
  }
}

int PS4Interface::getThrottle() { return _throttle; }
int PS4Interface::getSteering() { return _steering; }

bool PS4Interface::isButtonPressed(uint8_t buttonId) {
  return false; // Placeholder
}
