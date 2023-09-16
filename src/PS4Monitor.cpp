// PS4Monitor.cpp
#include "PS4Monitor.h"

PS4Monitor::PS4Monitor(uint8_t address) : ds4(address) {}

void PS4Monitor::begin()
{
    ds4.begin();
}

void PS4Monitor::update()
{
    ds4.get_ps4();
}

void PS4Monitor::showStatus()
{
    Serial.println((String) "PS4 left joystick value is x: " + ds4.l_joystick_x + " y: " + ds4.l_joystick_y);
    Serial.println((String) "PS4 right joystick value is x: " + ds4.r_joystick_x + " y: " + ds4.r_joystick_y);
    Serial.println((String) "PS4 R1 button is: " + ds4.button_r1);
    Serial.println((String) "PS4 R2 button is: " + ds4.button_r2);
}

uint8_t PS4Monitor::getLX()
{
    return ds4.l_joystick_x;
}
uint8_t PS4Monitor::getLY()
{
    return ds4.l_joystick_y;
}
uint8_t PS4Monitor::getRX()
{
    return ds4.r_joystick_x;
}
uint8_t PS4Monitor::getRY()
{
    return ds4.r_joystick_y;
}