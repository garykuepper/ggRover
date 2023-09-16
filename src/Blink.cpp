#include "Blink.h"
#include <Arduino.h>

Blink::Blink(int pin) : pin(pin), ledStatus(false) {}

void Blink::init() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void Blink::toggle() {
    ledStatus = !ledStatus;
    digitalWrite(pin, ledStatus ? HIGH : LOW);
}
