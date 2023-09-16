#include <Arduino.h>
#include "PS4Monitor.h"



const uint32_t baudRate = 115200;

PS4Monitor ps4;

void setup() {
    while (!Serial);
    Serial.begin(baudRate);
    ps4.begin();
}

void loop() {
    ps4.update();
}