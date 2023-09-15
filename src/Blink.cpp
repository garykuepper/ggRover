// Blink.cpp
#include "Blink.h"

Blink::Blink(uint8_t _pin, uint32_t _interval)
  : pin(_pin), interval(_interval), lastToggleTime(0), ledState(LOW) {
    pinMode(pin, OUTPUT);
}

void Blink::toggle() {
    ledState = !ledState;
    digitalWrite(pin, ledState);
    lastToggleTime = millis();
}

void Blink::update() {
    if (millis() - lastToggleTime >= interval) {
        toggle();
    }
}



String Blink::getStatus() const 
{
    if (ledState == HIGH) {
        return "LED ON";
    } else {
        return "LED OFF";
    }
}

