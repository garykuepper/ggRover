// BlinkDriver.cpp
#include "BlinkDriver.h"

BlinkDriver::BlinkDriver(uint8_t pin) {
    blinker = new Blink(pin, 500);
}

void BlinkDriver::run() {
    blinker->update();
}


void BlinkDriver::printStatusChange() {
    String currentStatus = blinker->getStatus();
    if (currentStatus != lastStatus) { // If the LED status has changed
        Serial.println(currentStatus); // Print the new status
        lastStatus = currentStatus;    // Update the last known status
    }
}
