#include "PS4Monitor.h"

PS4Monitor::PS4Monitor(uint8_t i2cAddr, Display& disp) : ds4(i2cAddr), display(disp) {}

void PS4Monitor::begin() {
    ds4.begin();
}

void PS4Monitor::update() {
    ds4.get_ps4();
    if (ds4.ps4_ok) {
        displayPS4Status();
    }
}

void PS4Monitor::displayPS4Status() {
    display.clear();

    String leftStick = "LX: " + String(ds4.l_joystick_x) + " LY: " + String(ds4.l_joystick_y);
    String rightStick = "RX: " + String(ds4.r_joystick_x) + " RY: " + String(ds4.r_joystick_y);

    display.writeText(0, 0, leftStick.c_str());
    display.writeText(0, 16, rightStick.c_str());

    display.update();
}