#include "Light.h"

Light::Light(int pin)
    : pin(pin), currentBrightness(0), fadeAmount(5), lastUpdate(0) {}

void Light::init() {
    pinMode(pin, OUTPUT);
    analogWrite(pin, currentBrightness);  
}

void Light::setBrightness(int brightness) {
    currentBrightness = brightness;
    analogWrite(pin, currentBrightness);
}

void Light::pulse() {
    currentBrightness = currentBrightness + fadeAmount;

    if (currentBrightness <= 0 || currentBrightness >= 255) {
        fadeAmount = -fadeAmount;
    }
    setBrightness(currentBrightness);
}

void Light::update() {
    uint32_t currentMillis = millis();
    if (currentMillis - lastUpdate >= interval) {
        pulse();
        lastUpdate = currentMillis;
    }
}
