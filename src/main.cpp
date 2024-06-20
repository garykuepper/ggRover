#include "Rover.h"

Rover rover;

void setup() {
    rover.begin();
}

void loop() {
    rover.update();
}
