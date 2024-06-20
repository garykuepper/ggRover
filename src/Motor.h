#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

class Motor {
public:
    Motor(uint8_t pwmPin);
    void setSpeed(int speed);
    void stop();

private:
    uint8_t _pwmPin;
};

#endif
