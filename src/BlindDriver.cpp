// BlinkDriver.cpp
#include "BlinkDriver.h"

BlinkDriver::BlinkDriver(uint8_t pin) 
{
    blinker = new Blink(pin, 500);
    startTime = millis();
    lastPrintTime = 0;
}

void BlinkDriver::run() 
{
    blinker->update();
}


void BlinkDriver::printStatusChange() 
{
    String currentStatus = blinker->getStatus();
    if (currentStatus != lastStatus) { // If the LED status has changed
        Serial.println(currentStatus); // Print the new status
        lastStatus = currentStatus;    // Update the last known status
    }
}
void BlinkDriver::printElapsedTime() 
{
    if (millis() - lastPrintTime >= 1000) {
        Serial.print("Time since start: ");
        Serial.print((millis() - startTime) / 1000);  // Convert to seconds
        Serial.println(" seconds");
        lastPrintTime = millis();
    }
}