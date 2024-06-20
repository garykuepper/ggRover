#ifndef ROVER_H
#define ROVER_H

#include "Motor.h"
#include "PS4Controller.h"
#include "Display.h"
#include <SimpleTimer.h>

class Rover {
public:
    Rover();
    void begin();
    void update();

private:
    Motor _motor1;
    Motor _motor2;
    Motor _motor3;
    Motor _motor4;
    PS4Controller _ps4;
    Display _display;
    SimpleTimer _timer;
    void updateDisplay();
    void controlMotors();
};

#endif
