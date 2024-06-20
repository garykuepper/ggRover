#include "Rover.h"

Rover::Rover()
    : _motor1(PWM_PIN1), _motor2(PWM_PIN2), _motor3(PWM_PIN3), _motor4(PWM_PIN4) {}

void Rover::begin() {
    _ps4.begin();
    _display.init();
    _timer.setInterval(20, std::bind(&Rover::updateDisplay, this));
    _timer.setInterval(50, std::bind(&Rover::controlMotors, this));
}

void Rover::update() {
    _timer.run();
    _ps4.update();
}

void Rover::updateDisplay() {
    _display.clear();
    _display.showTimeSinceStart();
    String xJoy = "L_X:" + String(_ps4.getLX()) + " R_X:" + String(_ps4.getRX());
    String yJoy = "L_Y:" + String(_ps4.getLY()) + " R_Y:" + String(_ps4.getRY());
    _display.writeText(0, 0, xJoy);
    _display.writeText(0, 10, yJoy);
    _display.update();
}

void Rover::controlMotors() {
    int lx = _ps4.getLX();
    int ly = _ps4.getLY();
    int rx = _ps4.getRX();
    int ry = _ps4.getRY();
    // Control logic for motors using joystick values
    _motor1.setSpeed(lx);
    _motor2.setSpeed(ly);
    _motor3.setSpeed(rx);
    _motor4.setSpeed(ry);
}
