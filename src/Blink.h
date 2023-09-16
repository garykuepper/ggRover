#ifndef BLINK_H
#define BLINK_H

#include <Arduino.h>

class Blink {
public:
    Blink(int pin);  // Constructor: sets up which pin the LED is connected to
    void init();     // Initialization: sets the pin mode
    void update();   // Update: checks and toggles the LED state

private:
    int pin;                   // Pin number where the LED is connected
    uint32_t lastBlinkTime;    // Last time the LED was toggled
    bool ledStatus;            // Current LED status (on/off)
};

#endif
