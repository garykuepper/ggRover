#include "Drivetrain.h"

Drivetrain::Drivetrain(int flPin, int frPin, int rlPin, int rrPin)
    : _flPin(flPin), _frPin(frPin), _rlPin(rlPin), _rrPin(rrPin) {}

void Drivetrain::begin() {
  pinMode(_flPin, OUTPUT);
  pinMode(_frPin, OUTPUT);
  pinMode(_rlPin, OUTPUT);
  pinMode(_rrPin, OUTPUT);
  stop();
}

void Drivetrain::drive(int throttle, int steering) {
  // Skeleton implementation for motor mixing
  int leftSpeed = throttle + steering;
  int rightSpeed = throttle - steering;

  // Constrain and apply to PWM pins (simplified)
  analogWrite(_flPin, constrain(leftSpeed, 0, 255));
  analogWrite(_rlPin, constrain(leftSpeed, 0, 255));
  analogWrite(_frPin, constrain(rightSpeed, 0, 255));
  analogWrite(_rrPin, constrain(rightSpeed, 0, 255));
}

void Drivetrain::stop() {
  analogWrite(_flPin, 0);
  analogWrite(_frPin, 0);
  analogWrite(_rlPin, 0);
  analogWrite(_rrPin, 0);
}
