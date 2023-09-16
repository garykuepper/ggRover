#include "Blink.h"
#include <Arduino.h>

Blink::Blink(int pin) : pin(pin), lastBlinkTime(0), ledStatus(false) {}

void Blink::init() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void Blink::update() {
    uint32_t currentMillis = millis();
    if (currentMillis - lastBlinkTime >= 1000) {
        ledStatus = !ledStatus;
        digitalWrite(pin, ledStatus ? HIGH : LOW);
        lastBlinkTime = currentMillis;
    }
}
