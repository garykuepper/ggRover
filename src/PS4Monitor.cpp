// PS4Monitor.cpp
#include "PS4Monitor.h"

PS4Monitor::PS4Monitor(uint8_t address) : ds4(address), lastUpdate(0) {}

void PS4Monitor::begin() {
    ds4.begin();
    
}

void PS4Monitor::update() {
    unsigned long currentMillis = millis();
    if (isTimeToUpdate(currentMillis)) {
        lastUpdate = currentMillis;
        ds4.get_ps4();
        if (ds4.ps4_ok) {
            showStatus();
        }
    }
}

bool PS4Monitor::isTimeToUpdate(unsigned long currentMillis) const {
    return (currentMillis - lastUpdate) >= read_interval;
}

void PS4Monitor::showStatus() {
    Serial.println((String)"PS4 left joystick value is x: " + ds4.l_joystick_x + " y: " + ds4.l_joystick_y);
    Serial.println((String)"PS4 right joystick value is x: " + ds4.r_joystick_x + " y: " + ds4.r_joystick_y);
    Serial.println((String)"PS4 R1 button is: " + ds4.button_r1);
    Serial.println((String)"PS4 R2 button is: " + ds4.button_r2);
}
