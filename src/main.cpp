#include <Arduino.h>
#include "PS4Monitor.h"
#include "Display.h"


const uint32_t baudRate = 115200;

PS4Monitor ps4;
Display display;

void setup() {
    //while (!Serial);
    //Serial.begin(baudRate);
    ps4.begin();
    display.init();
}

void loop() {
    ps4.update();
    display.writeX(0,0,ps4.getX(),50);
}