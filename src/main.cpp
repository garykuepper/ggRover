#include <Arduino.h>
#include "OLEDDriver.h"


OLEDDriver oledDriver;

void setup() {
    oledDriver.initialize();
}

void loop() {
    oledDriver.run();
}
