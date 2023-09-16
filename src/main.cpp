#include <Arduino.h>
#include "Display.h"

Display display;

void setup() {
    display.init();
}

void loop() {
    display.animate();
    display.showTimeSinceStart();
}
