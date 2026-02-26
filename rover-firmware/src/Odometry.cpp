#include "Odometry.h"

Odometry::Odometry() : _leftTicks(0), _rightTicks(0) {}

void Odometry::begin() {
  // TODO: Initialize STM32 Hardware Timers (TIM2, TIM3 etc.) in Encoder Mode
}

void Odometry::reset() {
  _leftTicks = 0;
  _rightTicks = 0;
}

uint32_t Odometry::getLeftTicks() {
  // TODO: Read from hardware timer register
  return _leftTicks;
}

uint32_t Odometry::getRightTicks() {
  // TODO: Read from hardware timer register
  return _rightTicks;
}

float Odometry::getDistance() {
  // Example calculation based on wheel diameter and ticks per resolution
  return (_leftTicks + _rightTicks) / 2.0f * 0.5f; 
}
