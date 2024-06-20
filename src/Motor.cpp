#include "Motor.h"

Motor::Motor(uint8_t pwmPin) : _pwmPin(pwmPin) {
    pinMode(_pwmPin, OUTPUT);
}

void Motor::setSpeed(int speed) {
    analogWrite(_pwmPin, speed);
}

void Motor::stop() {
    analogWrite(_pwmPin, 0);
}
