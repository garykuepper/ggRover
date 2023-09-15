#include "OLEDDriver.h"
#include <Arduino.h>

void OLEDDriver::initialize() {
    oledDisplay.begin();
}

void OLEDDriver::run() {
    displayTimeSinceStart();
}

void OLEDDriver::displayTimeSinceStart() {
    unsigned long elapsedTime = millis();
    oledDisplay.displayTime(elapsedTime);
}
